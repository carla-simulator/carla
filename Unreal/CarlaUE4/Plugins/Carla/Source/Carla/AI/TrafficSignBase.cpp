// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

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
