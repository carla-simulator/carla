// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "Engine/GameInstance.h"
#include "CarlaGameControllerBase.h"
#include "CarlaGameInstance.generated.h"

class UCarlaSettings;

/**
 *
 */
UCLASS()
class CARLA_API UCarlaGameInstance : public UGameInstance
{
  GENERATED_BODY()

public:

  UCarlaGameInstance();

  ~UCarlaGameInstance();

  void InitializeGameControllerIfNotPresent();

  CarlaGameControllerBase &GetGameController()
  {
    check(GameController != nullptr);
    return *GameController;
  }

  const UCarlaSettings &GetCarlaSettings() const
  {
    check(CarlaSettings != nullptr);
    return *CarlaSettings;
  }

private:

  UPROPERTY(Category = "CARLA Settings", EditAnywhere)
  UCarlaSettings *CarlaSettings;

  TUniquePtr<CarlaGameControllerBase> GameController;
};
