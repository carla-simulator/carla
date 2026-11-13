// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include <util/ue-header-guard-end.h>

#include "CarlaNavigationSubsystem.generated.h"

/// Thin wrapper over the engine navigation system (UNavigationSystemV1) used
/// by the server-side pedestrian navigation RPCs. On World Partition maps the
/// navmesh streams with the cells, so every query below operates on the
/// currently loaded tiles only.
UCLASS()
class CARLA_API UCarlaNavigationSubsystem : public UWorldSubsystem
{
  GENERATED_BODY()

public:

  static UCarlaNavigationSubsystem *Get(UWorld *World)
  {
    return World != nullptr
        ? World->GetSubsystem<UCarlaNavigationSubsystem>()
        : nullptr;
  }

  /// True iff the current world has a RecastNavMesh with valid nav data:
  /// the signal for clients that walker navigation runs server-side here.
  bool HasServerSideNavigation() const;

  /// Random point on the loaded navmesh where a walker can stand, in
  /// world-local centimeters. Points whose surroundings are still streaming
  /// in, or where a walker capsule would overlap static geometry (the terrain
  /// under a building footprint, issue #10212), are resampled; returns false
  /// when no acceptable point is found.
  bool GetRandomNavLocation(FVector &OutLocation) const;

private:

  /// Why a sampled nav point was rejected, for the Verbose log.
  enum class ERejectReason : uint8
  {
    None,
    NotStreamed,
    Overlap
  };

  /// Checks a sampled nav point: on World Partition maps the cells around it
  /// must be activated (so a building above it is already there), and a
  /// walker-sized capsule standing on it must not overlap static geometry.
  ERejectReason CheckWalkerLocation(
      const FVector &Location,
      const class ANavigationData &NavData) const;
};
