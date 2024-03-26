// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/ActorSpawnResult.h"

FString FActorSpawnResult::StatusToString(EActorSpawnResultStatus InStatus)
{
  static_assert(
      static_cast<uint8>(EActorSpawnResultStatus::SIZE) == 4u,
      "If you add a new status, please update this function.");

  switch (InStatus)
  {
    case EActorSpawnResultStatus::Success:
      return TEXT("Success");
    case EActorSpawnResultStatus::InvalidDescription:
      return TEXT("Spawn failed because of invalid actor description");
    case EActorSpawnResultStatus::Collision:
      return TEXT("Spawn failed because of collision at spawn position");
    case EActorSpawnResultStatus::UnknownError:
    default:
      return TEXT("Unknown error while trying to spawn actor");
  }
}
