// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Engine/TargetPoint.h"
#include "VehicleSpawnPoint.generated.h"

/// Base class for spawner locations for walkers.
UCLASS()
class CARLA_API AVehicleSpawnPoint : public ATargetPoint
{
  GENERATED_BODY()
};
