// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "LargeMapTileManager.h"
#include "LargeMapActorManager.h"

#include "LargeMapManager.generated.h"



UCLASS()
class CARLA_API ALargeMapManager : public AActor
{
  GENERATED_BODY()

public:
  ALargeMapManager();

  ~ALargeMapManager();

protected:
  virtual void BeginPlay() override;

  void PostWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin);

  void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);

  void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

public:

  void OnActorSpawned(const FActorView& ActorView);

  virtual void Tick(float DeltaTime) override;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void GenerateMap(const FString& InAssetsPath);

  void AddActor(const FActorView& ActorView, const FTransform& Transform);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void AddActorToConsider(AActor* InActor);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void RemoveActorToConsider(AActor* InActor);

  void EvaluatePossibleRebase();

private:

  TArray<FDVector> ActorsLocation;

  // Components
  ULargeMapTileManager* TileManager = nullptr;

  ULargeMapActorManager* ActorManager = nullptr;

  // Current Origin after rebase
  FIntVector CurrentOriginInt{ 0 };
  FDVector CurrentOriginD;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float RebaseOriginDistance = 2.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float TickInterval = 0.0f;

#if WITH_EDITOR

  void PrintMapInfo();

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintMapInfo = true;

#endif // WITH_EDITOR

};
