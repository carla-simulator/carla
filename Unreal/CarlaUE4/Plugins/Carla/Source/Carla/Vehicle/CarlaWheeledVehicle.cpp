// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CarlaWheeledVehicle.h"

#include "Agent/VehicleAgentComponent.h"

#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"

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

  VehicleAgentComponent = CreateDefaultSubobject<UVehicleAgentComponent>(TEXT("VehicleAgentComponent"));
  VehicleAgentComponent->SetupAttachment(RootComponent);

  GetVehicleMovementComponent()->bReverseAsBrake = false;
}

ACarlaWheeledVehicle::~ACarlaWheeledVehicle() {}

// =============================================================================
// -- Get functions ------------------------------------------------------------
// =============================================================================

float ACarlaWheeledVehicle::GetVehicleForwardSpeed() const
{
  return GetVehicleMovementComponent()->GetForwardSpeed();
}

FVector ACarlaWheeledVehicle::GetVehicleOrientation() const
{
  return GetVehicleTransform().GetRotation().GetForwardVector();
}

int32 ACarlaWheeledVehicle::GetVehicleCurrentGear() const
{
  return GetVehicleMovementComponent()->GetCurrentGear();
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

void ACarlaWheeledVehicle::ApplyVehicleControl(const FVehicleControl &VehicleControl)
{
  auto *MovementComponent = GetVehicleMovementComponent();
  MovementComponent->SetThrottleInput(VehicleControl.Throttle);
  MovementComponent->SetSteeringInput(VehicleControl.Steer);
  MovementComponent->SetBrakeInput(VehicleControl.Brake);
  MovementComponent->SetHandbrakeInput(VehicleControl.bHandBrake);
  if (Control.bReverse != VehicleControl.bReverse)
  {
    MovementComponent->SetUseAutoGears(!VehicleControl.bReverse);
    MovementComponent->SetTargetGear(VehicleControl.bReverse ? -1 : 1, true);
  }
  else
  {
    MovementComponent->SetUseAutoGears(!VehicleControl.bManualGearShift);
    if (VehicleControl.bManualGearShift)
    {
      MovementComponent->SetTargetGear(VehicleControl.Gear, true);
    }
  }
  Control = VehicleControl;
  Control.Gear = MovementComponent->GetCurrentGear();
  Control.bReverse = Control.Gear < 0;
}

void ACarlaWheeledVehicle::SetThrottleInput(const float Value)
{
  GetVehicleMovementComponent()->SetThrottleInput(Value);
  Control.Throttle = Value;
}

void ACarlaWheeledVehicle::SetSteeringInput(const float Value)
{
  GetVehicleMovementComponent()->SetSteeringInput(Value);
  Control.Steer = Value;
}

void ACarlaWheeledVehicle::SetBrakeInput(const float Value)
{
  GetVehicleMovementComponent()->SetBrakeInput(Value);
  Control.Brake = Value;
}

void ACarlaWheeledVehicle::SetReverse(const bool Value)
{
  if (Value != Control.bReverse) {
    Control.bReverse = Value;
    auto MovementComponent = GetVehicleMovementComponent();
    MovementComponent->SetUseAutoGears(!Control.bReverse);
    MovementComponent->SetTargetGear(Control.bReverse ? -1 : 1, true);
  }
}

void ACarlaWheeledVehicle::SetHandbrakeInput(const bool Value)
{
  GetVehicleMovementComponent()->SetHandbrakeInput(Value);
  Control.bHandBrake = Value;
}

FVehiclePhysicsControl ACarlaWheeledVehicle::GetVehiclePhysicsControl()
{  
  UCarlaWheeledVehicleMovementComponent4W *Vehicle4W = CastChecked<UCarlaWheeledVehicleMovementComponent4W>(GetVehicleMovement());

  FVehiclePhysicsControl PhysicsControl;

  // Engine Setup
  PhysicsControl.TorqueCurve = Vehicle4W->EngineSetup.TorqueCurve.EditorCurveData;
  PhysicsControl.MaxRPM = Vehicle4W->EngineSetup.MaxRPM;
  PhysicsControl.MOI = Vehicle4W->EngineSetup.MOI;
  PhysicsControl.DampingRateFullThrottle = Vehicle4W->EngineSetup.DampingRateFullThrottle;
  PhysicsControl.DampingRateZeroThrottleClutchEngaged = Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchEngaged;
  PhysicsControl.DampingRateZeroThrottleClutchDisengaged = Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchDisengaged;

  // Transmission Setup
  PhysicsControl.bUseGearAutoBox = Vehicle4W->TransmissionSetup.bUseGearAutoBox;
  PhysicsControl.GearSwitchTime = Vehicle4W->TransmissionSetup.GearSwitchTime;
  PhysicsControl.ClutchStrength = Vehicle4W->TransmissionSetup.ClutchStrength;

  // Vehicle Setup
  PhysicsControl.Mass = Vehicle4W->Mass;
  PhysicsControl.DragCoefficient = Vehicle4W->DragCoefficient;
  PhysicsControl.InertiaTensorScale = Vehicle4W->InertiaTensorScale;

  return PhysicsControl;
}

void ACarlaWheeledVehicle::SetVehiclePhysicsControl(const FVehiclePhysicsControl &PhysicsControl)
{
  UCarlaWheeledVehicleMovementComponent4W *Vehicle4W = CastChecked<UCarlaWheeledVehicleMovementComponent4W>(GetVehicleMovement());

  // Engine Setup
  Vehicle4W->EngineSetup.TorqueCurve.EditorCurveData = PhysicsControl.TorqueCurve;
  Vehicle4W->EngineSetup.MaxRPM = PhysicsControl.MaxRPM;

  Vehicle4W->EngineSetup.MOI = PhysicsControl.MOI;
  
  Vehicle4W->EngineSetup.DampingRateFullThrottle = PhysicsControl.DampingRateFullThrottle;
  Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchEngaged = PhysicsControl.DampingRateZeroThrottleClutchEngaged;
  Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchDisengaged = PhysicsControl.DampingRateZeroThrottleClutchDisengaged;

  Vehicle4W->ApplyEngineSetup(Vehicle4W->EngineSetup);

  // Transmission Setup
  Vehicle4W->TransmissionSetup.bUseGearAutoBox = PhysicsControl.bUseGearAutoBox;
  Vehicle4W->TransmissionSetup.GearSwitchTime = PhysicsControl.GearSwitchTime;
  Vehicle4W->TransmissionSetup.ClutchStrength = PhysicsControl.ClutchStrength;

  Vehicle4W->ApplyTransmissionSetup(Vehicle4W->TransmissionSetup);

  // Vehicle Setup
  Vehicle4W->Mass = PhysicsControl.Mass;
  Vehicle4W->DragCoefficient = PhysicsControl.DragCoefficient;
  Vehicle4W->InertiaTensorScale = PhysicsControl.InertiaTensorScale;
}