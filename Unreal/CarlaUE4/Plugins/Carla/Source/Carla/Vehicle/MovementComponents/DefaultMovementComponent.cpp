// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "DefaultMovementComponent.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

void UDefaultMovementComponent::CreateDefaultMovementComponent(ACarlaWheeledVehicle* Vehicle)
{
  UDefaultMovementComponent* DefaultMovementComponent = NewObject<UDefaultMovementComponent>(Vehicle);
  DefaultMovementComponent->RegisterComponent();
  Vehicle->SetCarlaMovementComponent(DefaultMovementComponent);
}

void UDefaultMovementComponent::BeginPlay()
{
  Super::BeginPlay();
}

void UDefaultMovementComponent::ProcessControl(FVehicleControl &Control)
{
  auto *MovementComponent = CarlaVehicle->GetVehicleMovementComponent();
  MovementComponent->SetThrottleInput(Control.Throttle);
  MovementComponent->SetSteeringInput(Control.Steer);
  MovementComponent->SetBrakeInput(Control.Brake);
  MovementComponent->SetHandbrakeInput(Control.bHandBrake);
  if (CarlaVehicle->GetVehicleControl().bReverse != Control.bReverse)
  {
    MovementComponent->SetUseAutoGears(!Control.bReverse);
    MovementComponent->SetTargetGear(Control.bReverse ? -1 : 1, true);
  }
  else
  {
    MovementComponent->SetUseAutoGears(!Control.bManualGearShift);
    if (Control.bManualGearShift)
    {
      MovementComponent->SetTargetGear(Control.Gear, true);
    }
  }
  Control.Gear = MovementComponent->GetCurrentGear();
}

// FVector GetVelocity() const override;

int32 UDefaultMovementComponent::GetVehicleCurrentGear() const
{
  return CarlaVehicle->GetVehicleMovementComponent()->GetCurrentGear();
}

float UDefaultMovementComponent::GetVehicleForwardSpeed() const
{
  return CarlaVehicle->GetVehicleMovementComponent()->GetForwardSpeed();
}
