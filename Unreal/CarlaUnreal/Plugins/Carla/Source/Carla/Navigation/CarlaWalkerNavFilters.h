// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Navigation/CarlaNavAreas.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "NavFilters/NavigationQueryFilter.h"
#include <util/ue-header-guard-end.h>

#include "CarlaWalkerNavFilters.generated.h"

/// Query filter for walkers that must not cross roads (the default): road and
/// crosswalk polys are excluded, so paths stay on sidewalks/terrain and random
/// nav locations never land on the roadway.
UCLASS()
class CARLA_API UCarlaWalkerNavFilter : public UNavigationQueryFilter
{
  GENERATED_BODY()

public:

  UCarlaWalkerNavFilter()
  {
    FNavigationFilterArea Road;
    Road.AreaClass = UNavAreaCarlaRoad::StaticClass();
    Road.bIsExcluded = true;
    Areas.Add(Road);

    FNavigationFilterArea Crosswalk;
    Crosswalk.AreaClass = UNavAreaCarlaCrosswalk::StaticClass();
    Crosswalk.bIsExcluded = true;
    Areas.Add(Crosswalk);
  }
};

/// Query filter for "crosser" walkers (drawn with probability
/// pedestrians_cross_factor): road and crosswalk are traversable and keep the
/// default costs of their area classes (crosswalk ~1.5x, road ~10x), so
/// crossings happen at crosswalks unless the detour is unreasonable.
UCLASS()
class CARLA_API UCarlaWalkerCrosserNavFilter : public UNavigationQueryFilter
{
  GENERATED_BODY()
};
