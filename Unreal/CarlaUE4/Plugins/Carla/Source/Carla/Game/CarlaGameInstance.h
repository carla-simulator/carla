// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Engine/GameInstance.h"

#include "Carla/Game/CarlaGameControllerBase.h"
#include "Carla/Game/DataRouter.h"
#include "Carla/Server/TheNewCarlaServer.h"

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

  /// Starts the Carla server if not already running.
  void StartServer();

  ICarlaGameControllerBase &GetGameController()
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

  UFUNCTION(BlueprintCallable)
  UCarlaEpisode *GetCarlaEpisode()
  {
    return CurrentEpisode;
  }

  FDataRouter &GetDataRouter()
  {
    return DataRouter;
  }

  void NotifyBeginEpisode(UCarlaEpisode &Episode)
  {
    CurrentEpisode = &Episode;
    Server.NotifyBeginEpisode(Episode);
  }

  void Tick(float /*DeltaSeconds*/)
  {
    Server.RunSome(10u); /// @todo
  }

  void NotifyEndEpisode()
  {
    Server.NotifyEndEpisode();
    CurrentEpisode = nullptr;
  }

  const FTheNewCarlaServer &GetServer() const
  {
    return Server;
  }

private:

  UPROPERTY(VisibleAnywhere)
  bool bServerIsRunning = false;

  UPROPERTY(Category = "CARLA Settings", EditAnywhere)
  UCarlaSettings *CarlaSettings = nullptr;

  UPROPERTY()
  UCarlaEpisode *CurrentEpisode = nullptr;

  FDataRouter DataRouter;

  TUniquePtr<ICarlaGameControllerBase> GameController;

  FTheNewCarlaServer Server;
};
