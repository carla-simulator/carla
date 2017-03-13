// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "MockGameController.h"

MockGameController::MockGameController()
{
  bCanEverTick = false;
}

APlayerStart *MockGameController::ChoosePlayerStart(
    const TArray<APlayerStart *> &AvailableStartSpots)
{
  return AvailableStartSpots[0u];
}

void MockGameController::RegisterPlayer(AController *NewPlayer)
{
  ACarlaVehicleController *VehicleController = Cast<ACarlaVehicleController>(NewPlayer);
  if (VehicleController != nullptr) {
    if (!VehicleController->IsInManualMode())
      VehicleController->SetManualMode(true);
  } else {
    UE_LOG(LogCarla, Warning, TEXT("Player is not a ACarlaVehicleController"));
  }
}
