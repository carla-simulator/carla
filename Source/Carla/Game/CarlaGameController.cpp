// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameController.h"

CarlaGameController::CarlaGameController()
{
  bCanEverTick = true;
}

APlayerStart *CarlaGameController::ChoosePlayerStart(
    const TArray<APlayerStart *> &AvailableStartSpots)
{
  UE_LOG(LogCarla, Error, TEXT("Not implemented"));
  return AvailableStartSpots[0u];
}

void CarlaGameController::RegisterPlayer(AController *NewPlayer)
{
  UE_LOG(LogCarla, Error, TEXT("Not implemented"));
}

void CarlaGameController::Tick(float DeltaSeconds)
{
  UE_LOG(LogCarla, Error, TEXT("Not implemented"));
}
