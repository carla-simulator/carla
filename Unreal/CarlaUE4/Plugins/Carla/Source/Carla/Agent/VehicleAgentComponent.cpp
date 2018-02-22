// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "VehicleAgentComponent.h"

#include "WheeledVehicle.h"

UVehicleAgentComponent::UVehicleAgentComponent(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer) {}

void UVehicleAgentComponent::OnComponentCreated()
{
  Super::OnComponentCreated();

  WheeledVehicle = Cast<AWheeledVehicle>(GetOwner());
  checkf(WheeledVehicle != nullptr, TEXT("UVehicleAgentComponent can only be attached to AWheeledVehicle"));
}
