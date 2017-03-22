// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "MockGameController.h"

APlayerStart *MockGameController::ChoosePlayerStart(
    const TArray<APlayerStart *> &AvailableStartSpots)
{
  return AvailableStartSpots[0u];
}

void MockGameController::RegisterPlayer(AController &NewPlayer)
{
  ACarlaVehicleController *VehicleController = Cast<ACarlaVehicleController>(&NewPlayer);
  if (VehicleController != nullptr) {
    VehicleController->SetManualMode(true);
  } else {
    UE_LOG(LogCarla, Warning, TEXT("Player is not a ACarlaVehicleController"));
  }
}

void MockGameController::RegisterCaptureCamera(const ASceneCaptureCamera &CaptureCamera)
{

}

void MockGameController::Tick(float DeltaSeconds)
{

}
