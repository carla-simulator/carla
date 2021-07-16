// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Game/CarlaStatics.h"

#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"
#include "PhysXPublic.h"
#include "PhysXVehicleManager.h"
#include "TireConfig.h"
#include "VehicleWheel.h"
#include "Carla/Util/ActorAttacher.h"
#include "Carla/Util/EmptyActor.h"
#include "MovementComponents/DefaultMovementComponent.h"
#include "Carla/Trigger/FrictionTrigger.h"
#include "Carla/Util/BoundingBoxCalculator.h"

#include "Rendering/SkeletalMeshRenderData.h"

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

void ACarlaWheeledVehicle::BeginPlay()
{
  Super::BeginPlay();

  UDefaultMovementComponent::CreateDefaultMovementComponent(this);

  float FrictionScale = 3.5f;

  UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(
      GetVehicleMovementComponent());
  check(Vehicle4W != nullptr);

  // Setup Tire Configs with default value. This is needed to avoid getting
  // friction values of previously created TireConfigs for the same vehicle
  // blueprint.
  TArray<float> OriginalFrictions;
  OriginalFrictions.Init(FrictionScale, Vehicle4W->Wheels.Num());
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
  TArray<FWheelSetup> NewWheelSetups = Vehicle4W->WheelSetups;

  for (const auto &WheelSetup : NewWheelSetups)
  {
    UVehicleWheel *Wheel = WheelSetup.WheelClass.GetDefaultObject();
    check(Wheel != nullptr);
  }

  Vehicle4W->WheelSetups = NewWheelSetups;

  LastPhysicsControl = GetVehiclePhysicsControl();
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
  UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(
      GetVehicleMovement());
  check(Vehicle4W != nullptr);

  TArray<float> WheelsFrictionScale;
  for (auto &Wheel : Vehicle4W->Wheels)
  {
    WheelsFrictionScale.Add(Wheel->TireConfig->GetFrictionScale());
  }
  return WheelsFrictionScale;
}

void ACarlaWheeledVehicle::SetWheelsFrictionScale(TArray<float> &WheelsFrictionScale)
{
  UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(
      GetVehicleMovement());
  check(Vehicle4W != nullptr);
  check(Vehicle4W->Wheels.Num() == WheelsFrictionScale.Num());

  for (int32 i = 0; i < Vehicle4W->Wheels.Num(); ++i)
  {
    Vehicle4W->Wheels[i]->TireConfig->SetFrictionScale(WheelsFrictionScale[i]);
  }
}

FVehiclePhysicsControl ACarlaWheeledVehicle::GetVehiclePhysicsControl() const
{
  UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(
      GetVehicleMovement());
  check(Vehicle4W != nullptr);

  FVehiclePhysicsControl PhysicsControl;

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

    PhysicsWheel.TireFriction = Vehicle4W->Wheels[i]->TireConfig->GetFrictionScale();
    PhysicsWheel.LatStiffMaxLoad = Vehicle4W->Wheels[i]->LatStiffMaxLoad;
    PhysicsWheel.LatStiffValue = Vehicle4W->Wheels[i]->LatStiffValue;
    PhysicsWheel.LongStiffValue = Vehicle4W->Wheels[i]->LongStiffValue;
    PhysicsWheel.Position = Vehicle4W->Wheels[i]->Location;

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

    Wheel->LatStiffMaxLoad = PhysicsControl.Wheels[i].LatStiffMaxLoad;
    Wheel->LatStiffValue   = PhysicsControl.Wheels[i].LatStiffValue;
    Wheel->LongStiffValue  = PhysicsControl.Wheels[i].LongStiffValue;

    // Assigning new tire config
    Wheel->TireConfig = DuplicateObject<UTireConfig>(Wheel->TireConfig, nullptr);

    // Setting a new value to friction
    Wheel->TireConfig->SetFrictionScale(PhysicsControl.Wheels[i].TireFriction);
  }

  Vehicle4W->WheelSetups = NewWheelSetups;

  for (int32 i = 0; i < PhysicsWheelsNum; ++i)
  {
    PxVehicleWheelData PWheelData = Vehicle4W->PVehicle->mWheelsSimData.getWheelData(i);

    PWheelData.mRadius = PhysicsControl.Wheels[i].Radius;
    PWheelData.mMaxSteer = FMath::DegreesToRadians(PhysicsControl.Wheels[i].MaxSteerAngle);
    PWheelData.mDampingRate = M2ToCm2(PhysicsControl.Wheels[i].DampingRate);
    PWheelData.mMaxBrakeTorque = M2ToCm2(PhysicsControl.Wheels[i].MaxBrakeTorque);
    PWheelData.mMaxHandBrakeTorque = M2ToCm2(PhysicsControl.Wheels[i].MaxHandBrakeTorque);
    Vehicle4W->PVehicle->mWheelsSimData.setWheelData(i, PWheelData);

    Vehicle4W->Wheels[i]->TireConfig->SetFrictionScale(PhysicsControl.Wheels[i].TireFriction);
    Vehicle4W->Wheels[i]->LatStiffMaxLoad = PhysicsControl.Wheels[i].LatStiffMaxLoad;
    Vehicle4W->Wheels[i]->LatStiffValue   = PhysicsControl.Wheels[i].LatStiffValue;
    Vehicle4W->Wheels[i]->LongStiffValue  = PhysicsControl.Wheels[i].LongStiffValue;
  }

  // Recreate Physics State for vehicle setup
  GetWorld()->GetPhysicsScene()->GetPxScene()->lockWrite();
  Vehicle4W->RecreatePhysicsState();
  GetWorld()->GetPhysicsScene()->GetPxScene()->unlockWrite();

  for (int32 i = 0; i < PhysicsWheelsNum; ++i)
  {
    Vehicle4W->Wheels[i]->LatStiffMaxLoad = PhysicsControl.Wheels[i].LatStiffMaxLoad;
    Vehicle4W->Wheels[i]->LatStiffValue   = PhysicsControl.Wheels[i].LatStiffValue;
    Vehicle4W->Wheels[i]->LongStiffValue  = PhysicsControl.Wheels[i].LongStiffValue;
  }

  auto * Recorder = UCarlaStatics::GetRecorder(GetWorld());
  if (Recorder && Recorder->IsEnabled())
  {
    Recorder->AddPhysicsControl(*this);
  }
}

void ACarlaWheeledVehicle::ActivateVelocityControl(const FVector &Velocity)
{
  VelocityControl->Activate(Velocity);
}

void ACarlaWheeledVehicle::DeactivateVelocityControl()
{
  VelocityControl->Deactivate();
}

void ACarlaWheeledVehicle::SetVehicleLightState(const FVehicleLightState &LightState)
{
  InputControl.LightState = LightState;
  RefreshLightState(LightState);
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
    check((uint8)WheelLocation < 4)
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
  check((uint8)WheelLocation < 4)
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

  UWheeledVehicleMovementComponent4W *Vehicle4W = Cast<UWheeledVehicleMovementComponent4W>(
      GetVehicleMovement());
  check(Vehicle4W != nullptr);

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
    Vehicle4W->RecreatePhysicsState();
    VehicleAnim->ResetWheelCustomRotations();
  }
  else 
  {
    Vehicle4W->DestroyPhysicsState();
  }

  GetWorld()->GetPhysicsScene()->GetPxScene()->unlockWrite();

  bPhysicsEnabled = enabled;
}

FVector ACarlaWheeledVehicle::GetVelocity() const
{
  return BaseMovementComponent->GetVelocity();
}
