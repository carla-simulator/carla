// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Client.h"

#include "carla/Exception.h"
#include "carla/Version.h"
#include "carla/client/TimeoutException.h"
#include "carla/rpc/ActorDescription.h"
#include "carla/rpc/BoneTransformData.h"
#include "carla/rpc/Client.h"
#include "carla/rpc/DebugShape.h"
#include "carla/rpc/Response.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/rpc/VehicleLightState.h"
#include "carla/rpc/WalkerBoneControl.h"
#include "carla/rpc/WalkerControl.h"
#include "carla/streaming/Client.h"

#include <rpc/rpc_error.h>

#include <thread>

namespace carla {
namespace client {
namespace detail {

  template <typename T>
  static T Get(carla::rpc::Response<T> &response) {
    return response.Get();
  }

  static bool Get(carla::rpc::Response<void> &) {
    return true;
  }

  // ===========================================================================
  // -- Client::Pimpl ----------------------------------------------------------
  // ===========================================================================

  class Client::Pimpl {
  public:

    Pimpl(const std::string &host, uint16_t port, size_t worker_threads)
      : endpoint(host + ":" + std::to_string(port)),
        rpc_client(host, port),
        streaming_client(host) {
      rpc_client.set_timeout(5000u);
      streaming_client.AsyncRun(
          worker_threads > 0u ? worker_threads : std::thread::hardware_concurrency());
    }

    template <typename ... Args>
    auto RawCall(const std::string &function, Args && ... args) {
      try {
        return rpc_client.call(function, std::forward<Args>(args) ...);
      } catch (const ::rpc::timeout &) {
        throw_exception(TimeoutException(endpoint, GetTimeout()));
      }
    }

    template <typename T, typename ... Args>
    auto CallAndWait(const std::string &function, Args && ... args) {
      auto object = RawCall(function, std::forward<Args>(args) ...);
      using R = typename carla::rpc::Response<T>;
      auto response = object.template as<R>();
      if (response.HasError()) {
        throw_exception(std::runtime_error(response.GetError().What()));
      }
      return Get(response);
    }

    template <typename ... Args>
    void AsyncCall(const std::string &function, Args && ... args) {
      // Discard returned future.
      rpc_client.async_call(function, std::forward<Args>(args) ...);
    }

    time_duration GetTimeout() const {
      auto timeout = rpc_client.get_timeout();
      DEBUG_ASSERT(timeout.has_value());
      return time_duration::milliseconds(static_cast<size_t>(*timeout));
    }

    const std::string endpoint;

    rpc::Client rpc_client;

    streaming::Client streaming_client;
  };

  // ===========================================================================
  // -- Client -----------------------------------------------------------------
  // ===========================================================================

  Client::Client(
      const std::string &host,
      const uint16_t port,
      const size_t worker_threads)
    : _pimpl(std::make_unique<Pimpl>(host, port, worker_threads)) {}

  bool Client::IsTrafficManagerRunning(uint16_t port) const {
    return _pimpl->CallAndWait<bool>("is_traffic_manager_running", port);
  }

  std::pair<std::string, uint16_t> Client::GetTrafficManagerRunning(uint16_t port) const {
    return _pimpl->CallAndWait<std::pair<std::string, uint16_t>>("get_traffic_manager_running", port);
  };

  bool Client::AddTrafficManagerRunning(std::pair<std::string, uint16_t> trafficManagerInfo) const {
    return _pimpl->CallAndWait<bool>("add_traffic_manager_running", trafficManagerInfo);
  };

  void Client::DestroyTrafficManager(uint16_t port) const {
    _pimpl->AsyncCall("destroy_traffic_manager", port);
  }

  Client::~Client() = default;

  void Client::SetTimeout(time_duration timeout) {
    _pimpl->rpc_client.set_timeout(static_cast<int64_t>(timeout.milliseconds()));
  }

  time_duration Client::GetTimeout() const {
    return _pimpl->GetTimeout();
  }

  const std::string Client::GetEndpoint() const {
    return _pimpl->endpoint;
  }

  std::string Client::GetClientVersion() {
    return ::carla::version();
  }

  std::string Client::GetServerVersion() {
    return _pimpl->CallAndWait<std::string>("version");
  }

  void Client::LoadEpisode(std::string map_name) {
    // Await response, we need to be sure in this one.
    _pimpl->CallAndWait<void>("load_new_episode", std::move(map_name));
  }

  void Client::CopyOpenDriveToServer(std::string opendrive, const rpc::OpendriveGenerationParameters & params) {
    // Await response, we need to be sure in this one.
    _pimpl->CallAndWait<void>("copy_opendrive_to_file", std::move(opendrive), params);
  }

  rpc::EpisodeInfo Client::GetEpisodeInfo() {
    return _pimpl->CallAndWait<rpc::EpisodeInfo>("get_episode_info");
  }

  rpc::MapInfo Client::GetMapInfo() {
    return _pimpl->CallAndWait<rpc::MapInfo>("get_map_info");
  }

  std::vector<uint8_t> Client::GetNavigationMesh() const {
    return _pimpl->CallAndWait<std::vector<uint8_t>>("get_navigation_mesh");
  }

  std::vector<std::string> Client::GetAvailableMaps() {
    return _pimpl->CallAndWait<std::vector<std::string>>("get_available_maps");
  }

  std::vector<rpc::ActorDefinition> Client::GetActorDefinitions() {
    return _pimpl->CallAndWait<std::vector<rpc::ActorDefinition>>("get_actor_definitions");
  }

  rpc::Actor Client::GetSpectator() {
    return _pimpl->CallAndWait<carla::rpc::Actor>("get_spectator");
  }

  rpc::EpisodeSettings Client::GetEpisodeSettings() {
    return _pimpl->CallAndWait<rpc::EpisodeSettings>("get_episode_settings");
  }

  uint64_t Client::SetEpisodeSettings(const rpc::EpisodeSettings &settings) {
    return _pimpl->CallAndWait<uint64_t>("set_episode_settings", settings);
  }

  rpc::WeatherParameters Client::GetWeatherParameters() {
    return _pimpl->CallAndWait<rpc::WeatherParameters>("get_weather_parameters");
  }

  void Client::SetWeatherParameters(const rpc::WeatherParameters &weather) {
    _pimpl->AsyncCall("set_weather_parameters", weather);
  }

  std::vector<rpc::Actor> Client::GetActorsById(
      const std::vector<ActorId> &ids) {
    using return_t = std::vector<rpc::Actor>;
    return _pimpl->CallAndWait<return_t>("get_actors_by_id", ids);
  }

  rpc::VehiclePhysicsControl Client::GetVehiclePhysicsControl(
      rpc::ActorId vehicle) const {
    return _pimpl->CallAndWait<carla::rpc::VehiclePhysicsControl>("get_physics_control", vehicle);
  }

  rpc::VehicleLightState Client::GetVehicleLightState(
      rpc::ActorId vehicle) const {
    return _pimpl->CallAndWait<carla::rpc::VehicleLightState>("get_vehicle_light_state", vehicle);
  }

  void Client::ApplyPhysicsControlToVehicle(
      rpc::ActorId vehicle,
      const rpc::VehiclePhysicsControl &physics_control) {
    return _pimpl->AsyncCall("apply_physics_control", vehicle, physics_control);
  }

  void Client::SetLightStateToVehicle(
      rpc::ActorId vehicle,
      const rpc::VehicleLightState &light_state) {
    return _pimpl->AsyncCall("set_vehicle_light_state", vehicle, light_state);
  }

  rpc::Actor Client::SpawnActor(
      const rpc::ActorDescription &description,
      const geom::Transform &transform) {
    return _pimpl->CallAndWait<rpc::Actor>("spawn_actor", description, transform);
  }

  rpc::Actor Client::SpawnActorWithParent(
      const rpc::ActorDescription &description,
      const geom::Transform &transform,
      rpc::ActorId parent,
      rpc::AttachmentType attachment_type) {

      if(attachment_type == rpc::AttachmentType::SpringArm) {
        const auto a = transform.location.MakeSafeUnitVector(std::numeric_limits<float>::epsilon());
        const auto z = geom::Vector3D(0.0f, 0.f, 1.0f);
        constexpr float OneEps = 1.0f - std::numeric_limits<float>::epsilon();
        if (geom::Math::Dot(a, z) > OneEps) {
          std::cout << "WARNING: Transformations with translation only in the 'z' axis are ill-formed when \
            using SprintArm attachment. Please, be careful with that." << std::endl;
        }
      }

    return _pimpl->CallAndWait<rpc::Actor>("spawn_actor_with_parent",
        description,
        transform,
        parent,
        attachment_type);
  }

  bool Client::DestroyActor(rpc::ActorId actor) {
    try {
      return _pimpl->CallAndWait<void>("destroy_actor", actor);
    } catch (const std::exception &e) {
      log_error("failed to destroy actor", actor, ':', e.what());
      return false;
    }
  }

  void Client::SetActorLocation(rpc::ActorId actor, const geom::Location &location) {
    _pimpl->AsyncCall("set_actor_location", actor, location);
  }

  void Client::SetActorTransform(rpc::ActorId actor, const geom::Transform &transform) {
    _pimpl->AsyncCall("set_actor_transform", actor, transform);
  }

  void Client::SetActorTargetVelocity(rpc::ActorId actor, const geom::Vector3D &vector) {
    _pimpl->AsyncCall("set_actor_target_velocity", actor, vector);
  }

  void Client::SetActorTargetAngularVelocity(rpc::ActorId actor, const geom::Vector3D &vector) {
    _pimpl->AsyncCall("set_actor_target_angular_velocity", actor, vector);
  }

  void Client::EnableActorConstantVelocity(rpc::ActorId actor, const geom::Vector3D &vector) {
    _pimpl->AsyncCall("enable_actor_constant_velocity", actor, vector);
  }

  void Client::DisableActorConstantVelocity(rpc::ActorId actor) {
    _pimpl->AsyncCall("disable_actor_constant_velocity", actor);
  }

  void Client::AddActorImpulse(rpc::ActorId actor, const geom::Vector3D &impulse) {
    _pimpl->AsyncCall("add_actor_impulse", actor, impulse);
  }

  void Client::AddActorImpulse(rpc::ActorId actor, const geom::Vector3D &impulse, const geom::Vector3D &location) {
    _pimpl->AsyncCall("add_actor_impulse_at_location", actor, impulse, location);
  }

  void Client::AddActorForce(rpc::ActorId actor, const geom::Vector3D &force) {
    _pimpl->AsyncCall("add_actor_force", actor, force);
  }

  void Client::AddActorForce(rpc::ActorId actor, const geom::Vector3D &force, const geom::Vector3D &location) {
    _pimpl->AsyncCall("add_actor_force_at_location", actor, force, location);
  }

  void Client::AddActorAngularImpulse(rpc::ActorId actor, const geom::Vector3D &vector) {
    _pimpl->AsyncCall("add_actor_angular_impulse", actor, vector);
  }

  void Client::AddActorTorque(rpc::ActorId actor, const geom::Vector3D &vector) {
    _pimpl->AsyncCall("add_actor_torque", actor, vector);
  }

  void Client::SetActorSimulatePhysics(rpc::ActorId actor, const bool enabled) {
    _pimpl->AsyncCall("set_actor_simulate_physics", actor, enabled);
  }

  void Client::SetActorEnableGravity(rpc::ActorId actor, const bool enabled) {
    _pimpl->AsyncCall("set_actor_enable_gravity", actor, enabled);
  }

  void Client::SetActorAutopilot(rpc::ActorId vehicle, const bool enabled) {
    _pimpl->AsyncCall("set_actor_autopilot", vehicle, enabled);
  }

  void Client::ApplyControlToVehicle(rpc::ActorId vehicle, const rpc::VehicleControl &control) {
    _pimpl->AsyncCall("apply_control_to_vehicle", vehicle, control);
  }

  void Client::ApplyControlToWalker(rpc::ActorId walker, const rpc::WalkerControl &control) {
    _pimpl->AsyncCall("apply_control_to_walker", walker, control);
  }

  void Client::ApplyBoneControlToWalker(rpc::ActorId walker, const rpc::WalkerBoneControl &control) {
    _pimpl->AsyncCall("apply_bone_control_to_walker", walker, control);
  }

  void Client::SetTrafficLightState(
      rpc::ActorId traffic_light,
      const rpc::TrafficLightState traffic_light_state) {
    _pimpl->AsyncCall("set_traffic_light_state", traffic_light, traffic_light_state);
  }

  void Client::SetTrafficLightGreenTime(rpc::ActorId traffic_light, float green_time) {
    _pimpl->AsyncCall("set_traffic_light_green_time", traffic_light, green_time);
  }

  void Client::SetTrafficLightYellowTime(rpc::ActorId traffic_light, float yellow_time) {
    _pimpl->AsyncCall("set_traffic_light_yellow_time", traffic_light, yellow_time);
  }

  void Client::SetTrafficLightRedTime(rpc::ActorId traffic_light, float red_time) {
    _pimpl->AsyncCall("set_traffic_light_red_time", traffic_light, red_time);
  }

  void Client::FreezeTrafficLight(rpc::ActorId traffic_light, bool freeze) {
    _pimpl->AsyncCall("freeze_traffic_light", traffic_light, freeze);
  }

  void Client::ResetTrafficLightGroup(rpc::ActorId traffic_light) {
    _pimpl->AsyncCall("reset_traffic_light_group", traffic_light);
  }

  void Client::ResetAllTrafficLights() {
    _pimpl->CallAndWait<void>("reset_all_traffic_lights");
  }

  void Client::FreezeAllTrafficLights(bool frozen) {
    _pimpl->AsyncCall("freeze_all_traffic_lights", frozen);
  }

  rpc::VehicleLightStateList Client::GetVehiclesLightStates() {
    return _pimpl->CallAndWait<std::vector<std::pair<carla::ActorId, uint32_t>>>("get_vehicle_light_states");
  }

  std::vector<ActorId> Client::GetGroupTrafficLights(rpc::ActorId traffic_light) {
    using return_t = std::vector<ActorId>;
    return _pimpl->CallAndWait<return_t>("get_group_traffic_lights", traffic_light);
  }

  std::string Client::StartRecorder(std::string name, bool additional_data) {
    return _pimpl->CallAndWait<std::string>("start_recorder", name, additional_data);
  }

  void Client::StopRecorder() {
    return _pimpl->AsyncCall("stop_recorder");
  }

  std::string Client::ShowRecorderFileInfo(std::string name, bool show_all) {
    return _pimpl->CallAndWait<std::string>("show_recorder_file_info", name, show_all);
  }

  std::string Client::ShowRecorderCollisions(std::string name, char type1, char type2) {
    return _pimpl->CallAndWait<std::string>("show_recorder_collisions", name, type1, type2);
  }

  std::string Client::ShowRecorderActorsBlocked(std::string name, double min_time, double min_distance) {
    return _pimpl->CallAndWait<std::string>("show_recorder_actors_blocked", name, min_time, min_distance);
  }

  std::string Client::ReplayFile(std::string name, double start, double duration, uint32_t follow_id) {
    return _pimpl->CallAndWait<std::string>("replay_file", name, start, duration, follow_id);
  }

  void Client::StopReplayer(bool keep_actors) {
    _pimpl->AsyncCall("stop_replayer", keep_actors);
  }

  void Client::SetReplayerTimeFactor(double time_factor) {
    _pimpl->AsyncCall("set_replayer_time_factor", time_factor);
  }

  void Client::SetReplayerIgnoreHero(bool ignore_hero) {
    _pimpl->AsyncCall("set_replayer_ignore_hero", ignore_hero);
  }

  void Client::SubscribeToStream(
      const streaming::Token &token,
      std::function<void(Buffer)> callback) {
    _pimpl->streaming_client.Subscribe(token, std::move(callback));
  }

  void Client::UnSubscribeFromStream(const streaming::Token &token) {
    _pimpl->streaming_client.UnSubscribe(token);
  }

  void Client::DrawDebugShape(const rpc::DebugShape &shape) {
    _pimpl->AsyncCall("draw_debug_shape", shape);
  }

  void Client::ApplyBatch(std::vector<rpc::Command> commands, bool do_tick_cue) {
    _pimpl->AsyncCall("apply_batch", std::move(commands), do_tick_cue);
  }

  std::vector<rpc::CommandResponse> Client::ApplyBatchSync(
      std::vector<rpc::Command> commands,
      bool do_tick_cue) {
    auto result = _pimpl->RawCall("apply_batch", std::move(commands), do_tick_cue);
    return result.as<std::vector<rpc::CommandResponse>>();
  }

  uint64_t Client::SendTickCue() {
    return _pimpl->CallAndWait<uint64_t>("tick_cue");
  }

  std::vector<rpc::LightState> Client::QueryLightsStateToServer() const {
    using return_t = std::vector<rpc::LightState>;
    return _pimpl->CallAndWait<return_t>("query_lights_state", _pimpl->endpoint);
  }

  void Client::UpdateServerLightsState(std::vector<rpc::LightState>& lights, bool discard_client) const {
    _pimpl->AsyncCall("update_lights_state", _pimpl->endpoint, std::move(lights), discard_client);
  }

  std::vector<geom::BoundingBox> Client::GetLevelBBs(uint8_t queried_tag) const {
    using return_t = std::vector<geom::BoundingBox>;
    return _pimpl->CallAndWait<return_t>("get_all_level_BBs", queried_tag);
  }

} // namespace detail
} // namespace client
} // namespace carla
