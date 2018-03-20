// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Engine/TargetPoint.h"
#include "WalkerSpawnPoint.generated.h"

/// Base class for spawner locations for walkers.
UCLASS(Abstract)
class CARLA_API AWalkerSpawnPointBase : public ATargetPoint
{
  GENERATED_BODY()
};

/// Used to set spawner locations for walkers in the level. These positions will
/// be used solely to spawn walkers at begin play.
UCLASS()
class CARLA_API AWalkerStartSpawnPoint : public AWalkerSpawnPointBase
{
  GENERATED_BODY()
};

/// Used to set spawner locations for walkers in the level. These positions will
/// be used as spawn points as well as destination points for walkers.
UCLASS()
class CARLA_API AWalkerSpawnPoint : public AWalkerSpawnPointBase
{
  GENERATED_BODY()
};
