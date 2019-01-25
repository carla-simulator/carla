// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"
#include "AgentComponent.h"
#include "Engine/World.h"
#include "Game/CarlaGameModeBase.h"
#include "Game/DataRouter.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

static FDataRouter *GetDataRouter(UWorld *World)
{
  check(World != nullptr);
  ACarlaGameModeBase *GameMode = Cast<ACarlaGameModeBase>(World->GetAuthGameMode());
  return GameMode != nullptr ? &GameMode->GetDataRouter() : nullptr;
}

UAgentComponent::UAgentComponent(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  bVisible = false;
  bHiddenInGame = true;
  SetShouldUpdatePhysicsVolume(false);
  PrimaryComponentTick.bCanEverTick = false;
}

void UAgentComponent::AcceptVisitor(IAgentComponentVisitor &Visitor) const
{
  unimplemented();
}

void UAgentComponent::BeginPlay()
{
  Super::BeginPlay();

  if (bRegisterAgentComponent)
  {
    /**
      * This only returns true if the current game mode is not null
      * because you can only access a game mode if you are the host
      * @param oftheworld UWorld is needed to access the game mode
      * @return true if there is a game mode and it is not null
     */
    if(UGameplayStatics::GetGameMode(GetWorld())!=nullptr)
    {
      auto *DataRouter = GetDataRouter(GetWorld());
      if (DataRouter != nullptr)
      {
        DataRouter->RegisterAgent(this);
      }
    } else
    {
        UCarlaGameInstance* GameInstance = Cast<UCarlaGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
        if(GameInstance)    GameInstance->GetDataRouter().RegisterAgent(this);
    }
    bAgentComponentIsRegistered = true;
  }
}

void UAgentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  if (bAgentComponentIsRegistered)
  {
    FDataRouter *DataRouter = nullptr;
    if(UGameplayStatics::GetGameMode(GetWorld())!=nullptr)
    {
      DataRouter = GetDataRouter(GetWorld());
    }
    else
    {
      UCarlaGameInstance *GameInstance = Cast<UCarlaGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
      if(GameInstance)
      {
        DataRouter = &GameInstance->GetDataRouter();
      }
    }
    if (DataRouter != nullptr)
    {
      DataRouter->DeregisterAgent(this);
    }
    bAgentComponentIsRegistered = false;
  }

  Super::EndPlay(EndPlayReason);
}

