// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/client/Actor.h"
#include "carla/client/GarbageCollectionPolicy.h"
#include "carla/client/TrafficLight.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Walker.h"
#include "carla/client/detail/ActorFactory.h"
#include "carla/client/detail/Client.h"
#include "carla/client/detail/Episode.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/profiler/LifetimeProfiled.h"
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
    /// @name Load a new episode
    // =========================================================================
    /// @{

    EpisodeProxy ReloadEpisode() {
      return LoadEpisode("");
    }

    EpisodeProxy LoadEpisode(std::string map_name);

    /// @}
    // =========================================================================
    /// @name Access to current episode
    // =========================================================================
    /// @{

    /// @pre Cannot be called previous to GetCurrentEpisode.
    auto GetCurrentEpisodeId() const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetId();
    }

    EpisodeProxy GetCurrentEpisode();

    /// @}
    // =========================================================================
    /// @name Map related methods
    // =========================================================================
    /// @{

    SharedPtr<Map> GetCurrentMap();

    std::vector<std::string> GetAvailableMaps() {
      return _client.GetAvailableMaps();
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

    void Tick() {
      _client.SendTickCue();
    }

    /// @}
    // =========================================================================
    /// @name Access to global objects in the episode
    // =========================================================================
    /// @{

    SharedPtr<BlueprintLibrary> GetBlueprintLibrary();

    SharedPtr<Actor> GetSpectator();

    rpc::EpisodeSettings GetEpisodeSettings() {
      return _client.GetEpisodeSettings();
    }

    void SetEpisodeSettings(const rpc::EpisodeSettings &settings) {
      _client.SetEpisodeSettings(settings);
    }

    rpc::WeatherParameters GetWeatherParameters() {
      return _client.GetWeatherParameters();
    }

    void SetWeatherParameters(const rpc::WeatherParameters &weather) {
      _client.SetWeatherParameters(weather);
    }

    rpc::VehiclePhysicsControl GetVehiclePhysicsControl(const Vehicle &vehicle) const {
      return _client.GetVehiclePhysicsControl(vehicle.GetId());
    }

    /// @}
    // =========================================================================
    /// @name General operations with actors
    // =========================================================================
    /// @{

    boost::optional<rpc::Actor> GetActorById(ActorId id) const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetActorById(id);
    }

    std::vector<rpc::Actor> GetActorsById(const std::vector<ActorId> &actor_ids) const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetActorsById(actor_ids);
    }

    std::vector<rpc::Actor> GetAllTheActorsInTheEpisode() const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetActors();
    }

    /// Creates an actor instance out of a description of an existing actor.
    /// Note that this does not spawn an actor.
    ///
    /// If @a gc is GarbageCollectionPolicy::Enabled, the shared pointer
    /// returned is provided with a custom deleter that calls Destroy() on the
    /// actor. This method does not support GarbageCollectionPolicy::Inherit.
    SharedPtr<Actor> MakeActor(
        rpc::Actor actor_description,
        GarbageCollectionPolicy gc = GarbageCollectionPolicy::Disabled) {
      RELEASE_ASSERT(gc != GarbageCollectionPolicy::Inherit);
      return ActorFactory::MakeActor(GetCurrentEpisode(), std::move(actor_description), gc);
    }

    /// Spawns an actor into the simulation.
    ///
    /// If @a gc is GarbageCollectionPolicy::Enabled, the shared pointer
    /// returned is provided with a custom deleter that calls Destroy() on the
    /// actor. If @gc is GarbageCollectionPolicy::Inherit, the default garbage
    /// collection policy is used.
    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent = nullptr,
        rpc::AttachmentType attachment_type = rpc::AttachmentType::Rigid,
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
      _client.SetActorVelocity(actor.GetId(), vector);
    }

    geom::Vector3D GetActorAngularVelocity(const Actor &actor) const {
      return GetActorDynamicState(actor).angular_velocity;
    }

    void SetActorAngularVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.SetActorAngularVelocity(actor.GetId(), vector);
    }

    void AddActorImpulse(const Actor &actor, const geom::Vector3D &vector) {
      _client.AddActorImpulse(actor.GetId(), vector);
    }

    geom::Vector3D GetActorAcceleration(const Actor &actor) const {
      return GetActorDynamicState(actor).acceleration;
    }

    void SetActorLocation(Actor &actor, const geom::Location &location) {
      _client.SetActorLocation(actor.GetId(), location);
    }

    void SetActorTransform(Actor &actor, const geom::Transform &transform) {
      _client.SetActorTransform(actor.GetId(), transform);
    }

    void SetActorSimulatePhysics(Actor &actor, bool enabled) {
      _client.SetActorSimulatePhysics(actor.GetId(), enabled);
    }

    /// @}
    // =========================================================================
    /// @name Operations with vehicles
    // =========================================================================
    /// @{

    void SetVehicleAutopilot(Vehicle &vehicle, bool enabled = true) {
      _client.SetActorAutopilot(vehicle.GetId(), enabled);
    }

    void ApplyControlToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control) {
      _client.ApplyControlToVehicle(vehicle.GetId(), control);
    }

    void ApplyControlToWalker(Walker &walker, const rpc::WalkerControl &control) {
      _client.ApplyControlToWalker(walker.GetId(), control);
    }

    void ApplyPhysicsControlToVehicle(Vehicle &vehicle, const rpc::VehiclePhysicsControl &physicsControl) {
      _client.ApplyPhysicsControlToVehicle(vehicle.GetId(), physicsControl);
    }
    /// @}
    // =========================================================================
    /// @name Operations with the recorder
    // =========================================================================
    /// @{

    std::string StartRecorder(std::string name) {
      return _client.StartRecorder(std::move(name));
    }

    void StopRecorder(void) {
      _client.StopRecorder();
    }

    std::string ShowRecorderFileInfo(std::string name, bool show_all) {
      return _client.ShowRecorderFileInfo(std::move(name), show_all);
    }

    std::string ShowRecorderCollisions(std::string name, char type1, char type2) {
      return _client.ShowRecorderCollisions(std::move(name), type1, type2);
    }

    std::string ShowRecorderActorsBlocked(std::string name, double min_time, double min_distance) {
      return _client.ShowRecorderActorsBlocked(std::move(name), min_time, min_distance);
    }

    std::string ReplayFile(std::string name, double start, double duration, uint32_t follow_id) {
      return _client.ReplayFile(std::move(name), start, duration, follow_id);
    }

    void SetReplayerTimeFactor(double time_factor) {
      _client.SetReplayerTimeFactor(time_factor);
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
      _client.SetTrafficLightState(trafficLight.GetId(), trafficLightState);
    }

    void SetTrafficLightGreenTime(TrafficLight &trafficLight, float greenTime) {
      _client.SetTrafficLightGreenTime(trafficLight.GetId(), greenTime);
    }

    void SetTrafficLightYellowTime(TrafficLight &trafficLight, float yellowTime) {
      _client.SetTrafficLightYellowTime(trafficLight.GetId(), yellowTime);
    }

    void SetTrafficLightRedTime(TrafficLight &trafficLight, float redTime) {
      _client.SetTrafficLightRedTime(trafficLight.GetId(), redTime);
    }

    void FreezeTrafficLight(TrafficLight &trafficLight, bool freeze) {
      _client.FreezeTrafficLight(trafficLight.GetId(), freeze);
    }

    std::vector<ActorId> GetGroupTrafficLights(TrafficLight &trafficLight) {
      return _client.GetGroupTrafficLights(trafficLight.GetId());
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
    // =========================================================================
    /// @name Apply commands in batch
    // =========================================================================
    /// @{

    void ApplyBatch(std::vector<rpc::Command> commands, bool do_tick_cue) {
      _client.ApplyBatch(std::move(commands), do_tick_cue);
    }

    auto ApplyBatchSync(std::vector<rpc::Command> commands, bool do_tick_cue) {
      return _client.ApplyBatchSync(std::move(commands), do_tick_cue);
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
