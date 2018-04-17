// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "VehicleAgentComponent.h"

static bool IsPlayer(const ACarlaWheeledVehicle &InVehicle)
{
  auto *Controller = Cast<AWheeledVehicleAIController>(InVehicle.GetController());
  return (Controller != nullptr) && Controller->IsPossessingThePlayer();
}

UVehicleAgentComponent::UVehicleAgentComponent(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer) {}

void UVehicleAgentComponent::BeginPlay()
{
  Vehicle = Cast<ACarlaWheeledVehicle>(GetOwner());
  checkf(Vehicle != nullptr, TEXT("UVehicleAgentComponent can only be attached to ACarlaWheeledVehicle"));

  // We only want to register non-player agents.
  bRegisterAgentComponent = !IsPlayer(*Vehicle);

  Super::BeginPlay();
}
