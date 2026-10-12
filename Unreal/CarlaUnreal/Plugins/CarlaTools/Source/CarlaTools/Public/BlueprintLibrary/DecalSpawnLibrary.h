// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Materials/MaterialInterface.h"
#include <util/ue-header-guard-end.h>

#include "DecalSpawnLibrary.generated.h"

UENUM(BlueprintType)
enum class EDecalType : uint8
{
  Generic,
  Manhole,
  ManholeCurb,
  RoadDebris,
  Dirt,
  // Appended, not inserted -- EDecalType is stored as a raw byte in saved
  // DecalLibrary data assets, so existing entries would silently point at
  // the wrong type if a new value were inserted earlier in this list.
  RoadDamage
};

/// One entry of a decal library: a material plus the ranges used to
/// randomize each spawned instance. Weighted by Probability against the
/// other entries in the same library array.
USTRUCT(BlueprintType)
struct FDecalSpawnParams
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
  TObjectPtr<UMaterialInterface> Decal = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal", meta = (ClampMin = "0.0"))
  float Probability = 1.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
  float MinScale = 1.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
  float MaxScale = 1.0f;

  /// Yaw is randomized within [-RotationOffset, +RotationOffset] degrees
  /// around the base rotation (the replaced actor's own rotation, or the
  /// road-flow rotation when bFollowRoadFlow is set).
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal", meta = (ClampMin = "0.0"))
  float RotationOffset = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
  bool bFollowRoadFlow = false;

  /// Fixed yaw added on top of the road-flow rotation, only applied when
  /// bFollowRoadFlow is set and a road was actually found. Corrects
  /// decals whose texture was authored with its long axis across the
  /// wrong side (e.g. tire marks coming out perpendicular to the road
  /// instead of running along it).
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
  float RoadFlowYawOffset = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
  EDecalType DecalType = EDecalType::Generic;
};

UCLASS(BlueprintType)
class CARLATOOLS_API UDecalSpawnLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// Replaces every actor in ActorsToReplace with a decal: picks an entry
  /// from DecalLibrary (weighted by Probability), places it at the
  /// replaced actor's location, applies a random scale/rotation-offset
  /// from that entry's ranges, and tags it with DecalType and
  /// SpawnedDecalTag.
  ///
  /// If the replaced actor is already a DecalActor, it is edited in place
  /// (same actor, same World Partition external-actor package on disk —
  /// just resaved) instead of being destroyed and replaced by a freshly
  /// spawned one; any other class (e.g. BP_Decals) is destroyed and a new
  /// DecalActor is spawned in its place, since it can't be turned into one
  /// in place. At full-map scale this matters: a repeat pass over
  /// thousands of already-placed DecalActor keeps the same package files
  /// (a normal resave), instead of pairing every one of them with a
  /// deleted-and-recreated file.
  ///
  /// ActorsToReplace is read once into a fixed-size snapshot before any
  /// editing starts, so a decal this call touches can never be picked up
  /// again by this same call — safe to pass "get all actors of class
  /// BP_Decals" for a first pass, or "get all actors of class DecalActor"
  /// for a repeat pass, without risking runaway spawning.
  ///
  /// Pitch/Roll are never taken from the replaced actor: they are always
  /// Pitch=-90/Roll=0 (decal projecting straight down onto flat ground).
  /// Yaw is taken from the replaced actor's own rotation, or — if an entry
  /// has bFollowRoadFlow set — from the nearest ARoutePlanner spline
  /// within RoadFlowSearchRadius (see
  /// ARoutePlanner::FindRoadFlowRotationAtLocation), plus that entry's
  /// RoadFlowYawOffset, falling back to the replaced actor's yaw (no
  /// offset) if none is found within range; either way, RotationOffset is
  /// then added on top.
  ///
  /// Returns the number of decals spawned or edited (== ActorsToReplace.Num()
  /// minus any entries skipped because DecalLibrary was empty or all
  /// weights were zero).
  UFUNCTION(BlueprintCallable, Category = "Carla Tools|Decals", meta = (WorldContext = "WorldContextObject"))
  static int32 ReplaceActorsWithRandomDecals(
      UObject *WorldContextObject,
      const TArray<AActor *> &ActorsToReplace,
      const TArray<FDecalSpawnParams> &DecalLibrary,
      FRandomStream RandomStream,
      float RoadFlowSearchRadius = 500.0f,
      FName SpawnedDecalTag = TEXT("SpawnedDecal"));
};
