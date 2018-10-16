// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/WeatherParameters.h"

namespace carla {
namespace client {

  class Actor;
  class ActorBlueprint;
  class BlueprintLibrary;

  class World {
  public:

    explicit World(detail::EpisodeProxy episode) : _episode(std::move(episode)) {}

    World(const World &) = default;
    World(World &&) = default;

    World &operator=(const World &) = default;
    World &operator=(World &&) = default;

    uint32_t GetId() const;

    const std::string &GetMapName() const;

    SharedPtr<BlueprintLibrary> GetBlueprintLibrary() const;

    SharedPtr<Actor> GetSpectator() const;

    rpc::WeatherParameters GetWeather() const;

    void SetWeather(const rpc::WeatherParameters &weather);

    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr);

    SharedPtr<Actor> TrySpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr);

  private:

    detail::EpisodeProxy _episode;
  };

} // namespace client
} // namespace carla
