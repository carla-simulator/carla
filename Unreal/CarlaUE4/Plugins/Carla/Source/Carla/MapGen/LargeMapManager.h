// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "LargeMapTileManager.h"
#include "LargeMapActorManager.h"

#include "LargeMapManager.generated.h"


// TODO: Create custom allocators for FGhostActor

USTRUCT()
struct FActorToConsider
{
  GENERATED_BODY()

  FActorToConsider() {}

  FActorToConsider(AActor* InActor) : Actor(InActor) {}

  AActor* Actor = nullptr;

  // Absolute location = Current Origin + Relative Tile Location
  FDVector Location;

  bool operator==(const FActorToConsider& Other)
  {
    return Actor == Other.Actor &&
           Location == Other.Location;
  }
};

/*
  Actor that was spawned or queried to be spawn at some point but it was so far away
  from the origin that was removed from the level (or not spawned).
  It is possible that the actor keeps receiving updates, eg, traffic manager.
  FGhostActor is a wrapper of the info and state of the actor in case it needs to be re-spawned.
*/
struct FGhostActor
{
  FGhostActor(const FTransform& InTransform, const FActorView& InActorView) :
    Transform(InTransform),
    ActorView(InActorView) {}

  FTransform Transform;

  FActorView ActorView;
};

USTRUCT()
struct FCarlaMapTile
{
  GENERATED_BODY()

#if WITH_EDITOR
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FString Name; // Tile_{TileID_X}_{TileID_Y}
#endif // WITH_EDITOR

  // Absolute location, does not depend on rebasing
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FVector Location{0.0f};

  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  // TODO: support [multiple] sublevels per category
  ULevelStreamingDynamic* StreamingLevel = nullptr;

  // Assets in tile waiting to be spawned
  // TODO: categorize assets type and prioritize roads
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  TArray<FAssetData> PendingAssetsInTile;

  // Current actors alive in this Tile
  // TODO: Actors in this tile should be owned by StreamingLevel
  TArray<TWeakObjectPtr<AActor>> Actors;

  // Actors that need to be spawned once the level is loaded
  TArray<FGhostActor> Ghosts;

  bool AssetsSpawned = false;
};

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