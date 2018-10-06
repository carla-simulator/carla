// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/client/GarbageCollectionPolicy.h"
#include "carla/geom/Transform.h"

namespace carla {
namespace client {

namespace detail {
  class Client;
  class ActorFactory;
}

  class Actor;
  class ActorBlueprint;
  class BlueprintLibrary;

  class World {
  public:

    World(const World &) = default;
    World(World &&) = default;

    World &operator=(const World &) = default;
    World &operator=(World &&) = default;

    SharedPtr<BlueprintLibrary> GetBlueprintLibrary() const;

    SharedPtr<Actor> GetSpectator() const;

    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr);

    SharedPtr<Actor> TrySpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr);

  private:

    friend class Actor;
    friend class detail::ActorFactory;
    friend class detail::Client;

    World(SharedPtr<detail::Client> parent) : parent(std::move(parent)) {}

    SharedPtr<detail::Client> parent;
  };

} // namespace client
} // namespace carla
