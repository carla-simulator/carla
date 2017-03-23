// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/GameModeBase.h"
#include "CarlaGameControllerBase.h"
#include "CarlaGameMode.generated.h"

class APlayerStart;
class ASceneCaptureCamera;

/**
 *
 */
UCLASS()
class CARLA_API ACarlaGameMode : public AGameModeBase
{
  GENERATED_BODY()

  /** Use a mock controller instead of connecting to a client */
  UPROPERTY(Category = "CARLA Game Controller", EditAnywhere)
  bool bUseMockController = false;

public:

  ACarlaGameMode();

  virtual void InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) override;

  virtual void RestartPlayer(AController *NewPlayer) override;

  virtual void BeginPlay() override;

  virtual void Tick(float DeltaSeconds) override;

  void RegisterCaptureCamera(ASceneCaptureCamera &CaptureCamera);

private:

  void RegisterPlayer(AController &NewPlayer);

  /// Iterate all the APlayerStart present in the world and add the ones with
  /// unoccupied locations to @a UnOccupiedStartPoints.
  ///
  /// @return APlayerStart if "Play from Here" was used while in PIE mode.
  APlayerStart *FindUnOccupiedStartPoints(
      AController *Player,
      TArray<APlayerStart *> &UnOccupiedStartPoints);

  CarlaGameControllerBase *GameController;

  AController *PlayerController;
};
