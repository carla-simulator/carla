// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameInstance.h"

#include "CarlaGameController.h"
#include "MockGameController.h"

UCarlaGameInstance::~UCarlaGameInstance() {}

void UCarlaGameInstance::InitializeGameControllerIfNotPresent(bool bUseMockController)
{
  if (GameController == nullptr) {
    if (bUseMockController) {
      GameController = MakeUnique<MockGameController>();
      UE_LOG(LogCarla, Warning, TEXT("Using mock CARLA controller"));
    } else {
      GameController = MakeUnique<CarlaGameController>();
    }
  }
}
