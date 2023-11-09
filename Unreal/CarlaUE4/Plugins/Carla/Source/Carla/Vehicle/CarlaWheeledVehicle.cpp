// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"
#include "MovementComponents/DefaultMovementComponent.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "UObject/UObjectGlobals.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

#include "PhysXPublic.h"
#include "PhysXVehicleManager.h"
#include "TireConfig.h"
#include "VehicleWheel.h"

#include "Carla.h"
#include "Carla/Game/CarlaHUD.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Trigger/FrictionTrigger.h"
#include "Carla/Util/ActorAttacher.h"
#include "Carla/Util/EmptyActor.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include "Carla/Vegetation/VegetationManager.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

ACarlaWheeledVehicle::ACarlaWheeledVehicle(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  VehicleBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("VehicleBounds"));
  VehicleBounds->SetupAttachment(RootComponent);
  VehicleBounds->SetHiddenInGame(true);
  VehicleBounds->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

  VelocityControl = CreateDefaultSubobject<UVehicleVelocityControl>(TEXT("VelocityControl"));
  VelocityControl->Deactivate();

  GetVehicleMovementComponent()->bReverseAsBrake = false;
  BaseMovementComponent = CreateDefaultSubobject<UBaseCarlaMovementComponent>(TEXT("BaseMovementComponent"));
}

ACarlaWheeledVehicle::~ACarlaWheeledVehicle() {}

void ACarlaWheeledVehicle::SetWheelCollision(UWheeledVehicleMovementComponent4W *Vehicle4W,
    const FVehiclePhysicsControl &PhysicsControl ) {

  #ifdef WHEEL_SWEEP_ENABLED
    const bool IsBike = IsTwoWheeledVehicle();

    if (IsBike)
      return;

    const bool IsEqual = Vehicle4W->UseSweepWheelCollision == PhysicsControl.UseSweepWheelCollision;

    if (IsEqual)
      return;

    Vehicle4W->UseSweepWheelCollision = PhysicsControl.UseSweepWheelCollision;

  #else

    if (PhysicsControl.UseSweepWheelCollision)
      UE_LOG(LogCarla, Warning, TEXT("Error: Sweep for wheel collision is not available. \
      Make sure you have installed the required patch.") );

  #endif

}

void ACarlaWheeledVehicle::SetWheelCollisionNW(UWheeledVehicleMovementComponentNW *VehicleNW,
    const FVehiclePhysicsControl &PhysicsControl ) {

  #ifdef WHEEL_SWEEP_ENABLED
    const bool IsEqual = VehicleNW->UseSweepWheelCollision == PhysicsControl.UseSweepWheelCollision;

    if (IsEqual)
      return;

    VehicleNW->UseSweepWheelCollision = PhysicsControl.UseSweepWheelCollision;

  #else

    if (PhysicsControl.UseSweepWheelCollision)
      UE_LOG(LogCarla, Warning, TEXT("Error: Sweep for wheel collision is not available. \
      Make sure you have installed the required patch.") );

  #endif
}

void ACarlaWheeledVehicle::BeginPlay()
{
  Super::BeginPlay();

  UDefaultMovementComponent::CreateDefaultMovementComponent(this);

  // Get constraint components and their initial transforms
  FTransform ActorInverseTransform = GetActorTransform().Inverse();
  ConstraintsComponents.Empty();
  DoorComponentsTransform.Empty();
  ConstraintDoor.Empty();
  for (FName& ComponentName : ConstraintComponentNames)
  {
    UPhysicsConstraintComponent* ConstraintComponent =
        Cast<UPhysicsConstraintComponent>(GetDefaultSubobjectByName(ComponentName));
    if (ConstraintComponent)
    {
      UPrimitiveComponent* DoorComponent = Cast<UPrimitiveComponent>(
          GetDefaultSubobjectByName(ConstraintComponent->ComponentName1.ComponentName));
      if(DoorComponent)
      {
        UE_LOG(LogCarla, Warning, TEXT("Door name: %s"), *(DoorComponent->GetName()));
        FTransform ComponentWorldTransform = DoorComponent->GetComponentTransform();
        FTransform RelativeTransform = ComponentWorldTransform * ActorInverseTransform;
        DoorComponentsTransform.Add(DoorComponent, RelativeTransform);
        ConstraintDoor.Add(ConstraintComponent, DoorComponent);
        ConstraintsComponents.Add(ConstraintComponent);
        ConstraintComponent->TermComponentConstraint();
      }
      else
      {
        UE_LOG(LogCarla, Error, TEXT("Missing component for constraint: %s"), *(ConstraintComponent->GetName()));
      }
    }
  }
  ResetConstraints();

  // get collision disable constraints (used to prevent doors from colliding with each other)
  CollisionDisableConstraints.Empty();
  TArray<UPhysicsConstraintComponent*> Constraints;
  GetComponents(Constraints);
  for (UPhysicsConstraintComponent* Constraint : Constraints)
  {
    if (!ConstraintsComponents.Contains(Constraint))
    {
      UPrimitiveComponent* CollisionDisabledComponent1 = Cast<UPrimitiveComponent>(
          GetDefaultSubobjectByName(Constraint->ComponentName1.ComponentName));
      UPrimitiveComponent* CollisionDisabledComponent2 = Cast<UPrimitiveComponent>(
          GetDefaultSubobjectByName(Constraint->ComponentName2.ComponentName));
      if (CollisionDisabledComponent1)
      {
        CollisionDisableConstraints.Add(CollisionDisabledComponent1, Constraint);
      }
      if (CollisionDisabledComponent2)
      {
        CollisionDisableConstraints.Add(CollisionDisabledComponent2, Constraint);
      }
    }
  }

  float FrictionScale = 3.5f;

  UWheeledVehicleMovementComponent* MovementComponent = GetVehicleMovementComponent();

  if (MovementComponent)
  {
    check(MovementComponent != nullptr);

    // Setup Tire Configs with default value. This is needed to avoid getting
    // friction values of previously created TireConfigs for the same vehicle
    // blueprint.
    TArray<float> OriginalFrictions;
    OriginalFrictions.Init(FrictionScale, MovementComponent->Wheels.Num());
    SetWheelsFrictionScale(OriginalFrictions);

    // Check if it overlaps with a Friction trigger, if so, update the friction
    // scale.
    TArray<AActor *> OverlapActors;
    GetOverlappingActors(OverlapActors, AFrictionTrigger::StaticClass());
    for (const auto &Actor : OverlapActors)
    {
      AFrictionTrigger *FrictionTrigger = Cast<AFrictionTrigger>(Actor);
      if (FrictionTrigger)
      {
        FrictionScale = FrictionTrigger->Friction;
      }
    }

    // Set the friction scale to Wheel CDO and update wheel setups
    TArray<FWheelSetup> NewWheelSetups = MovementComponent->WheelSetups;
    for (const auto &WheelSetup : NewWheelSetups)
    {
      UVehicleWheel *Wheel = WheelSetup.WheelClass.GetDefaultObject();
      check(Wheel != nullptr);
    }

    MovementComponent->WheelSetups = NewWheelSetups;

    LastPhysicsControl = GetVehiclePhysicsControl();

    // Update physics in the Ackermann Controller
    AckermannController.UpdateVehiclePhysics(this);
  }

  AddReferenceToManager();
}

bool ACarlaWheeledVehicle::IsInVehicleRange(const FVector& Location) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ACarlaWheeledVehicle::IsInVehicleRange);

  return FoliageBoundingBox.IsInside(Location);
}

void ACarlaWheeledVehicle::UpdateDetectionBox()
{
  const FTransform GlobalTransform = GetActorTransform();
  const FVector Vec { DetectionSize, DetectionSize, DetectionSize };
  FBox Box = FBox(-Vec, Vec);
  const FTransform NonScaledTransform(GlobalTransform.GetRotation(), GlobalTransform.GetLocation(), {1.0f, 1.0f, 1.0f});
  FoliageBoundingBox = Box.TransformBy(NonScaledTransform);
}

const TArray<int32> ACarlaWheeledVehicle::GetFoliageInstancesCloseToVehicle(const UInstancedStaticMeshComponent* Component) const
{  
  TRACE_CPUPROFILER_EVENT_SCOPE(ACarlaWheeledVehicle::GetFoliageInstancesCloseToVehicle);
  return Component->GetInstancesOverlappingBox(FoliageBoundingBox);
}

FBox ACarlaWheeledVehicle::GetDetectionBox() const
{  
  TRACE_CPUPROFILER_EVENT_SCOPE(ACarlaWheeledVehicle::GetDetectionBox);
  return FoliageBoundingBox;
}

float ACarlaWheeledVehicle::GetDetectionSize() const
{
  return DetectionSize;
}

void ACarlaWheeledVehicle::DrawFoliageBoundingBox() const
{
  const FVector& Center = FoliageBoundingBox.GetCenter();
  const FVector& Extent = FoliageBoundingBox.GetExtent();
  const FQuat& Rotation = GetActorQuat();
  DrawDebugBox(GetWorld(), Center, Extent, Rotation, FColor::Magenta, false, 0.0f, 0, 5.0f);
}

FBoxSphereBounds ACarlaWheeledVehicle::GetBoxSphereBounds() const
{
  ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
  if (LargeMap)
  {
    FTransform GlobalTransform = LargeMap->LocalToGlobalTransform(GetActorTransform());
    return VehicleBounds->CalcBounds(GlobalTransform);
  }
  return VehicleBounds->CalcBounds(GetActorTransform());
}

void ACarlaWheeledVehicle::AdjustVehicleBounds()
{
  FBoundingBox BoundingBox = UBoundingBoxCalculator::GetVehicleBoundingBox(this);

  const FTransform& CompToWorldTransform = RootComponent->GetComponentTransform();
  const FRotator Rotation = CompToWorldTransform.GetRotation().Rotator();
  const FVector Translation = CompToWorldTransform.GetLocation();
  const FVector Scale = CompToWorldTransform.GetScale3D();

  // Invert BB origin to local space
  BoundingBox.Origin -= Translation;
  BoundingBox.Origin = Rotation.UnrotateVector(BoundingBox.Origin);
  BoundingBox.Origin /= Scale;

  // Prepare Box Collisions
  FTransform Transform;
  Transform.SetTranslation(BoundingBox.Origin);
  VehicleBounds->SetRelativeTransform(Transform);
  VehicleBounds->SetBoxExtent(BoundingBox.Extent);
}

// =============================================================================
// -- Get functions ------------------------------------------------------------
// =============================================================================

float ACarlaWheeledVehicle::GetVehicleForwardSpeed() const
{
  return BaseMovementComponent->GetVehicleForwardSpeed();
}

FVector ACarlaWheeledVehicle::GetVehicleOrientation() const
{
  return GetVehicleTransform().GetRotation().GetForwardVector();
}

int32 ACarlaWheeledVehicle::GetVehicleCurrentGear() const
{
    return BaseMovementComponent->GetVehicleCurrentGear();
}

FTransform ACarlaWheeledVehicle::GetVehicleBoundingBoxTransform() const
{
  return VehicleBounds->GetRelativeTransform();
}

FVector ACarlaWheeledVehicle::GetVehicleBoundingBoxExtent() const
{
  return VehicleBounds->GetScaledBoxExtent();
}

float ACarlaWheeledVehicle::GetMaximumSteerAngle() const
{
  const auto &Wheels = GetVehicleMovementComponent()->Wheels;
  check(Wheels.Num() > 0);
  const auto *FrontWheel = Wheels[0];
  check(FrontWheel != nullptr);
  return FrontWheel->SteerAngle;
}

// =============================================================================
// -- Set functions ------------------------------------------------------------
// =============================================================================

void ACarlaWheeledVehicle::FlushVehicleControl()
{
  if (bAckermannControlActive) {
    AckermannController.UpdateVehicleState(this);
    AckermannController.RunLoop(InputControl.Control);
  }

  BaseMovementComponent->ProcessControl(InputControl.Control);
  InputControl.Control.bReverse = InputControl.Control.Gear < 0;
  LastAppliedControl = InputControl.Control;
  InputControl.Priority = EVehicleInputPriority::INVALID;
}

void ACarlaWheeledVehicle::SetThrottleInput(const float Value)
{
  FVehicleControl Control = InputControl.Control;
  Control.Throttle = Value;
  ApplyVehicleControl(Control, EVehicleInputPriority::User);
}

void ACarlaWheeledVehicle::SetSteeringInput(const float Value)
{
  FVehicleControl Control = InputControl.Control;
  Control.Steer = Value;
  ApplyVehicleControl(Control, EVehicleInputPriority::User);
}

void ACarlaWheeledVehicle::SetBrakeInput(const float Value)
{
  FVehicleControl Control = InputControl.Control;
  Control.Brake = Value;
  ApplyVehicleControl(Control, EVehicleInputPriority::User);
}

void ACarlaWheeledVehicle::SetReverse(const bool Value)
{
  FVehicleControl Control = InputControl.Control;
  Control.bReverse = Value;
  ApplyVehicleControl(Control, EVehicleInputPriority::User);
}

void ACarlaWheeledVehicle::SetHandbrakeInput(const bool Value)
{
  FVehicleControl Control = InputControl.Control;
  Control.bHandBrake = Value;
  ApplyVehicleControl(Control, EVehicleInputPriority::User);
}

TArray<float> ACarlaWheeledVehicle::GetWheelsFrictionScale()
{

  UWheeledVehicleMovementComponent* Movement = GetVehicleMovement();
  TArray<float> WheelsFrictionScale;
  if (Movement)
  {
    check(Movement != nullptr);

    for (auto &Wheel : Movement->Wheels)
    {
      WheelsFrictionScale.Add(Wheel->TireConfig->GetFrictionScale());
    }
  }
  return WheelsFrictionScale;
}

void ACarlaWheeledVehicle::SetWheelsFrictionScale(TArray<float> &WheelsFrictionScale)
{

  UWheeledVehicleMovementComponent* Movement = GetVehicleMovement();
  if (Movement)
  {
    check(Movement != nullptr);
    check(Movement->Wheels.Num() == WheelsFrictionScale.Num());

    for (int32 i = 0; i < Movement->Wheels.Num(); ++i)
    {
      Movement->Wheels[i]->TireConfig->SetFrictionScale(WheelsFrictionScale[i]);
    }
  }
}

FVehiclePhysicsControl ACarlaWheeledVehicle::GetVehiclePhysicsControl() const
{
  FVehiclePhysicsControl PhysicsControl;

  if (!bIsNWVehicle) {
    UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(
          GetVehicleMovement());
    check(Vehicle4W != nullptr);

    // Engine Setup
    PhysicsControl.TorqueCurve = Vehicle4W->EngineSetup.TorqueCurve.EditorCurveData;
    PhysicsControl.MaxRPM = Vehicle4W->EngineSetup.MaxRPM;
    PhysicsControl.MOI = Vehicle4W->EngineSetup.MOI;
    PhysicsControl.DampingRateFullThrottle = Vehicle4W->EngineSetup.DampingRateFullThrottle;
    PhysicsControl.DampingRateZeroThrottleClutchEngaged =
        Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchEngaged;
    PhysicsControl.DampingRateZeroThrottleClutchDisengaged =
        Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchDisengaged;

    // Transmission Setup
    PhysicsControl.bUseGearAutoBox = Vehicle4W->TransmissionSetup.bUseGearAutoBox;
    PhysicsControl.GearSwitchTime = Vehicle4W->TransmissionSetup.GearSwitchTime;
    PhysicsControl.ClutchStrength = Vehicle4W->TransmissionSetup.ClutchStrength;
    PhysicsControl.FinalRatio = Vehicle4W->TransmissionSetup.FinalRatio;

    TArray<FGearPhysicsControl> ForwardGears;

    for (const auto &Gear : Vehicle4W->TransmissionSetup.ForwardGears)
    {
      FGearPhysicsControl GearPhysicsControl;

      GearPhysicsControl.Ratio = Gear.Ratio;
      GearPhysicsControl.UpRatio = Gear.UpRatio;
      GearPhysicsControl.DownRatio = Gear.DownRatio;

      ForwardGears.Add(GearPhysicsControl);
    }

    PhysicsControl.ForwardGears = ForwardGears;

    // Vehicle Setup
    PhysicsControl.Mass = Vehicle4W->Mass;
    PhysicsControl.DragCoefficient = Vehicle4W->DragCoefficient;

    // Center of mass offset (Center of mass is always zero vector in local
    // position)
    UPrimitiveComponent *UpdatedPrimitive = Cast<UPrimitiveComponent>(Vehicle4W->UpdatedComponent);
    check(UpdatedPrimitive != nullptr);

    PhysicsControl.CenterOfMass = UpdatedPrimitive->BodyInstance.COMNudge;

    // Transmission Setup
    PhysicsControl.SteeringCurve = Vehicle4W->SteeringCurve.EditorCurveData;

    // Wheels Setup
    TArray<FWheelPhysicsControl> Wheels;

    for (int32 i = 0; i < Vehicle4W->WheelSetups.Num(); ++i)
    {
      FWheelPhysicsControl PhysicsWheel;

      PxVehicleWheelData PWheelData = Vehicle4W->PVehicle->mWheelsSimData.getWheelData(i);
      PhysicsWheel.DampingRate = Cm2ToM2(PWheelData.mDampingRate);
      PhysicsWheel.MaxSteerAngle = FMath::RadiansToDegrees(PWheelData.mMaxSteer);
      PhysicsWheel.Radius = PWheelData.mRadius;
      PhysicsWheel.MaxBrakeTorque = Cm2ToM2(PWheelData.mMaxBrakeTorque);
      PhysicsWheel.MaxHandBrakeTorque = Cm2ToM2(PWheelData.mMaxHandBrakeTorque);

      PxVehicleTireData PTireData = Vehicle4W->PVehicle->mWheelsSimData.getTireData(i);
      PhysicsWheel.LatStiffMaxLoad = PTireData.mLatStiffX;
      PhysicsWheel.LatStiffValue = PTireData.mLatStiffY;
      PhysicsWheel.LongStiffValue = PTireData.mLongitudinalStiffnessPerUnitGravity;

      PhysicsWheel.TireFriction = Vehicle4W->Wheels[i]->TireConfig->GetFrictionScale();
      PhysicsWheel.Position = Vehicle4W->Wheels[i]->Location;

      Wheels.Add(PhysicsWheel);
    }

    PhysicsControl.Wheels = Wheels;

  } else {
    UWheeledVehicleMovementComponentNW *VehicleNW = Cast<UWheeledVehicleMovementComponentNW>(
      GetVehicleMovement());

    check(VehicleNW != nullptr);

    // Engine Setup
    PhysicsControl.TorqueCurve = VehicleNW->EngineSetup.TorqueCurve.EditorCurveData;
    PhysicsControl.MaxRPM = VehicleNW->EngineSetup.MaxRPM;
    PhysicsControl.MOI = VehicleNW->EngineSetup.MOI;
    PhysicsControl.DampingRateFullThrottle = VehicleNW->EngineSetup.DampingRateFullThrottle;
    PhysicsControl.DampingRateZeroThrottleClutchEngaged =
        VehicleNW->EngineSetup.DampingRateZeroThrottleClutchEngaged;
    PhysicsControl.DampingRateZeroThrottleClutchDisengaged =
        VehicleNW->EngineSetup.DampingRateZeroThrottleClutchDisengaged;

    // Transmission Setup
    PhysicsControl.bUseGearAutoBox = VehicleNW->TransmissionSetup.bUseGearAutoBox;
    PhysicsControl.GearSwitchTime = VehicleNW->TransmissionSetup.GearSwitchTime;
    PhysicsControl.ClutchStrength = VehicleNW->TransmissionSetup.ClutchStrength;
    PhysicsControl.FinalRatio = VehicleNW->TransmissionSetup.FinalRatio;

    TArray<FGearPhysicsControl> ForwardGears;

    for (const auto &Gear : VehicleNW->TransmissionSetup.ForwardGears)
    {
      FGearPhysicsControl GearPhysicsControl;

      GearPhysicsControl.Ratio = Gear.Ratio;
      GearPhysicsControl.UpRatio = Gear.UpRatio;
      GearPhysicsControl.DownRatio = Gear.DownRatio;

      ForwardGears.Add(GearPhysicsControl);
    }

    PhysicsControl.ForwardGears = ForwardGears;

    // VehicleNW Setup
    PhysicsControl.Mass = VehicleNW->Mass;
    PhysicsControl.DragCoefficient = VehicleNW->DragCoefficient;

    // Center of mass offset (Center of mass is always zero vector in local
    // position)
    UPrimitiveComponent *UpdatedPrimitive = Cast<UPrimitiveComponent>(VehicleNW->UpdatedComponent);
    check(UpdatedPrimitive != nullptr);

    PhysicsControl.CenterOfMass = UpdatedPrimitive->BodyInstance.COMNudge;

    // Transmission Setup
    PhysicsControl.SteeringCurve = VehicleNW->SteeringCurve.EditorCurveData;

    // Wheels Setup
    TArray<FWheelPhysicsControl> Wheels;

    for (int32 i = 0; i < VehicleNW->WheelSetups.Num(); ++i)
    {
      FWheelPhysicsControl PhysicsWheel;

      PxVehicleWheelData PWheelData = VehicleNW->PVehicle->mWheelsSimData.getWheelData(i);
      PhysicsWheel.DampingRate = Cm2ToM2(PWheelData.mDampingRate);
      PhysicsWheel.MaxSteerAngle = FMath::RadiansToDegrees(PWheelData.mMaxSteer);
      PhysicsWheel.Radius = PWheelData.mRadius;
      PhysicsWheel.MaxBrakeTorque = Cm2ToM2(PWheelData.mMaxBrakeTorque);
      PhysicsWheel.MaxHandBrakeTorque = Cm2ToM2(PWheelData.mMaxHandBrakeTorque);

      PxVehicleTireData PTireData = VehicleNW->PVehicle->mWheelsSimData.getTireData(i);
      PhysicsWheel.LatStiffMaxLoad = PTireData.mLatStiffX;
      PhysicsWheel.LatStiffValue = PTireData.mLatStiffY;
      PhysicsWheel.LongStiffValue = PTireData.mLongitudinalStiffnessPerUnitGravity;

      PhysicsWheel.TireFriction = VehicleNW->Wheels[i]->TireConfig->GetFrictionScale();
      PhysicsWheel.Position = VehicleNW->Wheels[i]->Location;

      Wheels.Add(PhysicsWheel);
    }

    PhysicsControl.Wheels = Wheels;

  }
  return PhysicsControl;
}

FVehicleLightState ACarlaWheeledVehicle::GetVehicleLightState() const
{
  return InputControl.LightState;
}

void ACarlaWheeledVehicle::RestoreVehiclePhysicsControl()
{
  ApplyVehiclePhysicsControl(LastPhysicsControl);
}

void ACarlaWheeledVehicle::ApplyVehiclePhysicsControl(const FVehiclePhysicsControl &PhysicsControl)
{
  LastPhysicsControl = PhysicsControl;
  if (!bIsNWVehicle) {
    UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(
          GetVehicleMovement());
    check(Vehicle4W != nullptr);

    

    // Engine Setup
    Vehicle4W->EngineSetup.TorqueCurve.EditorCurveData = PhysicsControl.TorqueCurve;
    Vehicle4W->EngineSetup.MaxRPM = PhysicsControl.MaxRPM;

    Vehicle4W->EngineSetup.MOI = PhysicsControl.MOI;

    Vehicle4W->EngineSetup.DampingRateFullThrottle = PhysicsControl.DampingRateFullThrottle;
    Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchEngaged =
        PhysicsControl.DampingRateZeroThrottleClutchEngaged;
    Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchDisengaged =
        PhysicsControl.DampingRateZeroThrottleClutchDisengaged;

    // Transmission Setup
    Vehicle4W->TransmissionSetup.bUseGearAutoBox = PhysicsControl.bUseGearAutoBox;
    Vehicle4W->TransmissionSetup.GearSwitchTime = PhysicsControl.GearSwitchTime;
    Vehicle4W->TransmissionSetup.ClutchStrength = PhysicsControl.ClutchStrength;
    Vehicle4W->TransmissionSetup.FinalRatio = PhysicsControl.FinalRatio;

    TArray<FVehicleGearData> ForwardGears;

    for (const auto &Gear : PhysicsControl.ForwardGears)
    {
      FVehicleGearData GearData;

      GearData.Ratio = Gear.Ratio;
      GearData.UpRatio = Gear.UpRatio;
      GearData.DownRatio = Gear.DownRatio;

      ForwardGears.Add(GearData);
    }

    Vehicle4W->TransmissionSetup.ForwardGears = ForwardGears;

    // Vehicle Setup
    Vehicle4W->Mass = PhysicsControl.Mass;
    Vehicle4W->DragCoefficient = PhysicsControl.DragCoefficient;

    // Center of mass
    UPrimitiveComponent *UpdatedPrimitive = Cast<UPrimitiveComponent>(Vehicle4W->UpdatedComponent);
    check(UpdatedPrimitive != nullptr);

    UpdatedPrimitive->BodyInstance.COMNudge = PhysicsControl.CenterOfMass;

    // Transmission Setup
    Vehicle4W->SteeringCurve.EditorCurveData = PhysicsControl.SteeringCurve;

    // Wheels Setup
    const int PhysicsWheelsNum = PhysicsControl.Wheels.Num();
    if (PhysicsWheelsNum != 4)
    {
      UE_LOG(LogCarla, Error, TEXT("Number of WheelPhysicsControl is not 4."));
      return;
    }

    // Change, if required, the collision mode for wheels
    SetWheelCollision(Vehicle4W, PhysicsControl);

    TArray<FWheelSetup> NewWheelSetups = Vehicle4W->WheelSetups;

    for (int32 i = 0; i < PhysicsWheelsNum; ++i)
    {
      UVehicleWheel *Wheel = NewWheelSetups[i].WheelClass.GetDefaultObject();
      check(Wheel != nullptr);

      // Assigning new tire config
      Wheel->TireConfig = DuplicateObject<UTireConfig>(Wheel->TireConfig, nullptr);

      // Setting a new value to friction
      Wheel->TireConfig->SetFrictionScale(PhysicsControl.Wheels[i].TireFriction);
    }

    Vehicle4W->WheelSetups = NewWheelSetups;

    // Recreate Physics State for vehicle setup
    GetWorld()->GetPhysicsScene()->GetPxScene()->lockWrite();
    Vehicle4W->RecreatePhysicsState();
    GetWorld()->GetPhysicsScene()->GetPxScene()->unlockWrite();

    for (int32 i = 0; i < PhysicsWheelsNum; ++i)
    {
      PxVehicleWheelData PWheelData = Vehicle4W->PVehicle->mWheelsSimData.getWheelData(i);

      PWheelData.mRadius = PhysicsControl.Wheels[i].Radius;
      PWheelData.mMaxSteer = FMath::DegreesToRadians(PhysicsControl.Wheels[i].MaxSteerAngle);
      PWheelData.mDampingRate = M2ToCm2(PhysicsControl.Wheels[i].DampingRate);
      PWheelData.mMaxBrakeTorque = M2ToCm2(PhysicsControl.Wheels[i].MaxBrakeTorque);
      PWheelData.mMaxHandBrakeTorque = M2ToCm2(PhysicsControl.Wheels[i].MaxHandBrakeTorque);
      Vehicle4W->PVehicle->mWheelsSimData.setWheelData(i, PWheelData);

      PxVehicleTireData PTireData = Vehicle4W->PVehicle->mWheelsSimData.getTireData(i);
      PTireData.mLatStiffX = PhysicsControl.Wheels[i].LatStiffMaxLoad;
      PTireData.mLatStiffY = PhysicsControl.Wheels[i].LatStiffValue;
      PTireData.mLongitudinalStiffnessPerUnitGravity = PhysicsControl.Wheels[i].LongStiffValue;
      Vehicle4W->PVehicle->mWheelsSimData.setTireData(i, PTireData);
    }

    ResetConstraints();
  } else {
    UWheeledVehicleMovementComponentNW *VehicleNW = Cast<UWheeledVehicleMovementComponentNW>(
          GetVehicleMovement());

    check(VehicleNW != nullptr);

    // Engine Setup
    VehicleNW->EngineSetup.TorqueCurve.EditorCurveData = PhysicsControl.TorqueCurve;
    VehicleNW->EngineSetup.MaxRPM = PhysicsControl.MaxRPM;

    VehicleNW->EngineSetup.MOI = PhysicsControl.MOI;

    VehicleNW->EngineSetup.DampingRateFullThrottle = PhysicsControl.DampingRateFullThrottle;
    VehicleNW->EngineSetup.DampingRateZeroThrottleClutchEngaged =
        PhysicsControl.DampingRateZeroThrottleClutchEngaged;
    VehicleNW->EngineSetup.DampingRateZeroThrottleClutchDisengaged =
        PhysicsControl.DampingRateZeroThrottleClutchDisengaged;

    // Transmission Setup
    VehicleNW->TransmissionSetup.bUseGearAutoBox = PhysicsControl.bUseGearAutoBox;
    VehicleNW->TransmissionSetup.GearSwitchTime = PhysicsControl.GearSwitchTime;
    VehicleNW->TransmissionSetup.ClutchStrength = PhysicsControl.ClutchStrength;
    VehicleNW->TransmissionSetup.FinalRatio = PhysicsControl.FinalRatio;

    TArray<FVehicleNWGearData> ForwardGears;

    for (const auto &Gear : PhysicsControl.ForwardGears)
    {
      FVehicleNWGearData GearData;

      GearData.Ratio = Gear.Ratio;
      GearData.UpRatio = Gear.UpRatio;
      GearData.DownRatio = Gear.DownRatio;

      ForwardGears.Add(GearData);
    }

    VehicleNW->TransmissionSetup.ForwardGears = ForwardGears;

    // VehicleNW Setup
    VehicleNW->Mass = PhysicsControl.Mass;
    VehicleNW->DragCoefficient = PhysicsControl.DragCoefficient;

    // Center of mass
    UPrimitiveComponent *UpdatedPrimitive = Cast<UPrimitiveComponent>(VehicleNW->UpdatedComponent);
    check(UpdatedPrimitive != nullptr);

    UpdatedPrimitive->BodyInstance.COMNudge = PhysicsControl.CenterOfMass;

    // Transmission Setup
    VehicleNW->SteeringCurve.EditorCurveData = PhysicsControl.SteeringCurve;

    // Wheels Setup
    const int PhysicsWheelsNum = PhysicsControl.Wheels.Num();

    // Change, if required, the collision mode for wheels
    SetWheelCollisionNW(VehicleNW, PhysicsControl);

    TArray<FWheelSetup> NewWheelSetups = VehicleNW->WheelSetups;

    for (int32 i = 0; i < PhysicsWheelsNum; ++i)
    {
      UVehicleWheel *Wheel = NewWheelSetups[i].WheelClass.GetDefaultObject();
      check(Wheel != nullptr);

      // Assigning new tire config
      Wheel->TireConfig = DuplicateObject<UTireConfig>(Wheel->TireConfig, nullptr);

      // Setting a new value to friction
      Wheel->TireConfig->SetFrictionScale(PhysicsControl.Wheels[i].TireFriction);
    }

    VehicleNW->WheelSetups = NewWheelSetups;

    // Recreate Physics State for vehicle setup
    GetWorld()->GetPhysicsScene()->GetPxScene()->lockWrite();
    VehicleNW->RecreatePhysicsState();
    GetWorld()->GetPhysicsScene()->GetPxScene()->unlockWrite();

    for (int32 i = 0; i < PhysicsWheelsNum; ++i)
    {
      PxVehicleWheelData PWheelData = VehicleNW->PVehicle->mWheelsSimData.getWheelData(i);

      PWheelData.mRadius = PhysicsControl.Wheels[i].Radius;
      PWheelData.mMaxSteer = FMath::DegreesToRadians(PhysicsControl.Wheels[i].MaxSteerAngle);
      PWheelData.mDampingRate = M2ToCm2(PhysicsControl.Wheels[i].DampingRate);
      PWheelData.mMaxBrakeTorque = M2ToCm2(PhysicsControl.Wheels[i].MaxBrakeTorque);
      PWheelData.mMaxHandBrakeTorque = M2ToCm2(PhysicsControl.Wheels[i].MaxHandBrakeTorque);
      VehicleNW->PVehicle->mWheelsSimData.setWheelData(i, PWheelData);

      PxVehicleTireData PTireData = VehicleNW->PVehicle->mWheelsSimData.getTireData(i);
      PTireData.mLatStiffX = PhysicsControl.Wheels[i].LatStiffMaxLoad;
      PTireData.mLatStiffY = PhysicsControl.Wheels[i].LatStiffValue;
      PTireData.mLongitudinalStiffnessPerUnitGravity = PhysicsControl.Wheels[i].LongStiffValue;
      VehicleNW->PVehicle->mWheelsSimData.setTireData(i, PTireData);
    }

    ResetConstraints();

  }

  auto * Recorder = UCarlaStatics::GetRecorder(GetWorld());
  if (Recorder && Recorder->IsEnabled())
  {
    Recorder->AddPhysicsControl(*this);
  }

  // Update physics in the Ackermann Controller
  AckermannController.UpdateVehiclePhysics(this);
  
}

void ACarlaWheeledVehicle::ActivateVelocityControl(const FVector &Velocity)
{
  VelocityControl->Activate(Velocity);
}

void ACarlaWheeledVehicle::DeactivateVelocityControl()
{
  VelocityControl->Deactivate();
}

void ACarlaWheeledVehicle::ShowDebugTelemetry(bool Enabled)
{
  if (GetWorld()->GetFirstPlayerController())
  {
    ACarlaHUD* hud = Cast<ACarlaHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
    if (hud) {

      // Set/Unset the car movement component in HUD to show the temetry
      if (Enabled) {
        hud->AddDebugVehicleForTelemetry(GetVehicleMovementComponent());
      }
      else{
        if (hud->DebugVehicle == GetVehicleMovementComponent()) {
          hud->AddDebugVehicleForTelemetry(nullptr);
          GetVehicleMovementComponent()->StopTelemetry();
        }
      }

    }
    else {
      UE_LOG(LogCarla, Warning, TEXT("ACarlaWheeledVehicle::ShowDebugTelemetry:: Cannot find HUD for debug info"));
    }
  }
}

void ACarlaWheeledVehicle::SetVehicleLightState(const FVehicleLightState &LightState)
{
  if (LightState.Position != InputControl.LightState.Position ||
      LightState.LowBeam != InputControl.LightState.LowBeam ||
      LightState.HighBeam != InputControl.LightState.HighBeam ||
      LightState.Brake != InputControl.LightState.Brake ||
      LightState.RightBlinker != InputControl.LightState.RightBlinker ||
      LightState.LeftBlinker != InputControl.LightState.LeftBlinker ||
      LightState.Reverse != InputControl.LightState.Reverse ||
      LightState.Fog != InputControl.LightState.Fog ||
      LightState.Interior != InputControl.LightState.Interior ||
      LightState.Special1 != InputControl.LightState.Special1 ||
      LightState.Special2 != InputControl.LightState.Special2)
  {
    InputControl.LightState = LightState;
    RefreshLightState(LightState);
  }
}

void ACarlaWheeledVehicle::SetFailureState(const carla::rpc::VehicleFailureState &InFailureState)
{
  FailureState = InFailureState;
}

void ACarlaWheeledVehicle::SetCarlaMovementComponent(UBaseCarlaMovementComponent* MovementComponent)
{
  if (BaseMovementComponent)
  {
    BaseMovementComponent->DestroyComponent();
  }
  BaseMovementComponent = MovementComponent;
}

void ACarlaWheeledVehicle::SetWheelSteerDirection(EVehicleWheelLocation WheelLocation, float AngleInDeg) {

  if (bPhysicsEnabled == false)
  {
    check((uint8)WheelLocation >= 0)
    UVehicleAnimInstance *VehicleAnim = Cast<UVehicleAnimInstance>(GetMesh()->GetAnimInstance());
    check(VehicleAnim != nullptr)
    VehicleAnim->SetWheelRotYaw((uint8)WheelLocation, AngleInDeg);
  }
  else
  {
    UE_LOG(LogTemp, Warning, TEXT("Cannot set wheel steer direction. Physics are enabled."))
  }
}

float ACarlaWheeledVehicle::GetWheelSteerAngle(EVehicleWheelLocation WheelLocation) {

  check((uint8)WheelLocation >= 0)
  UVehicleAnimInstance *VehicleAnim = Cast<UVehicleAnimInstance>(GetMesh()->GetAnimInstance());
  check(VehicleAnim != nullptr)
  check(VehicleAnim->GetWheeledVehicleMovementComponent() != nullptr)

  if (bPhysicsEnabled == true)
  {
    return VehicleAnim->GetWheeledVehicleMovementComponent()->Wheels[(uint8)WheelLocation]->GetSteerAngle();
  }
  else
  {
    return VehicleAnim->GetWheelRotAngle((uint8)WheelLocation);
  }
}

void ACarlaWheeledVehicle::SetSimulatePhysics(bool enabled) {
  if(!GetCarlaMovementComponent<UDefaultMovementComponent>())
  {
    return;
  }

  UWheeledVehicleMovementComponent* Movement = GetVehicleMovement();
  if (Movement)
  {
    check(Movement != nullptr);

    if(bPhysicsEnabled == enabled)
      return;

    SetActorEnableCollision(true);
    auto RootComponent = Cast<UPrimitiveComponent>(GetRootComponent());
    RootComponent->SetSimulatePhysics(enabled);
    RootComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    UVehicleAnimInstance *VehicleAnim = Cast<UVehicleAnimInstance>(GetMesh()->GetAnimInstance());
    check(VehicleAnim != nullptr)

    GetWorld()->GetPhysicsScene()->GetPxScene()->lockWrite();
    if (enabled)
    {
      Movement->RecreatePhysicsState();
      VehicleAnim->ResetWheelCustomRotations();
    }
    else
    {
      Movement->DestroyPhysicsState();
    }

    GetWorld()->GetPhysicsScene()->GetPxScene()->unlockWrite();

    bPhysicsEnabled = enabled;

    ResetConstraints();
  }

}

void ACarlaWheeledVehicle::ResetConstraints()
{
  for (int i = 0; i < ConstraintsComponents.Num(); i++)
  {
    OpenDoorPhys(EVehicleDoor(i));
  }
  for (int i = 0; i < ConstraintsComponents.Num(); i++)
  {
    CloseDoorPhys(EVehicleDoor(i));
  }
}

FVector ACarlaWheeledVehicle::GetVelocity() const
{
  return BaseMovementComponent->GetVelocity();
}

void ACarlaWheeledVehicle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  ShowDebugTelemetry(false);
  Super::EndPlay(EndPlayReason);
  RemoveReferenceToManager();
}

void ACarlaWheeledVehicle::OpenDoor(const EVehicleDoor DoorIdx) {
  if (int(DoorIdx) >= ConstraintsComponents.Num() && DoorIdx != EVehicleDoor::All) {
    UE_LOG(LogTemp, Warning, TEXT("This door is not configured for this car."));
    return;
  }

  if (DoorIdx == EVehicleDoor::All) {
    for (int i = 0; i < ConstraintsComponents.Num(); i++)
    {
      OpenDoorPhys(EVehicleDoor(i));
    }
    return;
  }

  OpenDoorPhys(DoorIdx);
}

void ACarlaWheeledVehicle::CloseDoor(const EVehicleDoor DoorIdx) {
  if (int(DoorIdx) >= ConstraintsComponents.Num() && DoorIdx != EVehicleDoor::All) {
    UE_LOG(LogTemp, Warning, TEXT("This door is not configured for this car."));
    return;
  }

  if (DoorIdx == EVehicleDoor::All) {
    for (int i = 0; i < ConstraintsComponents.Num(); i++)
    {
      CloseDoorPhys(EVehicleDoor(i));
    }
    return;
  }

  CloseDoorPhys(DoorIdx);
}

void ACarlaWheeledVehicle::OpenDoorPhys(const EVehicleDoor DoorIdx)
{
  UPhysicsConstraintComponent* Constraint = ConstraintsComponents[static_cast<int>(DoorIdx)];
  UPrimitiveComponent* DoorComponent = ConstraintDoor[Constraint];
  DoorComponent->DetachFromComponent(
      FDetachmentTransformRules(EDetachmentRule::KeepWorld, false));
  FTransform DoorInitialTransform =
      DoorComponentsTransform[DoorComponent] * GetActorTransform();
  DoorComponent->SetWorldTransform(DoorInitialTransform);
  DoorComponent->SetSimulatePhysics(true);
  DoorComponent->SetCollisionProfileName(TEXT("BlockAll"));
  float AngleLimit = Constraint->ConstraintInstance.GetAngularSwing1Limit();
  FRotator AngularRotationOffset = Constraint->ConstraintInstance.AngularRotationOffset;

  if (Constraint->ConstraintInstance.AngularRotationOffset.Yaw < 0.0f)
  {
    AngleLimit = -AngleLimit;
  }
  Constraint->SetAngularOrientationTarget(FRotator(0, AngleLimit, 0));
  Constraint->SetAngularDriveParams(DoorOpenStrength, 1.0, 0.0);

  Constraint->InitComponentConstraint();

  UPhysicsConstraintComponent** CollisionDisable =
      CollisionDisableConstraints.Find(DoorComponent);
  if (CollisionDisable)
  {
    (*CollisionDisable)->InitComponentConstraint();
  }
}

void ACarlaWheeledVehicle::CloseDoorPhys(const EVehicleDoor DoorIdx)
{
  UPhysicsConstraintComponent* Constraint = ConstraintsComponents[static_cast<int>(DoorIdx)];
  UPrimitiveComponent* DoorComponent = ConstraintDoor[Constraint];
  FTransform DoorInitialTransform =
      DoorComponentsTransform[DoorComponent] * GetActorTransform();
  DoorComponent->SetSimulatePhysics(false);
  DoorComponent->SetCollisionProfileName(TEXT("NoCollision"));
  DoorComponent->SetWorldTransform(DoorInitialTransform);
  DoorComponent->AttachToComponent(
      GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
}

void ACarlaWheeledVehicle::ApplyRolloverBehavior()
{
  auto roll = GetVehicleTransform().Rotator().Roll;

  // The angular velocity reduction is applied in 4 stages, to improve its smoothness.
  // Case 4 starts the timer to set the rollover flag, so users are notified.
  switch (RolloverBehaviorTracker) {
    case 0: CheckRollover(roll, std::make_pair(130.0, 230.0));      break;
    case 1: CheckRollover(roll, std::make_pair(140.0, 220.0));      break;
    case 2: CheckRollover(roll, std::make_pair(150.0, 210.0));      break;
    case 3: CheckRollover(roll, std::make_pair(160.0, 200.0));      break;
    case 4:
      GetWorld()->GetTimerManager().SetTimer(TimerHandler, this, &ACarlaWheeledVehicle::SetRolloverFlag, RolloverFlagTime);
      RolloverBehaviorTracker += 1;
      break;
    case 5: break;
    default:
      RolloverBehaviorTracker = 5;
  }

  // In case the vehicle recovers, reset the rollover tracker
  if (RolloverBehaviorTracker > 0 && -30 < roll && roll < 30){
    RolloverBehaviorTracker = 0;
    FailureState = carla::rpc::VehicleFailureState::None;
  }
}

void ACarlaWheeledVehicle::CheckRollover(const float roll, const std::pair<float, float> threshold_roll){
  if (threshold_roll.first < roll && roll < threshold_roll.second){
    auto RootComponent = Cast<UPrimitiveComponent>(GetRootComponent());
    auto angular_velocity = RootComponent->GetPhysicsAngularVelocityInDegrees();
    RootComponent->SetPhysicsAngularVelocity((1 - RolloverBehaviorForce) * angular_velocity);
    RolloverBehaviorTracker += 1;
  }
}

void ACarlaWheeledVehicle::SetRolloverFlag(){
  // Make sure the vehicle hasn't recovered since the timer started
  if (RolloverBehaviorTracker >= 4) {
    FailureState = carla::rpc::VehicleFailureState::Rollover;
  }
}

carla::rpc::VehicleFailureState ACarlaWheeledVehicle::GetFailureState() const{
  return FailureState;
}

void ACarlaWheeledVehicle::AddReferenceToManager()
{
  const UObject* World = GetWorld();
  TArray<AActor*> ActorsInLevel;
  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), ActorsInLevel);
  for (AActor* Actor : ActorsInLevel)
  {
    AVegetationManager* Manager = Cast<AVegetationManager>(Actor);
    if (!IsValid(Manager))
      continue;
    Manager->AddVehicle(this);
    return;
  }
}

void ACarlaWheeledVehicle::RemoveReferenceToManager()
{
  const UObject* World = GetWorld();
  TArray<AActor*> ActorsInLevel;
  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), ActorsInLevel);
  for (AActor* Actor : ActorsInLevel)
  {
    AVegetationManager* Manager = Cast<AVegetationManager>(Actor);
    if (!IsValid(Manager))
      continue;
    Manager->RemoveVehicle(this);
    return;
  }
}

FRotator ACarlaWheeledVehicle::GetPhysicsConstraintAngle(
    UPhysicsConstraintComponent* Component)
{
  return Component->ConstraintInstance.AngularRotationOffset;
}

void ACarlaWheeledVehicle::SetPhysicsConstraintAngle(
    UPhysicsConstraintComponent* Component, const FRotator &NewAngle)
{
  Component->ConstraintInstance.AngularRotationOffset = NewAngle;
}
