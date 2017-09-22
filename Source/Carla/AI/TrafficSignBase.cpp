// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "TrafficSignBase.h"

#include "Game/CarlaGameState.h"

ATrafficSignBase::ATrafficSignBase() : Super() {}

void ATrafficSignBase::BeginPlay()
{
  auto *GameState = GetWorld()->GetGameState<ACarlaGameState>();
  if (GameState != nullptr) {
    GameState->RegisterTrafficSign(this);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Missing CARLA game state!"));
  }
}
