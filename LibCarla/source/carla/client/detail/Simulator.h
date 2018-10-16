// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/Version.h"
#include "carla/client/Actor.h"
#include "carla/client/GarbageCollectionPolicy.h"
#include "carla/client/Vehicle.h"
#include "carla/client/detail/Client.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/client/detail/EpisodeState.h"
#include "carla/profiler/LifetimeProfiled.h"

namespace carla {
namespace client {

  class ActorBlueprint;
  class BlueprintLibrary;
  class Episode;
  class Sensor;

namespace detail {

  /// Connects and controls a CARLA Simulator.
  ///
  /// @todo Make sure this class is really thread-safe.
  class Simulator
    : public EnableSharedFromThis<Simulator>,
      private profiler::LifetimeProfiled,
      private NonCopyable {
  public:

    // =========================================================================
    /// @name Constructor
    // =========================================================================
    /// @{

    explicit Simulator(
        const std::string &host,
        uint16_t port,
        size_t worker_threads = 0u,
        bool enable_garbage_collection = false);

    /// @}
    // =========================================================================
    /// @name Access to current episode
    // =========================================================================
    /// @{

    auto GetCurrentEpisodeId() const {
      return _episode.GetId();
    }

    EpisodeProxy GetCurrentEpisode() {
      return EpisodeProxy{shared_from_this()};
    }

    /// @}
    // =========================================================================
    /// @name Garbage collection policy
    // =========================================================================
    /// @{

    GarbageCollectionPolicy GetGarbageCollectionPolicy() const {
      return _gc_policy;
    }

    /// @}
    // =========================================================================
    /// @name Pure networking operations
    // =========================================================================
    /// @{

    void SetNetworkingTimeout(time_duration timeout) {
      _client.SetTimeout(timeout);
    }

    std::string GetClientVersion() {
      return ::carla::version();
    }

    std::string GetServerVersion() {
      return _client.GetServerVersion();
    }

    /// @}
    // =========================================================================
    /// @name Access to global objects in the episode
    // =========================================================================
    /// @{

    SharedPtr<BlueprintLibrary> GetBlueprintLibrary();

    SharedPtr<Actor> GetSpectator();

    rpc::WeatherParameters GetWeatherParameters() {
      return _client.GetWeatherParameters();
    }

    void SetWeatherParameters(const rpc::WeatherParameters &weather) {
      _client.SetWeatherParameters(weather);
    }

    /// @}
    // =========================================================================
    /// @name General operations with actors
    // =========================================================================
    /// @{

    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr,
        GarbageCollectionPolicy gc = GarbageCollectionPolicy::Inherit);

    bool DestroyActor(Actor &actor);

    geom::Location GetActorLocation(const Actor &actor);

    geom::Transform GetActorTransform(const Actor &actor);

    void SetActorLocation(Actor &actor, const geom::Location &location) {
      _client.SetActorLocation(actor.Serialize(), location);
    }

    void SetActorTransform(Actor &actor, const geom::Transform &transform) {
      _client.SetActorTransform(actor.Serialize(), transform);
    }

    /// @}
    // =========================================================================
    /// @name Operations with vehicles
    // =========================================================================
    /// @{

    void SetVehicleAutopilot(Vehicle &vehicle, bool enabled = true) {
      _client.SetActorAutopilot(vehicle.Serialize(), enabled);
    }

    void ApplyControlToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control) {
      _client.ApplyControlToActor(vehicle.Serialize(), control);
    }

    /// @}
    // =========================================================================
    /// @name Operations with sensors
    // =========================================================================
    /// @{

    void SubscribeToSensor(
        const Sensor &sensor,
        std::function<void(SharedPtr<sensor::SensorData>)> callback);

    void UnSubscribeFromSensor(const Sensor &sensor);

    /// @}

  private:

    Client _client;

    EpisodeState _episode;

    GarbageCollectionPolicy _gc_policy;
  };

} // namespace detail
} // namespace client
} // namespace carla
