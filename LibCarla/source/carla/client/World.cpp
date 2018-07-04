// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/World.h"

#include "carla/Logging.h"

namespace carla {
namespace client {

  SharedPtr<Actor> World::TrySpawnActor(
      const ActorBlueprint &blueprint,
      const Transform &transform) {
    try {
      return SpawnActor(blueprint, transform);
    } catch (const std::exception & DEBUG_ONLY(e)) {
      DEBUG_ONLY(log_debug("TrySpawnActor: failed with:", e.what()));
      return nullptr;
    }
  }

} // namespace client
} // namespace carla
