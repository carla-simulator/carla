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

  /// Random point on the loaded navmesh, in world-local centimeters.
  bool GetRandomNavLocation(FVector &OutLocation) const;
};
