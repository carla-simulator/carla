// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/Version.h"
#include "carla/client/Actor.h"
#include "carla/client/GarbageCollectionPolicy.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Walker.h"
#include "carla/client/detail/Client.h"
#include "carla/client/detail/Episode.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/profiler/LifetimeProfiled.h"
#include "carla/client/TrafficLight.h"
#include "carla/rpc/TrafficLightState.h"

#include <memory>

namespace carla {
namespace client {

  class ActorBlueprint;
  class BlueprintLibrary;
  class Map;
  class Sensor;

namespace detail {

  /// Connects and controls a CARLA Simulator.
  ///
  /// @todo Make sure this class is really thread-safe.
  class Simulator
    : public std::enable_shared_from_this<Simulator>,
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
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetId();
    }

    const std::string &GetCurrentMapName() {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetMapName();
    }

    EpisodeProxy GetCurrentEpisode();

    SharedPtr<Map> GetCurrentMap();

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
      return _client.GetClientVersion();
    }

    std::string GetServerVersion() {
      return _client.GetServerVersion();
    }

    /// @}
    // =========================================================================
    /// @name Tick
    // =========================================================================
    /// @{

    Timestamp WaitForTick(time_duration timeout);

    void RegisterOnTickEvent(std::function<void(Timestamp)> callback) {
      DEBUG_ASSERT(_episode != nullptr);
      _episode->RegisterOnTickEvent(std::move(callback));
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

    std::vector<rpc::Actor> GetAllTheActorsInTheEpisode() const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetActors();
    }

    /// If @a gc is GarbageCollectionPolicy::Enabled, the shared pointer
    /// returned is provided with a custom deleter that calls Destroy() on the
    /// actor. If @gc is GarbageCollectionPolicy::Enabled, the default garbage
    /// collection policy is used.
    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr,
        GarbageCollectionPolicy gc = GarbageCollectionPolicy::Inherit);

    bool DestroyActor(Actor &actor);

    auto GetActorDynamicState(const Actor &actor) const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetState()->GetActorState(actor.GetId());
    }

    geom::Location GetActorLocation(const Actor &actor) const {
      return GetActorDynamicState(actor).transform.location;
    }

    geom::Transform GetActorTransform(const Actor &actor) const {
      return GetActorDynamicState(actor).transform;
    }

    geom::Vector3D GetActorVelocity(const Actor &actor) const {
      return GetActorDynamicState(actor).velocity;
    }

    void SetActorVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.SetActorVelocity(actor.Serialize(), vector);
    }

    geom::Vector3D GetActorAngularVelocity(const Actor &actor) const {
      return GetActorDynamicState(actor).angular_velocity;
    }

    void SetActorAngularVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.SetActorAngularVelocity(actor.Serialize(), vector);
    }

    void AddActorImpulse(const Actor &actor, const geom::Vector3D &vector) {
      _client.AddActorImpulse(actor.Serialize(), vector);
    }

    geom::Vector3D GetActorAcceleration(const Actor &actor) const {
      return GetActorDynamicState(actor).acceleration;
    }

    void SetActorLocation(Actor &actor, const geom::Location &location) {
      _client.SetActorLocation(actor.Serialize(), location);
    }

    void SetActorTransform(Actor &actor, const geom::Transform &transform) {
      _client.SetActorTransform(actor.Serialize(), transform);
    }

    void SetActorSimulatePhysics(Actor &actor, bool enabled) {
      _client.SetActorSimulatePhysics(actor.Serialize(), enabled);
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
      _client.ApplyControlToVehicle(vehicle.Serialize(), control);
    }

    void ApplyControlToWalker(Walker &walker, const rpc::WalkerControl &control) {
      _client.ApplyControlToWalker(walker.Serialize(), control);
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
    // =========================================================================
    /// @name Operations with traffic lights
    // =========================================================================
    /// @{
    void SetTrafficLightState(TrafficLight &trafficLight, const rpc::TrafficLightState trafficLightState) {
      _client.SetTrafficLightState(trafficLight.Serialize(), trafficLightState);
    }

    void SetTrafficLightGreenTime(TrafficLight &trafficLight, float greenTime) {
      _client.SetTrafficLightGreenTime(trafficLight.Serialize(), greenTime);
    }

    void SetTrafficLightYellowTime(TrafficLight &trafficLight, float yellowTime) {
      _client.SetTrafficLightYellowTime(trafficLight.Serialize(), yellowTime);
    }

    void SetTrafficLightRedTime(TrafficLight &trafficLight, float redTime) {
      _client.SetTrafficLightRedTime(trafficLight.Serialize(), redTime);
    }

    void FreezeTrafficLight(TrafficLight &trafficLight, bool freeze) {
      _client.FreezeTrafficLight(trafficLight.Serialize(), freeze);
    }

    /// @}
    // =========================================================================
    /// @name Debug
    // =========================================================================
    /// @{

    void DrawDebugShape(const rpc::DebugShape &shape) {
      _client.DrawDebugShape(shape);
    }

    /// @}

  private:

    Client _client;

    std::shared_ptr<Episode> _episode;

    GarbageCollectionPolicy _gc_policy;
  };

} // namespace detail
} // namespace client
} // namespace carla
