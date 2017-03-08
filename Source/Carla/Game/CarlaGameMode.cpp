// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameMode.h"

#include "CarlaGameState.h"
#include "CarlaPlayerState.h"
#include "CarlaServerController.h"

ACarlaGameMode::ACarlaGameMode()
{
  PlayerControllerClass = ACarlaServerController::StaticClass();
  GameStateClass = ACarlaGameState::StaticClass();
  PlayerStateClass = ACarlaPlayerState::StaticClass();
}
