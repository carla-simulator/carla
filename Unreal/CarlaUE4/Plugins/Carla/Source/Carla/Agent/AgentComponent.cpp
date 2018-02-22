// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "AgentComponent.h"

#include "Agent/AgentMap.h"
#include "Game/CarlaGameModeBase.h"
#include "Game/DataRouter.h"

static uint32 GetNextId()
{
  static uint32 COUNT = 0u;
  return ++COUNT;
}

static TSharedPtr<FAgentMap> GetAgentMap(UWorld *World)
{
  check(World != nullptr);
  auto *GameMode = Cast<ACarlaGameModeBase>(World->GetAuthGameMode());
  check(GameMode != nullptr);
  return GameMode->GetDataRouter().GetAgents();
}

UAgentComponent::UAgentComponent(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer),
    Id(GetNextId()) {}

void UAgentComponent::AcceptVisitor(IAgentComponentVisitor &Visitor) const
{
  unimplemented();
}

void UAgentComponent::BeginPlay()
{
  Super::BeginPlay();

  // Register this component in the World's list of agents.
  auto AgentMapPtr = GetAgentMap(GetWorld());
  if (AgentMapPtr.IsValid()) {
    AgentMapPtr->Agents.Add(Id, this);
    AgentMap = AgentMapPtr;
  } else {
    UE_LOG(LogCarla, Error, TEXT("AgentComponent: Missing AgentMap!"));
  }
}

void UAgentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  // Deregister this component in the World's list of agents.
  auto AgentMapPtr = AgentMap.Pin();
  if (AgentMapPtr.IsValid()) {
    AgentMapPtr->Agents.Remove(Id);
  }

  Super::EndPlay(EndPlayReason);
}
