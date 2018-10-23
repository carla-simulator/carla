// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/Time.h"
#include "carla/client/Timestamp.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/WeatherParameters.h"

namespace carla {
namespace client {

  class Actor;
  class ActorBlueprint;
  class ActorList;
  class BlueprintLibrary;

  class World {
  public:

    explicit World(detail::EpisodeProxy episode) : _episode(std::move(episode)) {}

    World(const World &) = default;
    World(World &&) = default;

    World &operator=(const World &) = default;
    World &operator=(World &&) = default;

    /// Get the id of the episode associated with this world.
    uint32_t GetId() const;

    /// Return the map name of this world. E.g., Town01.
    ///
    /// @note When playing in editor, the map name can be different than the
    /// usual map name of the town.
    const std::string &GetMapName() const;

    /// Return the list of blueprints available in this world. This blueprints
    /// can be used to spawning actor into the world.
    SharedPtr<BlueprintLibrary> GetBlueprintLibrary() const;

    /// Return the spectator actor. The spectator controls the view in the
    /// simulator window.
    SharedPtr<Actor> GetSpectator() const;

    /// Retrieve the weather parameters currently active in the world.
    rpc::WeatherParameters GetWeather() const;

    /// Change the weather in the simulation.
    void SetWeather(const rpc::WeatherParameters &weather);

    /// Return a list with all the actors currently present in the world.
    SharedPtr<ActorList> GetActors() const;

    /// Spawn an actor into the world based on the @a blueprint provided at @a
    /// transform. If a @a parent is provided, the actor is attached to
    /// @a parent.
    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr);

    /// Same as SpawnActor but return nullptr on failure instead of throwing an
    /// exception.
    SharedPtr<Actor> TrySpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr) noexcept;

    /// Block calling thread until a world tick is received.
    Timestamp WaitForTick(time_duration timeout) const;

    /// Register a @a callback to be called every time a world tick is received.
    void OnTick(std::function<void(Timestamp)> callback);

  private:

    detail::EpisodeProxy _episode;
  };

} // namespace client
} // namespace carla
