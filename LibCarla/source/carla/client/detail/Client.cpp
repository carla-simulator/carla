// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Client.h"

#include "carla/Exception.h"
#include "carla/Version.h"
#include "carla/client/FileTransfer.h"
#include "carla/client/TimeoutException.h"
#include "carla/rpc/AckermannControllerSettings.h"
#include "carla/rpc/ActorDescription.h"
#include "carla/rpc/BoneTransformDataIn.h"
#include "carla/rpc/Client.h"
#include "carla/rpc/DebugShape.h"
#include "carla/rpc/Response.h"
#include "carla/rpc/VehicleAckermannControl.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/rpc/VehicleLightState.h"
#include "carla/rpc/WalkerBoneControlIn.h"
#include "carla/rpc/WalkerBoneControlOut.h"
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

  void Client::LoadEpisode(std::string map_name, bool reset_settings, rpc::MapLayer map_layer) {
    // Await response, we need to be sure in this one.
    _pimpl->CallAndWait<void>("load_new_episode", std::move(map_name), reset_settings, map_layer);
  }

  void Client::LoadLevelLayer(rpc::MapLayer map_layer) const {
    // Await response, we need to be sure in this one.
    _pimpl->CallAndWait<void>("load_map_layer", map_layer);
  }

  void Client::UnloadLevelLayer(rpc::MapLayer map_layer) const {
    // Await response, we need to be sure in this one.
    _pimpl->CallAndWait<void>("unload_map_layer", map_layer);
  }

  void Client::CopyOpenDriveToServer(std::string opendrive, const rpc::OpendriveGenerationParameters & params) {
    // Await response, we need to be sure in this one.
    _pimpl->CallAndWait<void>("copy_opendrive_to_file", std::move(opendrive), params);
  }

  void Client::ApplyColorTextureToObjects(
      const std::vector<std::string> &objects_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureColor& Texture) {
    _pimpl->CallAndWait<void>("apply_color_texture_to_objects", objects_name, parameter, Texture);
  }

  void Client::ApplyColorTextureToObjects(
      const std::vector<std::string> &objects_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureFloatColor& Texture) {
    _pimpl->CallAndWait<void>("apply_float_color_texture_to_objects", objects_name, parameter, Texture);
  }

  std::vector<std::string> Client::GetNamesOfAllObjects() const {
    return _pimpl->CallAndWait<std::vector<std::string>>("get_names_of_all_objects");
  }

  rpc::EpisodeInfo Client::GetEpisodeInfo() {
    return _pimpl->CallAndWait<rpc::EpisodeInfo>("get_episode_info");
  }

  rpc::MapInfo Client::GetMapInfo() {
    return _pimpl->CallAndWait<rpc::MapInfo>("get_map_info");
  }

  std::string Client::GetMapData() const{
    return _pimpl->CallAndWait<std::string>("get_map_data");
  }

  std::vector<uint8_t> Client::GetNavigationMesh() const {
    return _pimpl->CallAndWait<std::vector<uint8_t>>("get_navigation_mesh");
  }

  bool Client::SetFilesBaseFolder(const std::string &path) {
    return FileTransfer::SetFilesBaseFolder(path);
  }

  std::vector<std::string> Client::GetRequiredFiles(const std::string &folder, const bool download) const {
    // Get the list of required files
    auto requiredFiles = _pimpl->CallAndWait<std::vector<std::string>>("get_required_files", folder);

    if (download) {

      // For each required file, check if it exists and request it otherwise
      for (auto requiredFile : requiredFiles) {
        if (!FileTransfer::FileExists(requiredFile)) {
          RequestFile(requiredFile);
          log_info("Could not find the required file in cache, downloading... ", requiredFile);
        } else {
          log_info("Found the required file in cache! ", requiredFile);
        }
      }
    }
    return requiredFiles;
  }

  void Client::RequestFile(const std::string &name) const {
    // Download the binary content of the file from the server and write it on the client
    auto content = _pimpl->CallAndWait<std::vector<uint8_t>>("request_file", name);
    FileTransfer::WriteFile(name, content);
  }

  std::vector<uint8_t> Client::GetCacheFile(const std::string &name, const bool request_otherwise) const {
    // Get the file from the cache in the file transfer
    std::vector<uint8_t> file = FileTransfer::ReadFile(name);

    // If it isn't in the cache, download it if request otherwise is true
    if (file.empty() && request_otherwise) {
      RequestFile(name);
      file = FileTransfer::ReadFile(name);
    }
    return file;
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

  void Client::OpenVehicleDoor(
      rpc::ActorId vehicle,
      const rpc::VehicleDoor door_idx) {
    return _pimpl->AsyncCall("open_vehicle_door", vehicle, door_idx);
  }

  void Client::CloseVehicleDoor(
      rpc::ActorId vehicle,
      const rpc::VehicleDoor door_idx) {
    return _pimpl->AsyncCall("close_vehicle_door", vehicle, door_idx);
  }

  void Client::SetWheelSteerDirection(
        rpc::ActorId vehicle,
        rpc::VehicleWheelLocation vehicle_wheel,
        float angle_in_deg) {
    return _pimpl->AsyncCall("set_wheel_steer_direction", vehicle, vehicle_wheel, angle_in_deg);
  }

  float Client::GetWheelSteerAngle(
        rpc::ActorId vehicle,
        rpc::VehicleWheelLocation wheel_location){
    return _pimpl->CallAndWait<float>("get_wheel_steer_angle", vehicle, wheel_location);
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
      return _pimpl->CallAndWait<bool>("destroy_actor", actor);
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
    _pimpl->CallAndWait<void>("set_actor_simulate_physics", actor, enabled);
  }

  void Client::SetActorEnableGravity(rpc::ActorId actor, const bool enabled) {
    _pimpl->AsyncCall("set_actor_enable_gravity", actor, enabled);
  }

  void Client::SetActorAutopilot(rpc::ActorId vehicle, const bool enabled) {
    _pimpl->AsyncCall("set_actor_autopilot", vehicle, enabled);
  }

  void Client::ShowVehicleDebugTelemetry(rpc::ActorId vehicle, const bool enabled) {
    _pimpl->AsyncCall("show_vehicle_debug_telemetry", vehicle, enabled);
  }

  void Client::ApplyControlToVehicle(rpc::ActorId vehicle, const rpc::VehicleControl &control) {
    _pimpl->AsyncCall("apply_control_to_vehicle", vehicle, control);
  }

  void Client::ApplyAckermannControlToVehicle(rpc::ActorId vehicle, const rpc::VehicleAckermannControl &control) {
    _pimpl->AsyncCall("apply_ackermann_control_to_vehicle", vehicle, control);
  }

  rpc::AckermannControllerSettings Client::GetAckermannControllerSettings(
      rpc::ActorId vehicle) const {
    return _pimpl->CallAndWait<carla::rpc::AckermannControllerSettings>("get_ackermann_controller_settings", vehicle);
  }

  void Client::ApplyAckermannControllerSettings(rpc::ActorId vehicle, const rpc::AckermannControllerSettings &settings) {
    _pimpl->AsyncCall("apply_ackermann_controller_settings", vehicle, settings);
  }

  void Client::EnableCarSim(rpc::ActorId vehicle, std::string simfile_path) {
    _pimpl->AsyncCall("enable_carsim", vehicle, simfile_path);
  }

  void Client::UseCarSimRoad(rpc::ActorId vehicle, bool enabled) {
    _pimpl->AsyncCall("use_carsim_road", vehicle, enabled);
  }

  void Client::EnableChronoPhysics(
      rpc::ActorId vehicle,
      uint64_t MaxSubsteps,
      float MaxSubstepDeltaTime,
      std::string VehicleJSON,
      std::string PowertrainJSON,
      std::string TireJSON,
      std::string BaseJSONPath) {
    _pimpl->AsyncCall("enable_chrono_physics",
        vehicle,
        MaxSubsteps,
        MaxSubstepDeltaTime,
        VehicleJSON,
        PowertrainJSON,
        TireJSON,
        BaseJSONPath);
  }

  void Client::ApplyControlToWalker(rpc::ActorId walker, const rpc::WalkerControl &control) {
    _pimpl->AsyncCall("apply_control_to_walker", walker, control);
  }

  rpc::WalkerBoneControlOut Client::GetBonesTransform(rpc::ActorId walker) {
    auto res = _pimpl->CallAndWait<rpc::WalkerBoneControlOut>("get_bones_transform", walker);
    return res;
  }

  void Client::SetBonesTransform(rpc::ActorId walker, const rpc::WalkerBoneControlIn &bones) {
    _pimpl->AsyncCall("set_bones_transform", walker, bones);
  }

  void Client::BlendPose(rpc::ActorId walker, float blend) {
    _pimpl->AsyncCall("blend_pose", walker, blend);
  }

  void Client::GetPoseFromAnimation(rpc::ActorId walker) {
    _pimpl->AsyncCall("get_pose_from_animation", walker);
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

  std::vector<geom::BoundingBox> Client::GetLightBoxes(rpc::ActorId traffic_light) const {
    using return_t = std::vector<geom::BoundingBox>;
    return _pimpl->CallAndWait<return_t>("get_light_boxes", traffic_light);
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

  std::string Client::ReplayFile(std::string name, double start, double duration,
      uint32_t follow_id, bool replay_sensors) {
    return _pimpl->CallAndWait<std::string>("replay_file", name, start, duration,
        follow_id, replay_sensors);
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
    carla::streaming::detail::token_type thisToken(token);
    streaming::Token receivedToken = _pimpl->CallAndWait<streaming::Token>("get_sensor_token", thisToken.get_stream_id());
    _pimpl->streaming_client.Subscribe(receivedToken, std::move(callback));
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

  void Client::UpdateDayNightCycle(const bool active) const {
    _pimpl->AsyncCall("update_day_night_cycle", _pimpl->endpoint, active);
  }

  std::vector<geom::BoundingBox> Client::GetLevelBBs(uint8_t queried_tag) const {
    using return_t = std::vector<geom::BoundingBox>;
    return _pimpl->CallAndWait<return_t>("get_all_level_BBs", queried_tag);
  }

  std::vector<rpc::EnvironmentObject> Client::GetEnvironmentObjects(uint8_t queried_tag) const {
    using return_t = std::vector<rpc::EnvironmentObject>;
    return _pimpl->CallAndWait<return_t>("get_environment_objects", queried_tag);
  }

  void Client::EnableEnvironmentObjects(
      std::vector<uint64_t> env_objects_ids,
      bool enable) const {
    _pimpl->AsyncCall("enable_environment_objects", std::move(env_objects_ids), enable);
  }

  std::pair<bool,rpc::LabelledPoint> Client::ProjectPoint(
      geom::Location location, geom::Vector3D direction, float search_distance) const {
    using return_t = std::pair<bool,rpc::LabelledPoint>;
    return _pimpl->CallAndWait<return_t>("project_point", location, direction, search_distance);
  }

  std::vector<rpc::LabelledPoint> Client::CastRay(
      geom::Location start_location, geom::Location end_location) const {
    using return_t = std::vector<rpc::LabelledPoint>;
    return _pimpl->CallAndWait<return_t>("cast_ray", start_location, end_location);
  }

} // namespace detail
} // namespace client
} // namespace carla
