// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/Time.h"
#include "carla/client/DebugHelper.h"
#include "carla/client/Landmark.h"
#include "carla/client/LightManager.h"
#include "carla/client/Timestamp.h"
#include "carla/client/WorldSnapshot.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/Actor.h"
#include "carla/rpc/AttachmentType.h"
#include "carla/rpc/EpisodeSettings.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/rpc/WeatherParameters.h"
#include "carla/rpc/VehicleLightStateList.h"

#include <boost/optional.hpp>

namespace carla {
namespace client {

  class Actor;
  class ActorBlueprint;
  class ActorList;
  class BlueprintLibrary;
  class Map;
  class TrafficLight;
  class TrafficSign;

  class World {
  public:

    explicit World(detail::EpisodeProxy episode) : _episode(std::move(episode)) {}

    ~World(){}

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

    /// Returns a list of pairs where the firts element is the vehicle ID
    /// and the second one is the light state
    rpc::VehicleLightStateList GetVehiclesLightStates() const;

    /// Get a random location from the pedestrians navigation mesh
    boost::optional<geom::Location> GetRandomLocationFromNavigation() const;

    /// Return the spectator actor. The spectator controls the view in the
    /// simulator window.
    SharedPtr<Actor> GetSpectator() const;

    rpc::EpisodeSettings GetSettings() const;

    /// @return The id of the frame when the settings were applied.
    uint64_t ApplySettings(const rpc::EpisodeSettings &settings);

    /// Retrieve the weather parameters currently active in the world.
    rpc::WeatherParameters GetWeather() const;

    /// Change the weather in the simulation.
    void SetWeather(const rpc::WeatherParameters &weather);

    /// Return a snapshot of the world at this moment.
    WorldSnapshot GetSnapshot() const;

    /// Find actor by id, return nullptr if not found.
    SharedPtr<Actor> GetActor(ActorId id) const;

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
    WorldSnapshot WaitForTick(time_duration timeout) const;

    /// Register a @a callback to be called every time a world tick is received.
    ///
    /// @return ID of the callback, use it to remove the callback.
    size_t OnTick(std::function<void(WorldSnapshot)> callback);

    /// Remove a callback registered with OnTick.
    void RemoveOnTick(size_t callback_id);

    /// Signal the simulator to continue to next tick (only has effect on
    /// synchronous mode).
    ///
    /// @return The id of the frame that this call started.
    uint64_t Tick(time_duration timeout);

    /// set the probability that an agent could cross the roads in its path following
    /// percentage of 0.0f means no pedestrian can cross roads
    /// percentage of 0.5f means 50% of all pedestrians can cross roads
    /// percentage of 1.0f means all pedestrians can cross roads if needed
    void SetPedestriansCrossFactor(float percentage);

    SharedPtr<Actor> GetTrafficSign(const Landmark& landmark) const;

    SharedPtr<Actor> GetTrafficLight(const Landmark& landmark) const;

    SharedPtr<LightManager> GetLightManager() const;

    DebugHelper MakeDebugHelper() const {
      return DebugHelper{_episode};
    }

    detail::EpisodeProxy GetEpisode() const {
      return _episode;
    };

    void FreezeAllTrafficLights(bool frozen);

  private:

    detail::EpisodeProxy _episode;
  };

} // namespace client
} // namespace carla
