// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Vehicle/CarlaWheeledVehicle.h"

#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"
#include "MovementComponents/DefaultMovementComponent.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "VehicleAnimationInstance.h"
#include "UObject/UObjectGlobals.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Carla.h"
#include "Carla/Game/CarlaHUD.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Trigger/FrictionTrigger.h"
#include "Carla/Util/ActorAttacher.h"
#include "Carla/Util/EmptyActor.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include "Carla/Vegetation/VegetationManager.h"

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

void ACarlaWheeledVehicle::PrintROS2Message(const char* Message)
{
  FString ROSMessage = Message;
  UE_LOG(LogCarla, Warning, TEXT("ROS2 Message received: %s"), *ROSMessage);
}

ACarlaWheeledVehicle::ACarlaWheeledVehicle(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  VehicleBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("VehicleBounds"));
  VehicleBounds->SetupAttachment(RootComponent);
  VehicleBounds->SetHiddenInGame(true);
  VehicleBounds->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);


  VelocityControl = CreateDefaultSubobject<UVehicleVelocityControl>(TEXT("VelocityControl"));
  VelocityControl->Deactivate();

  GetChaosWheeledVehicleMovementComponent()->bReverseAsBrake = false;
  BaseMovementComponent = CreateDefaultSubobject<UBaseCarlaMovementComponent>(TEXT("BaseMovementComponent"));

}

ACarlaWheeledVehicle::~ACarlaWheeledVehicle() {}

void ACarlaWheeledVehicle::SetWheelCollisionNW(UChaosWheeledVehicleMovementComponent *VehicleNW,
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

  UChaosWheeledVehicleMovementComponent* MovementComponent = GetChaosWheeledVehicleMovementComponent();

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
    TArray<FChaosWheelSetup> NewWheelSetups = MovementComponent->WheelSetups;
    for (const FChaosWheelSetup& WheelSetup : NewWheelSetups)
    {
      UChaosVehicleWheel *Wheel = WheelSetup.WheelClass.GetDefaultObject();
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
  UChaosWheeledVehicleMovementComponent* MovementComponent = GetChaosWheeledVehicleMovementComponent();
  const auto& Wheels = MovementComponent->WheelSetups;
  check(Wheels.Num() > 0);
  const UChaosVehicleWheel* FrontWheel =
      Cast<UChaosVehicleWheel>(Wheels[0].WheelClass->GetDefaultObject());
  check(FrontWheel != nullptr);
  return FrontWheel->MaxSteerAngle;
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
  TArray<float> WheelsFrictionScale;
  UChaosWheeledVehicleMovementComponent* Movement = GetChaosWheeledVehicleMovementComponent();
  if (Movement)
  {
    check(Movement != nullptr);

    for (auto &Wheel : Movement->Wheels)
    {
      WheelsFrictionScale.Add(Wheel->FrictionForceMultiplier);
    }
  }

  return WheelsFrictionScale;
}

void ACarlaWheeledVehicle::SetWheelsFrictionScale(TArray<float> &WheelsFrictionScale)
{
  UChaosWheeledVehicleMovementComponent* Movement = GetChaosWheeledVehicleMovementComponent();
  if (Movement)
  {
    check(Movement != nullptr);
    check(Movement->Wheels.Num() == WheelsFrictionScale.Num());

    for (int32 i = 0; i < Movement->Wheels.Num(); ++i)
    {
      Movement->Wheels[i]->FrictionForceMultiplier = WheelsFrictionScale[i];
    }
  }
}

FVehiclePhysicsControl ACarlaWheeledVehicle::GetVehiclePhysicsControl() const
{
  FVehiclePhysicsControl PhysicsControl;
  UChaosWheeledVehicleMovementComponent* VehicleMovComponent = GetChaosWheeledVehicleMovementComponent();
  check(VehicleMovComponent != nullptr);

  // Engine Setup
  PhysicsControl.TorqueCurve = VehicleMovComponent->EngineSetup.TorqueCurve.EditorCurveData;
  PhysicsControl.MaxRPM = VehicleMovComponent->EngineSetup.MaxRPM;
  PhysicsControl.MOI = VehicleMovComponent->EngineSetup.EngineRevUpMOI;
#if 0 // @CARLAUE5
  PhysicsControl.DampingRateFullThrottle = VehicleMovComponent->EngineSetup.DampingRateFullThrottle;
  PhysicsControl.DampingRateZeroThrottleClutchEngaged =
      VehicleMovComponent->EngineSetup.DampingRateZeroThrottleClutchEngaged;
  PhysicsControl.DampingRateZeroThrottleClutchDisengaged =
      VehicleMovComponent->EngineSetup.DampingRateZeroThrottleClutchDisengaged;
#endif
  // Transmission Setup
  PhysicsControl.bUseGearAutoBox = VehicleMovComponent->TransmissionSetup.bUseAutomaticGears;
  PhysicsControl.GearSwitchTime = VehicleMovComponent->TransmissionSetup.GearChangeTime;
  //PhysicsControl.ClutchStrength = VehicleMovComponent->TransmissionSetup.ClutchStrength;
  PhysicsControl.FinalRatio = VehicleMovComponent->TransmissionSetup.FinalRatio;

  TArray<FGearPhysicsControl> ForwardGears;

  for (const auto &Gear : VehicleMovComponent->TransmissionSetup.ForwardGearRatios)
  {
    FGearPhysicsControl GearPhysicsControl;
    GearPhysicsControl.Ratio = Gear;
    ForwardGears.Add(GearPhysicsControl);
  }

  // ToDo:: Backwards?

  PhysicsControl.ForwardGears = ForwardGears;

  // VehicleMovComponent Setup
  PhysicsControl.Mass = VehicleMovComponent->Mass;
  PhysicsControl.DragCoefficient = VehicleMovComponent->DragCoefficient;

  // Center of mass offset (Center of mass is always zero vector in local
  // position)
  UPrimitiveComponent *UpdatedPrimitive = Cast<UPrimitiveComponent>(VehicleMovComponent->UpdatedComponent);
  check(UpdatedPrimitive != nullptr);

  PhysicsControl.CenterOfMass = UpdatedPrimitive->GetCenterOfMass();

  // Transmission Setup
  PhysicsControl.SteeringCurve = VehicleMovComponent->SteeringSetup.SteeringCurve.EditorCurveData;

  // Wheels Setup
  TArray<FWheelPhysicsControl> Wheels;

  for (int32 i = 0; i < VehicleMovComponent->WheelSetups.Num(); ++i)
  {
    FWheelPhysicsControl PhysicsWheel;

    // To Do:: Physics command to fill the wheel?
    /*PhysicsControl.Wheels[i].MaxSteerAngle = VehicleMovComponent->GetBodyInstance()->ActorHandle->;
    VehicleMovComponent->SetWheelRadius(i, PhysicsControl.Wheels[i].Radius);
    VehicleMovComponent->SetWheelMaxBrakeTorque(i, PhysicsControl.Wheels[i].MaxBrakeTorque);
    VehicleMovComponent->SetWheelHandbrakeTorque(i, PhysicsControl.Wheels[i].MaxHandBrakeTorque);
    VehicleMovComponent->SetWheelMaxBrakeTorque(i, PhysicsControl.Wheels[i].MaxBrakeTorque);
    */

    Wheels.Add(PhysicsWheel);
  }

  PhysicsControl.Wheels = Wheels;

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
  UChaosWheeledVehicleMovementComponent* VehicleMovComponent = GetChaosWheeledVehicleMovementComponent();
  check(VehicleMovComponent != nullptr);

  // Engine Setup
  VehicleMovComponent->EngineSetup.TorqueCurve.EditorCurveData = PhysicsControl.TorqueCurve;
  VehicleMovComponent->EngineSetup.MaxRPM = PhysicsControl.MaxRPM;

  VehicleMovComponent->EngineSetup.EngineRevUpMOI = PhysicsControl.MOI;
#if 0 // @CARLAUE5

  VehicleMovComponent->EngineSetup.DampingRateFullThrottle = PhysicsControl.DampingRateFullThrottle;
  VehicleMovComponent->EngineSetup.DampingRateZeroThrottleClutchEngaged =
      PhysicsControl.DampingRateZeroThrottleClutchEngaged;
  VehicleMovComponent->EngineSetup.DampingRateZeroThrottleClutchDisengaged =
      PhysicsControl.DampingRateZeroThrottleClutchDisengaged;
#endif
  // Transmission Setup
  VehicleMovComponent->TransmissionSetup.bUseAutomaticGears = PhysicsControl.bUseGearAutoBox;
  VehicleMovComponent->TransmissionSetup.GearChangeTime = PhysicsControl.GearSwitchTime;
  VehicleMovComponent->TransmissionSetup.FinalRatio = PhysicsControl.FinalRatio;

  /*
  This does not exist in Chaos control
  VehicleMovComponent->TransmissionSetup.ClutchStrength = PhysicsControl.ClutchStrength;
  */

  TArray<float> ForwardGears;

  for (const auto &Gear : PhysicsControl.ForwardGears)
  {
    /*
    We do not have up/down ratios on chaos
    FVehicleGearData GearData;
    GearData.Ratio = Gear.Ratio;
    GearData.UpRatio = Gear.UpRatio;
    GearData.DownRatio = Gear.DownRatio;
    */

    ForwardGears.Add(Gear.UpRatio);
  }

  VehicleMovComponent->TransmissionSetup.ForwardGearRatios = ForwardGears;

  // Vehicle Setup
  VehicleMovComponent->Mass = PhysicsControl.Mass;
  VehicleMovComponent->DragCoefficient = PhysicsControl.DragCoefficient;

  // Center of mass
  UPrimitiveComponent *UpdatedPrimitive = Cast<UPrimitiveComponent>(VehicleMovComponent->UpdatedComponent);
  check(UpdatedPrimitive != nullptr);

  UpdatedPrimitive->BodyInstance.COMNudge = PhysicsControl.CenterOfMass;

  // Transmission Setup
  VehicleMovComponent->SteeringSetup.SteeringCurve.EditorCurveData = PhysicsControl.SteeringCurve;

  // Wheels Setup
  const int PhysicsWheelsNum = PhysicsControl.Wheels.Num();
  
  // TODO:: What about bikes or NW vehicles?
  if (PhysicsWheelsNum != 4)
  {
    UE_LOG(LogCarla, Error, TEXT("Number of WheelPhysicsControl is not 4."));
    return;
  }

  // Change, if required, the collision mode for wheels
  SetWheelCollisionNW(VehicleMovComponent, PhysicsControl);
  TArray<FChaosWheelSetup> NewWheelSetups = VehicleMovComponent->WheelSetups;

  for (int32 i = 0; i < PhysicsWheelsNum; ++i)
  {
    UChaosVehicleWheel *Wheel = NewWheelSetups[i].WheelClass.GetDefaultObject();
    check(Wheel != nullptr);

    // Setting a new value to friction
    Wheel->FrictionForceMultiplier = PhysicsControl.Wheels[i].FrictionForceMultiplier;
  }

  VehicleMovComponent->WheelSetups = NewWheelSetups;

  // Recreate Physics State for vehicle setup
  //VehicleMovComponent->CreateVehicle();

  for (int32 i = 0; i < PhysicsWheelsNum; ++i)
  {
    VehicleMovComponent->SetWheelMaxSteerAngle(i, PhysicsControl.Wheels[i].MaxSteerAngle);
    VehicleMovComponent->SetWheelRadius(i, PhysicsControl.Wheels[i].Radius);
    VehicleMovComponent->SetWheelMaxBrakeTorque(i, PhysicsControl.Wheels[i].MaxBrakeTorque);
    VehicleMovComponent->SetWheelHandbrakeTorque(i, PhysicsControl.Wheels[i].MaxHandBrakeTorque);
    VehicleMovComponent->SetWheelMaxBrakeTorque(i, PhysicsControl.Wheels[i].MaxBrakeTorque);
#if 0 // @CARLAUE5 
    // TO DO:: Stiffnes in chaos?¿
    PxVehicleWheelData PWheelData = VehicleMovComponent->PVehicle->mWheelsSimData.getWheelData(i);
    PWheelData.mDampingRate = M2ToCm2(PhysicsControl.Wheels[i].DampingRate);
    PxVehicleTireData PTireData = VehicleMovComponent->PVehicle->mWheelsSimData.getTireData(i);
    PTireData.mLatStiffX = PhysicsControl.Wheels[i].LatStiffMaxLoad;
    PTireData.mLatStiffY = PhysicsControl.Wheels[i].LatStiffValue;
    PTireData.mLongitudinalStiffnessPerUnitGravity = PhysicsControl.Wheels[i].LongStiffValue;
    VehicleMovComponent->PVehicle->mWheelsSimData.setTireData(i, PTireData);
#endif
  }
  ResetConstraints();


  auto * Recorder = UCarlaStatics::GetRecorder(GetWorld());
  if (Recorder && Recorder->IsEnabled())
  {
    Recorder->AddPhysicsControl(*this);
  }

  // Update physics in the Ackermann Controller
  AckermannController.UpdateVehiclePhysics(this);
}

void ACarlaWheeledVehicle::ApplyVehicleControl(const FVehicleControl &Control, EVehicleInputPriority Priority)
{
  if (bAckermannControlActive) {
    AckermannController.Reset();
  }
  bAckermannControlActive = false;

  if (InputControl.Priority <= Priority)
  {
    InputControl.Control = Control;
    InputControl.Priority = Priority;
  }
}

  void ACarlaWheeledVehicle::ApplyVehicleAckermannControl(const FVehicleAckermannControl &AckermannControl, EVehicleInputPriority Priority)
  {
    bAckermannControlActive = true;
    LastAppliedAckermannControl = AckermannControl;
    AckermannController.SetTargetPoint(AckermannControl);
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
  UE_LOG(LogCarla, Warning, TEXT("ACarlaWheeledVehicle::ShowDebugTelemetry:: Chaos does not support Debug telemetry"));
  return;
  //To Do: Implement Debug Telemetry?
  /*
  if (GetWorld()->GetFirstPlayerController())
  {
    ACarlaHUD* hud = Cast<ACarlaHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
    if (hud) {

      // Set/Unset the car movement component in HUD to show the temetry
      if (Enabled) {
        hud->AddDebugVehicleForTelemetry(GetChaosWheeledVehicleMovementComponent());
      }
      else{
        if (hud->DebugVehicle == GetChaosWheeledVehicleMovementComponent()) {
          hud->AddDebugVehicleForTelemetry(nullptr);
          //GetChaosWheeledVehicleMovementComponent()->StopTelemetry();
        }
      }

    }
    else {
      UE_LOG(LogCarla, Warning, TEXT("ACarlaWheeledVehicle::ShowDebugTelemetry:: Cannot find HUD for debug info"));
    }
  }
  */
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

void ACarlaWheeledVehicle::SetCarlaMovementComponent(UBaseCarlaMovementComponent* NewBaseMovementComponent)
{
  if (BaseMovementComponent)
  {
    BaseMovementComponent->DestroyComponent();
  }
  BaseMovementComponent = NewBaseMovementComponent;
}

void ACarlaWheeledVehicle::SetWheelSteerDirection(EVehicleWheelLocation WheelLocation, float AngleInDeg) 
{
  if (bPhysicsEnabled == false)
  {
    check((uint8)WheelLocation >= 0)
    UVehicleAnimationInstance *VehicleAnim = Cast<UVehicleAnimationInstance>(GetMesh()->GetAnimInstance());
    check(VehicleAnim != nullptr)
    // ToDo We need to investigate about this
    //VehicleAnim->GetWheelAnimData()SetWheelRotYaw((uint8)WheelLocation, AngleInDeg);
  }
  else
  {
    UE_LOG(LogTemp, Warning, TEXT("Cannot set wheel steer direction. Physics are enabled."))
  }
}

float ACarlaWheeledVehicle::GetWheelSteerAngle(EVehicleWheelLocation WheelLocation) {

#if 0 // @CARLAUE5     // ToDo We need to investigate about this
  check((uint8)WheelLocation >= 0)
  UVehicleAnimationInstance *VehicleAnim = Cast<UVehicleAnimationInstance>(GetMesh()->GetAnimInstance());
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
#else
    return 0.0F;
#endif
}

void ACarlaWheeledVehicle::SetSimulatePhysics(bool enabled) {
  if(!GetCarlaMovementComponent<UDefaultMovementComponent>())
  {
    return;
  }

  UChaosWheeledVehicleMovementComponent* Movement = GetChaosWheeledVehicleMovementComponent();
  if (Movement)
  {
    check(Movement != nullptr);

    if(bPhysicsEnabled == enabled)
      return;

    SetActorEnableCollision(true);
    UPrimitiveComponent* RootPrimitive =
        Cast<UPrimitiveComponent>(GetRootComponent());
    RootPrimitive->SetSimulatePhysics(enabled);
    RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    UVehicleAnimationInstance *VehicleAnim = Cast<UVehicleAnimationInstance>(GetMesh()->GetAnimInstance());
    check(VehicleAnim != nullptr)

    if (enabled)
    {
      Movement->RecreatePhysicsState();
      //VehicleAnim->ResetWheelCustomRotations();
    }
    else
    {
      Movement->DestroyPhysicsState();
    }
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
  //ShowDebugTelemetry(false);
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

  RecordDoorChange(DoorIdx, true);
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
  RecordDoorChange(DoorIdx, false);
}

void ACarlaWheeledVehicle::RecordDoorChange(const EVehicleDoor DoorIdx, bool bIsOpen)
{
  auto * Recorder = UCarlaStatics::GetRecorder(GetWorld());
  if (Recorder && Recorder->IsEnabled())
  {
      Recorder->AddVehicleDoor(*this, DoorIdx, bIsOpen);
  }
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
    auto Root = Cast<UPrimitiveComponent>(GetRootComponent());
    auto AngularVelocity = Root->GetPhysicsAngularVelocityInDegrees();
    Root->SetPhysicsAngularVelocityInDegrees((1 - RolloverBehaviorForce) * AngularVelocity);
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

