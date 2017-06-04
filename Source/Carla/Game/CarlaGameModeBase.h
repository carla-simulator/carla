// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/GameModeBase.h"
#include "AI/VehicleSpawnerBase.h"
#include "AI/WalkerSpawnerBase.h"
#include "CarlaGameControllerBase.h"
#include "DynamicWeather.h"
#include "MockGameControllerSettings.h"
#include "CarlaGameModeBase.generated.h"

class APlayerStart;
class ASceneCaptureCamera;
class UTaggerDelegate;

/**
 *
 */
UCLASS(HideCategories=(ActorTick))
class CARLA_API ACarlaGameModeBase : public AGameModeBase
{
  GENERATED_BODY()

public:

  ACarlaGameModeBase(const FObjectInitializer& ObjectInitializer);

  virtual void InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) override;

  virtual void RestartPlayer(AController *NewPlayer) override;

  virtual void BeginPlay() override;

  virtual void Tick(float DeltaSeconds) override;

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

  void AttachCaptureCamerasToPlayer(AController &Player);

  void TagActorsForSemanticSegmentation();

  /// Iterate all the APlayerStart present in the world and add the ones with
  /// unoccupied locations to @a UnOccupiedStartPoints.
  ///
  /// @return APlayerStart if "Play from Here" was used while in PIE mode.
  APlayerStart *FindUnOccupiedStartPoints(
      AController *Player,
      TArray<APlayerStart *> &UnOccupiedStartPoints);

  CarlaGameControllerBase *GameController;

  UPROPERTY()
  UCarlaGameInstance *GameInstance;

  UPROPERTY()
  AController *PlayerController;

  UPROPERTY()
  UTaggerDelegate *TaggerDelegate;

  UPROPERTY()
  ADynamicWeather *DynamicWeather;

  UPROPERTY()
  AVehicleSpawnerBase *VehicleSpawner;

  UPROPERTY()
  AWalkerSpawnerBase *WalkerSpawner;
};
