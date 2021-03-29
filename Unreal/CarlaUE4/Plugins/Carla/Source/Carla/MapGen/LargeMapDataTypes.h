// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelStreamingDynamic.h"

#include "Math/DVector.h"

const float TileSide = 2.0f * 1000.0f * 100.0f; // 2km

struct FActorToConsider
{
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


struct FCarlaMapTile
{

#if WITH_EDITOR
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FString Name; // Tile_{TileID_X}_{TileID_Y}
#endif // WITH_EDITOR

  /// Absolute location, does not depend on rebasing
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FVector Location{0.0f};

  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  ULevelStreamingDynamic* StreamingLevel = nullptr;

  /// Assets in tile waiting to be spawned
  // TODO: categorize assets type and prioritize roads
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  TArray<FAssetData> PendingAssetsInTile;

  bool TilesSpawned = false;
};