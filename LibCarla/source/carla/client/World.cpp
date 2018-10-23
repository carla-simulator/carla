// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/World.h"

#include "carla/Logging.h"
#include "carla/client/Actor.h"
#include "carla/client/ActorBlueprint.h"
#include "carla/client/detail/Client.h"

#include <exception>

namespace carla {
namespace client {

  SharedPtr<BlueprintLibrary> World::GetBlueprintLibrary() const {
    return _episode->GetBlueprintLibrary();
  }

  SharedPtr<Actor> World::GetSpectator() const {
    return _episode->GetSpectator();
  }

  SharedPtr<Actor> World::SpawnActor(
      const ActorBlueprint &blueprint,
      const geom::Transform &transform,
      Actor *parent_actor) {
    try {
      return _episode->SpawnActor(blueprint, transform, parent_actor);
    } catch (const std::exception &e) {
      log_warning("SpawnActor: failed with:", e.what());
      throw;
    }
  }

  SharedPtr<Actor> World::TrySpawnActor(
      const ActorBlueprint &blueprint,
      const geom::Transform &transform,
      Actor *parent_actor) {
    try {
      return SpawnActor(blueprint, transform, parent_actor);
    } catch (const std::exception &) {
      return nullptr;
    }
  }

} // namespace client
} // namespace carla
