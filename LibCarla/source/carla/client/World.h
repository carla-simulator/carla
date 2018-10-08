// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/client/detail/Episode.h"
#include "carla/geom/Transform.h"

namespace carla {
namespace client {

  class Actor;
  class ActorBlueprint;
  class BlueprintLibrary;

  class World {
  public:

    World(detail::Episode episode) : _episode(std::move(episode)) {}

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

    detail::Episode _episode;
  };

} // namespace client
} // namespace carla
