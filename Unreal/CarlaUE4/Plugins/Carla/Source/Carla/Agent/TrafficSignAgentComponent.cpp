// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "TrafficSignAgentComponent.h"

#include "Traffic/TrafficSignBase.h"

UTrafficSignAgentComponent::UTrafficSignAgentComponent(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer) {}

void UTrafficSignAgentComponent::BeginPlay()
{
  TrafficSign = Cast<ATrafficSignBase>(GetOwner());
  checkf(TrafficSign != nullptr, TEXT("UTrafficSignAgentComponent can only be attached to ATrafficSignBase"));

  Super::BeginPlay();
}
