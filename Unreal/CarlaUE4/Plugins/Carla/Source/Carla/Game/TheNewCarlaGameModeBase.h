// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorSpawner.h"
#include "Carla/Game/CarlaEpisode.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "TheNewCarlaGameModeBase.generated.h"

/// Base class for the CARLA Game Mode.
UCLASS(HideCategories=(ActorTick))
class CARLA_API ATheNewCarlaGameModeBase : public AGameModeBase
{
  GENERATED_BODY()

public:

  ATheNewCarlaGameModeBase(const FObjectInitializer& ObjectInitializer);

protected:

  void InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) override;

  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  void Tick(float DeltaSeconds) override;

private:

  void SpawnActorSpawners();

  UPROPERTY()
  UCarlaGameInstance *GameInstance = nullptr;

  UPROPERTY()
  UCarlaEpisode *Episode = nullptr;

  /// List of actor spawners that will be used to define and spawn the actors
  /// available in game.
  UPROPERTY(EditAnywhere)
  TSet<TSubclassOf<AActorSpawner>> ActorSpawners;

  UPROPERTY()
  TArray<AActorSpawner *> ActorSpawnerInstances;
};
