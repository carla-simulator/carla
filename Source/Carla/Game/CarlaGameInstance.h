// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "Engine/GameInstance.h"
#include "CarlaGameControllerBase.h"
#include "CarlaGameInstance.generated.h"

/**
 *
 */
UCLASS()
class CARLA_API UCarlaGameInstance : public UGameInstance
{
  GENERATED_BODY()

public:

  ~UCarlaGameInstance();

  void InitializeGameControllerIfNotPresent(bool bUseMockController);

  CarlaGameControllerBase &GetGameController()
  {
    check(GameController != nullptr);
    return *GameController;
  }

private:

  TUniquePtr<CarlaGameControllerBase> GameController;
};
