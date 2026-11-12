// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Navigation/CarlaNavigationSubsystem.h"
#include "Carla.h"
#include "Carla/Navigation/CarlaWalkerNavFilters.h"

#include <util/ue-header-guard-begin.h>
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionRuntimeCell.h"
#include "WorldPartition/WorldPartitionStreamingSource.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include <util/ue-header-guard-end.h>

namespace
{
  /// Navmesh samples tried per GetRandomNavLocation call before giving up.
  constexpr int32 MaxSampleAttempts = 32;

  /// Radius around a sampled point whose World Partition cells must be
  /// activated before the point is accepted (cm).
  constexpr float StreamingQueryRadius = 500.0f;

  /// Clearance kept between the capsule's bottom and the sampled point, on
  /// top of the agent step height, so the overlap test does not touch the
  /// ground the walker stands on: covers the navmesh's height error and the
  /// capsule's rim on slopes up to the agent's max slope (cm).
  constexpr float GroundClearance = 10.0f;

  /// Fallbacks when the nav data config has no agent dimensions; they match
  /// the CarlaWalker agent in DefaultEngine.ini (cm).
  constexpr float FallbackWalkerRadius = 35.0f;
  constexpr float FallbackWalkerHeight = 180.0f;
  constexpr float FallbackWalkerStepHeight = 25.0f;
}

bool UCarlaNavigationSubsystem::HasServerSideNavigation() const
{
  const UNavigationSystemV1 *NavSys =
      FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
  if (NavSys == nullptr)
  {
    return false;
  }
  const ARecastNavMesh *NavMesh =
      Cast<ARecastNavMesh>(NavSys->GetDefaultNavDataInstance());
  return (NavMesh != nullptr) && NavMesh->HasValidNavmesh();
}

bool UCarlaNavigationSubsystem::GetRandomNavLocation(FVector &OutLocation) const
{
  UNavigationSystemV1 *NavSys =
      FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
  if (NavSys == nullptr)
  {
    return false;
  }
  ANavigationData *NavData = NavSys->GetDefaultNavDataInstance();
  if (NavData == nullptr)
  {
    return false;
  }
  // Sample with the non-crossing walker filter so random locations land on
  // sidewalk/terrain polys, never on the roadway -- matching the legacy
  // get_random_location_from_navigation semantics. On navmeshes built before
  // the road/crosswalk areas existed the filter simply has no effect.
  FSharedConstNavQueryFilter QueryFilter = UNavigationQueryFilter::GetQueryFilter(
      *NavData, TSubclassOf<UNavigationQueryFilter>(UCarlaWalkerNavFilter::StaticClass()));

  // The streamed navmesh also covers the terrain under some building
  // footprints, and on World Partition maps the building's cell can stream in
  // later than the nav chunk. A walker spawned there ends up inside the
  // building's collision and is pushed down through the terrain (#10212), so
  // every sample is checked and resampled until a walker fits.
  int32 NoPoly = 0;
  int32 NotStreamed = 0;
  int32 Overlapping = 0;
  for (int32 Attempt = 0; Attempt < MaxSampleAttempts; ++Attempt)
  {
    FNavLocation Result;
    if (!NavSys->GetRandomPoint(Result, NavData, QueryFilter))
    {
      return false;
    }
    // UNavigationSystemV1::GetRandomPoint reports success even when Detour
    // found no poly (e.g. it drew a tile whose polys the walker filter all
    // excludes) and leaves the location at the origin; a walker spawned there
    // falls out of the map.
    if (!Result.HasNodeRef())
    {
      ++NoPoly;
      continue;
    }
    switch (CheckWalkerLocation(Result.Location, *NavData))
    {
      case ERejectReason::None:
        if (NoPoly + NotStreamed + Overlapping > 0)
        {
          UE_LOG(LogCarla, Verbose,
              TEXT("Random nav location: accepted after rejecting %d without poly, %d not streamed, %d overlapping"),
              NoPoly, NotStreamed, Overlapping);
        }
        OutLocation = Result.Location;
        return true;
      case ERejectReason::NotStreamed:
        ++NotStreamed;
        break;
      case ERejectReason::Overlap:
        ++Overlapping;
        UE_LOG(LogCarla, Verbose,
            TEXT("Random nav location: rejected %s, a walker would overlap static geometry"),
            *Result.Location.ToString());
        break;
    }
  }
  UE_LOG(LogCarla, Log,
      TEXT("Random nav location: no acceptable point in %d samples (%d without poly, %d not streamed, %d overlapping)"),
      MaxSampleAttempts, NoPoly, NotStreamed, Overlapping);
  return false;
}

UCarlaNavigationSubsystem::ERejectReason UCarlaNavigationSubsystem::CheckWalkerLocation(
    const FVector &Location,
    const ANavigationData &NavData) const
{
  UWorld *World = GetWorld();
  check(World != nullptr);

  if (World->GetWorldPartition() != nullptr)
  {
    const UWorldPartitionSubsystem *WPSubsystem =
        World->GetSubsystem<UWorldPartitionSubsystem>();
    if (WPSubsystem != nullptr)
    {
      FWorldPartitionStreamingQuerySource Query(Location);
      Query.Radius = StreamingQueryRadius;
      Query.bUseGridLoadingRange = false;
      if (!WPSubsystem->IsStreamingCompleted(
              EWorldPartitionRuntimeCellState::Activated, {Query},
              /*bExactState=*/false))
      {
        return ERejectReason::NotStreamed;
      }
    }
  }

  const FNavDataConfig &Config = NavData.GetConfig();
  const float Radius =
      Config.AgentRadius > 0.0f ? Config.AgentRadius : FallbackWalkerRadius;
  const float Height =
      Config.AgentHeight > 0.0f ? Config.AgentHeight : FallbackWalkerHeight;
  const float StepHeight = Config.HasStepHeightOverride()
      ? Config.AgentStepHeight : FallbackWalkerStepHeight;
  const float HalfHeight = FMath::Max(Height * 0.5f, Radius);

  // Static geometry only: vehicles and other walkers move away, a building
  // does not.
  FCollisionQueryParams Params(SCENE_QUERY_STAT(CarlaWalkerNavLocation), false);
  const FVector Center =
      Location + FVector(0.0f, 0.0f, StepHeight + GroundClearance + HalfHeight);
  const bool bOverlaps = World->OverlapAnyTestByObjectType(
      Center, FQuat::Identity,
      FCollisionObjectQueryParams(ECC_WorldStatic),
      FCollisionShape::MakeCapsule(Radius, HalfHeight),
      Params);
  return bOverlaps ? ERejectReason::Overlap : ERejectReason::None;
}
