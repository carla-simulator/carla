// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/GameModeBase.h"
#include "CarlaGameControllerBase.h"
#include "CarlaGameMode.generated.h"

class APlayerStart;
class ASceneCaptureCamera;
class UTaggerDelegate;

/**
 *
 */
UCLASS()
class CARLA_API ACarlaGameMode : public AGameModeBase
{
  GENERATED_BODY()

public:

  ACarlaGameMode(const FObjectInitializer& ObjectInitializer);

  virtual void InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) override;

  virtual void RestartPlayer(AController *NewPlayer) override;

  virtual void BeginPlay() override;

  virtual void Tick(float DeltaSeconds) override;

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
};
