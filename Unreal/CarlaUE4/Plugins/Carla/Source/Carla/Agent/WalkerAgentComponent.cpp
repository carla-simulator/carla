// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "WalkerAgentComponent.h"

#include "GameFramework/Character.h"

UWalkerAgentComponent::UWalkerAgentComponent(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer) {}

float UWalkerAgentComponent::GetForwardSpeed() const
{
  /// @todo Is it necessary to compute this speed every tick?
  return FVector::DotProduct(Walker->GetVelocity(), Walker->GetActorRotation().Vector()) * 0.036f;
}

void UWalkerAgentComponent::BeginPlay()
{
  Walker = Cast<ACharacter>(GetOwner());
  checkf(Walker != nullptr, TEXT("UWalkerAgentComponent can only be attached to ACharacter"));

  Super::BeginPlay();
}
