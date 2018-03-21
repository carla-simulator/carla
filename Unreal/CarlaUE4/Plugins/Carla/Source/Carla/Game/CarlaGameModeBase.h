// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/GameModeBase.h"

#include "DynamicWeather.h"
#include "Game/CarlaGameControllerBase.h"
#include "Game/CarlaGameInstance.h"
#include "Game/MockGameControllerSettings.h"
#include "Vehicle/VehicleSpawnerBase.h"
#include "Walker/WalkerSpawnerBase.h"

#include "CarlaGameModeBase.generated.h"

class ACarlaVehicleController;
class APlayerStart;
class ASceneCaptureCamera;
class UCarlaGameInstance;
class UTaggerDelegate;
class UCarlaSettingsDelegate;
UCLASS(HideCategories=(ActorTick))
class CARLA_API ACarlaGameModeBase : public AGameModeBase
{
  GENERATED_BODY()

public:

  ACarlaGameModeBase(const FObjectInitializer& ObjectInitializer);

  virtual void InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) override;

  virtual void RestartPlayer(AController *NewPlayer) override;

  virtual void BeginPlay() override;

  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  virtual void Tick(float DeltaSeconds) override;

  FDataRouter &GetDataRouter()
  {
    check(GameInstance != nullptr);
    return GameInstance->GetDataRouter();
  }

  UFUNCTION(BlueprintPure, Category="CARLA Settings")
  UCarlaSettingsDelegate *GetCARLASettingsDelegate()
  {
    return CarlaSettingsDelegate;
  }

protected:

  /** Used only when networking is disabled. */
  UPROPERTY(Category = "Mock CARLA Controller", EditAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Mock CARLA Controller"))
  FMockGameControllerSettings MockGameControllerSettings;

  /** The class of DynamicWeather to spawn. */
  UPROPERTY(Category = "CARLA Classes", EditAnywhere, BlueprintReadOnly)
  TSubclassOf<ADynamicWeather> DynamicWeatherClass;

  /** The class of VehicleSpawner to spawn. */
  UPROPERTY(Category = "CARLA Classes", EditAnywhere, BlueprintReadOnly)
  TSubclassOf<AVehicleSpawnerBase> VehicleSpawnerClass;

  /** The class of WalkerSpawner to spawn. */
  UPROPERTY(Category = "CARLA Classes", EditAnywhere, BlueprintReadOnly)
  TSubclassOf<AWalkerSpawnerBase> WalkerSpawnerClass;

private:

  void RegisterPlayer(AController &NewPlayer);

  void AttachSensorsToPlayer();

  void TagActorsForSemanticSegmentation();

  /// Iterate all the APlayerStart present in the world and add the ones with
  /// unoccupied locations to @a UnOccupiedStartPoints.
  ///
  /// @return APlayerStart if "Play from Here" was used while in PIE mode.
  APlayerStart *FindUnOccupiedStartPoints(
      AController *Player,
      TArray<APlayerStart *> &UnOccupiedStartPoints);

  ICarlaGameControllerBase *GameController;

  UPROPERTY()
  UCarlaGameInstance *GameInstance;

  UPROPERTY()
  ACarlaVehicleController *PlayerController;

  UPROPERTY()
  UTaggerDelegate *TaggerDelegate;

  UPROPERTY()
  UCarlaSettingsDelegate* CarlaSettingsDelegate;

  UPROPERTY()
  ADynamicWeather *DynamicWeather;

  UPROPERTY()
  AVehicleSpawnerBase *VehicleSpawner;

  UPROPERTY()
  AWalkerSpawnerBase *WalkerSpawner;
};
