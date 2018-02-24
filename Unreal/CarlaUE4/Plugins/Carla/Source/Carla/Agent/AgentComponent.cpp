// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "AgentComponent.h"

#include "Game/CarlaGameModeBase.h"
#include "Game/DataRouter.h"

static uint32 GetNextAgentId()
{
  static uint32 COUNT = 0u;
  return ++COUNT;
}

static FDataRouter &GetDataRouter(UWorld *World)
{
  check(World != nullptr);
  auto *GameMode = Cast<ACarlaGameModeBase>(World->GetAuthGameMode());
  check(GameMode != nullptr);
  return GameMode->GetDataRouter();
}

UAgentComponent::UAgentComponent(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer),
    Id(GetNextAgentId()) {}

void UAgentComponent::AcceptVisitor(IAgentComponentVisitor &Visitor) const
{
  unimplemented();
}

void UAgentComponent::BeginPlay()
{
  Super::BeginPlay();
  GetDataRouter(GetWorld()).RegisterAgent(this);
}

void UAgentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  GetDataRouter(GetWorld()).DeregisterAgent(this);
  Super::EndPlay(EndPlayReason);
}
