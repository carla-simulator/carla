// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BaseCarlaMovementComponent.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

void UBaseCarlaMovementComponent::BeginPlay()
{
  Super::BeginPlay();
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(GetOwner());
  if (Vehicle)
  {
    CarlaVehicle = Vehicle;
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT("Error: Owner is not properly set for UBaseCarlaMovementComponent") );
  }
}

void UBaseCarlaMovementComponent::ProcessControl(FVehicleControl &Control)
{

}

FVector UBaseCarlaMovementComponent::GetVelocity() const
{
  if (CarlaVehicle)
  {
    return CarlaVehicle->AWheeledVehicle::GetVelocity();
  }
  return FVector();
}

int32 UBaseCarlaMovementComponent::GetVehicleCurrentGear() const
{
  return 0;
}

float UBaseCarlaMovementComponent::GetVehicleForwardSpeed() const
{
  return 0.f;
}
