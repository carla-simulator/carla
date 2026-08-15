// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Navigation/CarlaNavigationSubsystem.h"
#include "Carla.h"

#include <util/ue-header-guard-begin.h>
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include <util/ue-header-guard-end.h>

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
  FNavLocation Result;
  if (!NavSys->GetRandomPoint(Result))
  {
    return false;
  }
  OutLocation = Result.Location;
  return true;
}
