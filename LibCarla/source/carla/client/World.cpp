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
      const Transform &transform,
      Actor *parent) {
    try {
      return SpawnActor(blueprint, transform, parent);
    } catch (const std::exception &e) {
      log_warning("TrySpawnActor: failed with:", e.what());
      return nullptr;
    }
  }

} // namespace client
} // namespace carla
