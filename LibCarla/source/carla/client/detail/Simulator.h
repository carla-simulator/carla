// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Logging.h"
#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/client/Actor.h"
#include "carla/client/GarbageCollectionPolicy.h"
#include "carla/client/TrafficLight.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Walker.h"
#include "carla/client/World.h"
#include "carla/client/WorldSnapshot.h"
#include "carla/client/detail/ActorFactory.h"
#include "carla/client/detail/Client.h"
#include "carla/client/detail/Episode.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/profiler/LifetimeProfiled.h"
#include "carla/rpc/TrafficLightState.h"
#include "carla/rpc/VehicleLightStateList.h"
#include "carla/rpc/LabelledPoint.h"
#include "carla/rpc/VehicleWheels.h"
#include "carla/rpc/Texture.h"
#include "carla/rpc/MaterialParameter.h"

#include <boost/optional.hpp>

#include <memory>

namespace carla {
namespace client {

  class ActorBlueprint;
  class BlueprintLibrary;
  class Map;
  class Sensor;
  class WalkerAIController;
  class WalkerNavigation;

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

    EpisodeProxy ReloadEpisode(bool reset_settings = true) {
      return LoadEpisode("", reset_settings);
    }

    EpisodeProxy LoadEpisode(std::string map_name, bool reset_settings = true, rpc::MapLayer map_layers = rpc::MapLayer::All);

    void LoadLevelLayer(rpc::MapLayer map_layers) const {
      _client.LoadLevelLayer(map_layers);
    }

    void UnloadLevelLayer(rpc::MapLayer map_layers) const {
      _client.UnloadLevelLayer(map_layers);
    }

    EpisodeProxy LoadOpenDriveEpisode(
        std::string opendrive,
        const rpc::OpendriveGenerationParameters & params,
        bool reset_settings = true);

    /// @}
    // =========================================================================
    /// @name Access to current episode
    // =========================================================================
    /// @{

    /// @pre Cannot be called previous to GetCurrentEpisode.
    auto GetCurrentEpisodeId() {
      GetReadyCurrentEpisode();
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetId();
    }

    void GetReadyCurrentEpisode();
    EpisodeProxy GetCurrentEpisode();

    /// @}
    // =========================================================================
    /// @name World snapshot
    // =========================================================================
    /// @{

    World GetWorld() {
      return World{GetCurrentEpisode()};
    }

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
    /// @name Required files related methods
    // =========================================================================
    /// @{

    bool SetFilesBaseFolder(const std::string &path);

    std::vector<std::string> GetRequiredFiles(const std::string &folder = "", const bool download = true) const;

    void RequestFile(const std::string &name) const;

    std::vector<uint8_t> GetCacheFile(const std::string &name, const bool request_otherwise) const;

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

    float GetIMUISensorGravity() const {
      return _client.GetIMUISensorGravity();
    }

    void SetIMUISensorGravity(float NewIMUISensorGravity) {
      _client.SetIMUISensorGravity(NewIMUISensorGravity);
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

    std::vector<rpc::EnvironmentObject> GetEnvironmentObjects(uint8_t queried_tag) const {
      return _client.GetEnvironmentObjects(queried_tag);
    }

    void EnableEnvironmentObjects(
      std::vector<uint64_t> env_objects_ids,
      bool enable) const {
      _client.EnableEnvironmentObjects(env_objects_ids, enable);
    }

    std::pair<bool,rpc::LabelledPoint> ProjectPoint(
        geom::Location location, geom::Vector3D direction, float search_distance) const {
      return _client.ProjectPoint(location, direction, search_distance);
    }

    std::vector<rpc::LabelledPoint> CastRay(
        geom::Location start_location, geom::Location end_location) const {
      return _client.CastRay(start_location, end_location);
    }

    void SetAnnotationsTraverseTranslucency(
      bool enable) {
        _client.SetAnnotationsTraverseTranslucency(enable);
      }

    /// @}
    // =========================================================================
    /// @name AI
    // =========================================================================
    /// @{

    std::shared_ptr<WalkerNavigation> GetNavigation();

    void NavigationTick();

    void RegisterAIController(const WalkerAIController &controller);

    void UnregisterAIController(const WalkerAIController &controller);

    boost::optional<geom::Location> GetRandomLocationFromNavigation();

    void SetPedestriansCrossFactor(float percentage);

    void SetPedestriansSeed(unsigned int seed);

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
        GarbageCollectionPolicy gc = GarbageCollectionPolicy::Inherit,
        const std::string& socket_name = "");

    bool DestroyActor(Actor &actor);

    bool DestroyActor(ActorId actor_id)
    {
      return _client.DestroyActor(actor_id);
    }

    ActorSnapshot GetActorSnapshot(ActorId actor_id) const {
      DEBUG_ASSERT(_episode != nullptr);
      return _episode->GetState()->GetActorSnapshot(actor_id);
    }

    ActorSnapshot GetActorSnapshot(const Actor &actor) const {
      return GetActorSnapshot(actor.GetId());
    }

    rpc::ActorState GetActorState(const Actor &actor) const {
      return GetActorSnapshot(actor).actor_state;
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

    geom::BoundingBox GetActorBoundingBox(const Actor &actor) {
      return _client.GetActorBoundingBox(actor.GetId());
    }

    geom::Transform GetActorComponentWorldTransform(const Actor &actor, const std::string componentName) {
      return _client.GetActorComponentWorldTransform(actor.GetId(), componentName);
    }

    geom::Transform GetActorComponentRelativeTransform(const Actor &actor, std::string componentName) {
      return _client.GetActorComponentRelativeTransform(actor.GetId(), componentName);
    }

    std::vector<geom::Transform> GetActorBoneWorldTransforms(const Actor &actor) {
      return _client.GetActorBoneWorldTransforms(actor.GetId());
    }
    
    std::vector<geom::Transform> GetVehicleBoneWorldTransforms(const Vehicle &vehicle) {
      return _client.GetVehicleBoneWorldTransforms(vehicle.GetId());
    }

    std::vector<geom::Transform> GetActorBoneRelativeTransforms(const Actor &actor) {
      return _client.GetActorBoneRelativeTransforms(actor.GetId());
    }

    std::vector<std::string> GetActorComponentNames(const Actor &actor) {
      return _client.GetActorComponentNames(actor.GetId());
    }

    std::vector<std::string> GetActorBoneNames(const Actor &actor) {
      return _client.GetActorBoneNames(actor.GetId());
    }

    std::vector<geom::Transform> GetActorSocketWorldTransforms(const Actor &actor) {
      return _client.GetActorSocketWorldTransforms(actor.GetId());
    }

    std::vector<geom::Transform> GetActorSocketRelativeTransforms(const Actor &actor) {
      return _client.GetActorSocketRelativeTransforms(actor.GetId());
    }

    std::vector<std::string> GetActorSocketNames(const Actor &actor) {
      return _client.GetActorSocketNames(actor.GetId());
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

    void SetActorCollisions(Actor &actor, bool enabled) {
      _client.SetActorCollisions(actor.GetId(), enabled);
    }

    void SetActorCollisions(ActorId actor_id, bool enabled) {
      _client.SetActorCollisions(actor_id, enabled);
    }

    void SetActorDead(Actor &actor) {
      _client.SetActorDead(actor.GetId());
    }

    void SetActorDead(ActorId actor_id) {
      _client.SetActorDead(actor_id);
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

    rpc::VehicleTelemetryData GetVehicleTelemetryData(const Vehicle &vehicle) const {
      return _client.GetVehicleTelemetryData(vehicle.GetId());
    }

    void ShowVehicleDebugTelemetry(Vehicle &vehicle, bool enabled = true) {
      _client.ShowVehicleDebugTelemetry(vehicle.GetId(), enabled);
    }

    void SetLightsToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control) {
      _client.ApplyControlToVehicle(vehicle.GetId(), control);
    }

    void ApplyControlToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control) {
      _client.ApplyControlToVehicle(vehicle.GetId(), control);
    }

    void ApplyAckermannControlToVehicle(Vehicle &vehicle, const rpc::VehicleAckermannControl &control) {
      _client.ApplyAckermannControlToVehicle(vehicle.GetId(), control);
    }

    rpc::AckermannControllerSettings GetAckermannControllerSettings(const Vehicle &vehicle) const {
      return _client.GetAckermannControllerSettings(vehicle.GetId());
    }

    void ApplyAckermannControllerSettings(Vehicle &vehicle, const rpc::AckermannControllerSettings &settings) {
      _client.ApplyAckermannControllerSettings(vehicle.GetId(), settings);
    }

    void ApplyControlToWalker(Walker &walker, const rpc::WalkerControl &control) {
      _client.ApplyControlToWalker(walker.GetId(), control);
    }

    rpc::WalkerBoneControlOut GetBonesTransform(Walker &walker) {
      return _client.GetBonesTransform(walker.GetId());
    }

    void SetBonesTransform(Walker &walker, const rpc::WalkerBoneControlIn &bones) {
      return _client.SetBonesTransform(walker.GetId(), bones);
    }

    void BlendPose(Walker &walker, float blend) {
      return _client.BlendPose(walker.GetId(), blend);
    }

    void GetPoseFromAnimation(Walker &walker) {
      return _client.GetPoseFromAnimation(walker.GetId());
    }

    void ApplyPhysicsControlToVehicle(Vehicle &vehicle, const rpc::VehiclePhysicsControl &physicsControl) {
      _client.ApplyPhysicsControlToVehicle(vehicle.GetId(), physicsControl);
    }

    void SetLightStateToVehicle(Vehicle &vehicle, const rpc::VehicleLightState light_state) {
      _client.SetLightStateToVehicle(vehicle.GetId(), light_state);
    }

    void OpenVehicleDoor(Vehicle &vehicle, const rpc::VehicleDoor door_idx) {
      _client.OpenVehicleDoor(vehicle.GetId(), door_idx);
    }

    void CloseVehicleDoor(Vehicle &vehicle, const rpc::VehicleDoor door_idx) {
      _client.CloseVehicleDoor(vehicle.GetId(), door_idx);
    }

    void SetWheelSteerDirection(Vehicle &vehicle, rpc::VehicleWheelLocation wheel_location, float angle_in_deg) {
      _client.SetWheelSteerDirection(vehicle.GetId(), wheel_location, angle_in_deg);
    }

    float GetWheelSteerAngle(Vehicle &vehicle, rpc::VehicleWheelLocation wheel_location) {
      return _client.GetWheelSteerAngle(vehicle.GetId(), wheel_location);
    }

    void SetWheelPitchAngle(Vehicle &vehicle, rpc::VehicleWheelLocation wheel_location, float angle_in_deg) {
      _client.SetWheelPitchAngle(vehicle.GetId(), wheel_location, angle_in_deg);
    }

    float GetWheelPitchAngle(Vehicle &vehicle, rpc::VehicleWheelLocation wheel_location) {
      return _client.GetWheelPitchAngle(vehicle.GetId(), wheel_location);
    }

    void EnableCarSim(Vehicle &vehicle, std::string simfile_path) {
      _client.EnableCarSim(vehicle.GetId(), simfile_path);
    }

    void UseCarSimRoad(Vehicle &vehicle, bool enabled) {
      _client.UseCarSimRoad(vehicle.GetId(), enabled);
    }

    void EnableChronoPhysics(Vehicle &vehicle,
        uint64_t MaxSubsteps,
        float MaxSubstepDeltaTime,
        std::string VehicleJSON,
        std::string PowertrainJSON,
        std::string TireJSON,
        std::string BaseJSONPath) {
      _client.EnableChronoPhysics(vehicle.GetId(),
          MaxSubsteps,
          MaxSubstepDeltaTime,
          VehicleJSON,
          PowertrainJSON,
          TireJSON,
          BaseJSONPath);
    }

    void RestorePhysXPhysics(Vehicle &vehicle) {
      _client.RestorePhysXPhysics(vehicle.GetId());
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

    std::string ReplayFile(std::string name, double start, double duration,
        uint32_t follow_id, bool replay_sensors, geom::Transform offset) {
      return _client.ReplayFile(std::move(name), start, duration, follow_id, replay_sensors, offset);
    }

    void SetReplayerTimeFactor(double time_factor) {
      _client.SetReplayerTimeFactor(time_factor);
    }

    void SetReplayerIgnoreHero(bool ignore_hero) {
      _client.SetReplayerIgnoreHero(ignore_hero);
    }

    void SetReplayerIgnoreSpectator(bool ignore_spectator) {
      _client.SetReplayerIgnoreSpectator(ignore_spectator);
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

    void UnSubscribeFromSensor(Actor &sensor);

    void EnableGBuffers(const Sensor &sensor, bool bEnable);

    void DisableForROS(const Sensor &sensor);

    bool IsEnabledForROS(const Sensor &sensor);

    void EnableForROS(const Sensor &sensor);

    void SubscribeToGBuffer(
        Actor & sensor,
        uint32_t gbuffer_id,
        std::function<void(SharedPtr<sensor::SensorData>)> callback);

    void UnSubscribeFromGBuffer(
        Actor & sensor,
        uint32_t gbuffer_id);

    void Send(const Sensor &sensor, std::string message);

    void SetIgnoredVehicles(const Sensor &sensor, const std::vector<ActorId>& vehicle_ids);

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

    std::vector<geom::BoundingBox> GetLightBoxes(const TrafficLight &trafficLight) const {
      return _client.GetLightBoxes(trafficLight.GetId());
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

    void ClearDebugShape() {
      _client.ClearDebugShape();
    }

    void ClearDebugString() {
      _client.ClearDebugString();
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

    void UpdateDayNightCycle(const bool active) const {
      _client.UpdateDayNightCycle(active);
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
    // =========================================================================
    /// @name Texture updating operations
    // =========================================================================
    /// @{

    void ApplyColorTextureToObjects(
        const std::vector<std::string> &objects_name,
        const rpc::MaterialParameter& parameter,
        const rpc::TextureColor& Texture);

    void ApplyColorTextureToObjects(
        const std::vector<std::string> &objects_name,
        const rpc::MaterialParameter& parameter,
        const rpc::TextureFloatColor& Texture);

    std::vector<std::string> GetNamesOfAllObjects() const;

    /// @}

  private:

    bool ShouldUpdateMap(rpc::MapInfo& map_info);

    Client _client;

    SharedPtr<LightManager> _light_manager;

    std::shared_ptr<Episode> _episode;

    const GarbageCollectionPolicy _gc_policy;

    SharedPtr<Map> _cached_map;

    std::string _open_drive_file;
  };

} // namespace detail
} // namespace client
} // namespace carla
