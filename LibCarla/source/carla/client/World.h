// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/Time.h"
#include "carla/client/DebugHelper.h"
#include "carla/client/Timestamp.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/Actor.h"
#include "carla/rpc/AttachmentType.h"
#include "carla/rpc/EpisodeSettings.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/rpc/WeatherParameters.h"

namespace carla {
namespace client {

  class Actor;
  class ActorBlueprint;
  class ActorList;
  class BlueprintLibrary;
  class Map;

  class World {
  public:

    explicit World(detail::EpisodeProxy episode) : _episode(std::move(episode)) {}

    World(const World &) = default;
    World(World &&) = default;

    World &operator=(const World &) = default;
    World &operator=(World &&) = default;

    /// Get the id of the episode associated with this world.
    uint64_t GetId() const {
      return _episode.GetId();
    }

    /// Return the map that describes this world.
    SharedPtr<Map> GetMap() const;

    /// Return the list of blueprints available in this world. This blueprints
    /// can be used to spawning actor into the world.
    SharedPtr<BlueprintLibrary> GetBlueprintLibrary() const;

    /// Return the spectator actor. The spectator controls the view in the
    /// simulator window.
    SharedPtr<Actor> GetSpectator() const;

    rpc::EpisodeSettings GetSettings() const;

    void ApplySettings(const rpc::EpisodeSettings &settings);

    /// Retrieve the weather parameters currently active in the world.
    rpc::WeatherParameters GetWeather() const;

    /// Change the weather in the simulation.
    void SetWeather(const rpc::WeatherParameters &weather);

    /// Return a list with all the actors currently present in the world.
    SharedPtr<ActorList> GetActors() const;

    /// Return a list with the actors requested by ActorId.
    SharedPtr<ActorList> GetActors(const std::vector<ActorId> &actor_ids) const;

    /// Spawn an actor into the world based on the @a blueprint provided at @a
    /// transform. If a @a parent is provided, the actor is attached to
    /// @a parent.
    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr,
        rpc::AttachmentType attachment_type = rpc::AttachmentType::Rigid);

    /// Same as SpawnActor but return nullptr on failure instead of throwing an
    /// exception.
    SharedPtr<Actor> TrySpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr,
        rpc::AttachmentType attachment_type = rpc::AttachmentType::Rigid) noexcept;

    /// Block calling thread until a world tick is received.
    Timestamp WaitForTick(time_duration timeout) const;

    /// Register a @a callback to be called every time a world tick is received.
    void OnTick(std::function<void(Timestamp)> callback);

    /// Signal the simulator to continue to next tick (only has effect on
    /// synchronous mode).
    void Tick();

    DebugHelper MakeDebugHelper() const {
      return DebugHelper{_episode};
    }

  private:

    detail::EpisodeProxy _episode;
  };

} // namespace client
} // namespace carla
