// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "Engine/GameInstance.h"
#include "CarlaGameControllerBase.h"
#include "CarlaGameInstance.generated.h"

class UCarlaSettings;
struct FMockGameControllerSettings;

/// The game instance contains elements that must be kept alive in between
/// levels. It is instantiate once per game.
UCLASS()
class CARLA_API UCarlaGameInstance : public UGameInstance
{
  GENERATED_BODY()

public:

  UCarlaGameInstance();

  ~UCarlaGameInstance();

  void InitializeGameControllerIfNotPresent(
      const FMockGameControllerSettings &MockControllerSettings);

  CarlaGameControllerBase &GetGameController()
  {
    check(GameController != nullptr);
    return *GameController;
  }

  UCarlaSettings &GetCarlaSettings()
  {
    check(CarlaSettings != nullptr);
    return *CarlaSettings;
  }

  const UCarlaSettings &GetCarlaSettings() const
  {
    check(CarlaSettings != nullptr);
    return *CarlaSettings;
  }

  // Extra overload just for blueprints.
  UFUNCTION(BlueprintCallable)
  UCarlaSettings *GetCARLASettings()
  {
    return CarlaSettings;
  }

private:

  UPROPERTY(Category = "CARLA Settings", EditAnywhere)
  UCarlaSettings *CarlaSettings;

  TUniquePtr<CarlaGameControllerBase> GameController;
};
