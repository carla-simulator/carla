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
#include "carla/client/WorldSnapshot.h"
#include "carla/client/detail/ActorFactory.h"
#include "carla/client/detail/Client.h"
#include "carla/client/detail/Episode.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/client/detail/WalkerNavigation.h"
#include "carla/profiler/LifetimeProfiled.h"
#include "carla/rpc/TrafficLightState.h"
#include "carla/rpc/VehicleLightStateList.h"

#include <boost/optional.hpp>

#include <memory>

namespace carla {
namespace client {

  class ActorBlueprint;
  class BlueprintLibrary;
  class Map;
  class Sensor;
  class WalkerAIController;

namespace detail {

  /// Connects and controls a CARLA Simulator.
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

    EpisodeProxy LoadOpenDriveEpisode(
        std::string opendrive,
        const rpc::OpendriveGenerationParameters & params);

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
    /// @name World snapshot
    // =========================================================================
    /// @{

    WorldSnapshot GetWorldSnapshot() const {
      DEBUG_ASSERT(_episode != nullptr);
      return WorldSnapshot{_episode->GetState()};
    }

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

    time_duration GetNetworkingTimeout() {
      return _client.GetTimeout();
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

    WorldSnapshot WaitForTick(time_duration timeout);

    size_t RegisterOnTickEvent(std::function<void(WorldSnapshot)> callback) {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->RegisterOnTickEvent(std::move(callback));
    }

    void RemoveOnTickEvent(size_t id) {
      DEBUG_ASSERT(_episode != nullptr);
      _episode->RemoveOnTickEvent(id);
    }

    uint64_t Tick(time_duration timeout);

    /// @}
    // =========================================================================
    /// @name Access to global objects in the episode
    // =========================================================================
    /// @{

    std :: string GetEndpoint() {
    	return _client.GetEndpoint();
    }

    /// Query to know if a Traffic Manager is running on port
    bool IsTrafficManagerRunning(uint16_t port) const {
      return _client.IsTrafficManagerRunning(port);
    }

    /// Gets a pair filled with the <IP, port> of the Trafic Manager running on port.
    /// If there is no Traffic Manager running the pair will be ("", 0)
    std::pair<std::string, uint16_t> GetTrafficManagerRunning(uint16_t port) const {
      return _client.GetTrafficManagerRunning(port);
    }

    /// Informs that a Traffic Manager is running on <IP, port>
    bool AddTrafficManagerRunning(std::pair<std::string, uint16_t> trafficManagerInfo) const {
      return _client.AddTrafficManagerRunning(trafficManagerInfo);
    }

    void DestroyTrafficManager(uint16_t port) const {
      _client.DestroyTrafficManager(port);
    }

    void AddPendingException(std::string e) {
      _episode->AddPendingException(e);
    }

    SharedPtr<BlueprintLibrary> GetBlueprintLibrary();

    /// Returns a list of pairs where the firts element is the vehicle ID
    /// and the second one is the light state
    rpc::VehicleLightStateList GetVehiclesLightStates();

    SharedPtr<Actor> GetSpectator();

    rpc::EpisodeSettings GetEpisodeSettings() {
      return _client.GetEpisodeSettings();
    }

    uint64_t SetEpisodeSettings(const rpc::EpisodeSettings &settings);

    rpc::WeatherParameters GetWeatherParameters() {
      return _client.GetWeatherParameters();
    }

    void SetWeatherParameters(const rpc::WeatherParameters &weather) {
      _client.SetWeatherParameters(weather);
    }

    rpc::VehiclePhysicsControl GetVehiclePhysicsControl(const Vehicle &vehicle) const {
      return _client.GetVehiclePhysicsControl(vehicle.GetId());
    }

    rpc::VehicleLightState GetVehicleLightState(const Vehicle &vehicle) const {
      return _client.GetVehicleLightState(vehicle.GetId());
    }

    /// Returns all the BBs of all the elements of the level
    std::vector<geom::BoundingBox> GetLevelBBs(uint8_t queried_tag) const {
      return _client.GetLevelBBs(queried_tag);
    }

    /// @}
    // =========================================================================
    /// @name AI
    // =========================================================================
    /// @{

    void RegisterAIController(const WalkerAIController &controller);

    void UnregisterAIController(const WalkerAIController &controller);

    boost::optional<geom::Location> GetRandomLocationFromNavigation();

    std::shared_ptr<WalkerNavigation> GetNavigation() {
      return _episode->GetNavigation();
    }

    void SetPedestriansCrossFactor(float percentage);

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

    ActorSnapshot GetActorSnapshot(ActorId actor_id) const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetState()->GetActorSnapshot(actor_id);
    }

    ActorSnapshot GetActorSnapshot(const Actor &actor) const {
      return GetActorSnapshot(actor.GetId());
    }

    geom::Location GetActorLocation(const Actor &actor) const {
      return GetActorSnapshot(actor).transform.location;
    }

    geom::Transform GetActorTransform(const Actor &actor) const {
      return GetActorSnapshot(actor).transform;
    }

    geom::Vector3D GetActorVelocity(const Actor &actor) const {
      return GetActorSnapshot(actor).velocity;
    }

    void SetActorTargetVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.SetActorTargetVelocity(actor.GetId(), vector);
    }

    geom::Vector3D GetActorAngularVelocity(const Actor &actor) const {
      return GetActorSnapshot(actor).angular_velocity;
    }

    void SetActorTargetAngularVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.SetActorTargetAngularVelocity(actor.GetId(), vector);
    }
    void EnableActorConstantVelocity(const Actor &actor, const geom::Vector3D &vector) {
      _client.EnableActorConstantVelocity(actor.GetId(), vector);
    }

    void DisableActorConstantVelocity(const Actor &actor) {
      _client.DisableActorConstantVelocity(actor.GetId());
    }

    void AddActorImpulse(const Actor &actor, const geom::Vector3D &impulse) {
      _client.AddActorImpulse(actor.GetId(), impulse);
    }

    void AddActorImpulse(const Actor &actor, const geom::Vector3D &impulse, const geom::Vector3D &location) {
      _client.AddActorImpulse(actor.GetId(), impulse, location);
    }

    void AddActorForce(const Actor &actor, const geom::Vector3D &force) {
      _client.AddActorForce(actor.GetId(), force);
    }

    void AddActorForce(const Actor &actor, const geom::Vector3D &force, const geom::Vector3D &location) {
      _client.AddActorForce(actor.GetId(), force, location);
    }

    void AddActorAngularImpulse(const Actor &actor, const geom::Vector3D &vector) {
      _client.AddActorAngularImpulse(actor.GetId(), vector);
    }

    void AddActorTorque(const Actor &actor, const geom::Vector3D &torque) {
      _client.AddActorAngularImpulse(actor.GetId(), torque);
    }

    geom::Vector3D GetActorAcceleration(const Actor &actor) const {
      return GetActorSnapshot(actor).acceleration;
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

    void SetActorEnableGravity(Actor &actor, bool enabled) {
      _client.SetActorEnableGravity(actor.GetId(), enabled);
    }

    /// @}
    // =========================================================================
    /// @name Operations with vehicles
    // =========================================================================
    /// @{

    void SetVehicleAutopilot(Vehicle &vehicle, bool enabled = true) {
      _client.SetActorAutopilot(vehicle.GetId(), enabled);
    }

    void SetLightsToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control) {
      _client.ApplyControlToVehicle(vehicle.GetId(), control);
    }

    void ApplyControlToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control) {
      _client.ApplyControlToVehicle(vehicle.GetId(), control);
    }

    void ApplyControlToWalker(Walker &walker, const rpc::WalkerControl &control) {
      _client.ApplyControlToWalker(walker.GetId(), control);
    }

    void ApplyBoneControlToWalker(Walker &walker, const rpc::WalkerBoneControl &control) {
      _client.ApplyBoneControlToWalker(walker.GetId(), control);
    }

    void ApplyPhysicsControlToVehicle(Vehicle &vehicle, const rpc::VehiclePhysicsControl &physicsControl) {
      _client.ApplyPhysicsControlToVehicle(vehicle.GetId(), physicsControl);
    }

    void SetLightStateToVehicle(Vehicle &vehicle, const rpc::VehicleLightState light_state) {
      _client.SetLightStateToVehicle(vehicle.GetId(), light_state);
    }

    /// @}
    // =========================================================================
    /// @name Operations with the recorder
    // =========================================================================
    /// @{

    std::string StartRecorder(std::string name, bool additional_data) {
      return _client.StartRecorder(std::move(name), additional_data);
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

    void SetReplayerIgnoreHero(bool ignore_hero) {
      _client.SetReplayerIgnoreHero(ignore_hero);
    }

    void StopReplayer(bool keep_actors) {
      _client.StopReplayer(keep_actors);
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

    void ResetTrafficLightGroup(TrafficLight &trafficLight) {
      _client.ResetTrafficLightGroup(trafficLight.GetId());
    }

    void ResetAllTrafficLights() {
      _client.ResetAllTrafficLights();
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
    // =========================================================================
    /// @name Operations lights
    // =========================================================================
    /// @{

    SharedPtr<LightManager> GetLightManager() const {
      return _light_manager;
    }

    std::vector<rpc::LightState> QueryLightsStateToServer() const {
      return _client.QueryLightsStateToServer();
    }

    void UpdateServerLightsState(
        std::vector<rpc::LightState>& lights,
        bool discard_client = false) const {
      _client.UpdateServerLightsState(lights, discard_client);
    }

    size_t RegisterLightUpdateChangeEvent(std::function<void(WorldSnapshot)> callback) {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->RegisterLightUpdateChangeEvent(std::move(callback));
    }

    void RemoveLightUpdateChangeEvent(size_t id) {
      DEBUG_ASSERT(_episode != nullptr);
      _episode->RemoveLightUpdateChangeEvent(id);
    }

    void FreezeAllTrafficLights(bool frozen);

    /// @}

  private:

    Client _client;

    SharedPtr<LightManager> _light_manager;

    std::shared_ptr<Episode> _episode;

    const GarbageCollectionPolicy _gc_policy;
  };

} // namespace detail
} // namespace client
} // namespace carla
