// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/trafficmanager/Constants.h"
#include "carla/rpc/Actor.h"

#include <rpc/client.h>

namespace carla {
namespace traffic_manager {

using constants::Networking::TM_TIMEOUT;
using constants::Networking::TM_DEFAULT_PORT;

/// Provides communication with the rpc of TrafficManagerServer.
class TrafficManagerClient {

public:

  TrafficManagerClient(const TrafficManagerClient &) = default;
  TrafficManagerClient(TrafficManagerClient &&) = default;

  TrafficManagerClient &operator=(const TrafficManagerClient &) = default;
  TrafficManagerClient &operator=(TrafficManagerClient &&) = default;

  /// Parametric constructor to initialize the parameters.
  TrafficManagerClient(
      const std::string &_host,
      const uint16_t &_port)
    : tmhost(_host),
      tmport(_port) {

    /// Create client instance.
      if(!_client) {
        _client = new ::rpc::client(tmhost, tmport);
        _client->set_timeout(TM_TIMEOUT);
      }
  }

  /// Destructor method.
  ~TrafficManagerClient() {
    if(_client) {
      delete _client;
      _client = nullptr;
    }
  };

  /// Set parameters.
  void setServerDetails(const std::string &_host, const uint16_t &_port) {
    tmhost = _host;
    tmport = _port;
  }

  /// Get parameters.
  void getServerDetails(std::string &_host, uint16_t &_port) {
    _host = tmhost;
    _port = tmport;
  }

  /// Register vehicles to remote traffic manager server via RPC client.
  void RegisterVehicle(const std::vector<carla::rpc::Actor> &actor_list) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("register_vehicle", std::move(actor_list));
  }

  /// Unregister vehicles to remote traffic manager server via RPC client.
  void UnregisterVehicle(const std::vector<carla::rpc::Actor> &actor_list) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("unregister_vehicle", std::move(actor_list));
  }

  /// Method to set a vehicle's % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetPercentageSpeedDifference(const carla::rpc::Actor &_actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_percentage_speed_difference", std::move(_actor), percentage);
  }

  /// Method to set a lane offset displacement from the center line.
  /// Positive values imply a right offset while negative ones mean a left one.
  void SetLaneOffset(const carla::rpc::Actor &_actor, const float offset) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_lane_offset", std::move(_actor), offset);
  }

  /// Set a vehicle's exact desired velocity.
  void SetDesiredSpeed(const carla::rpc::Actor &_actor, const float value) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_desired_speed", std::move(_actor), value);
  }

  /// Method to set a global % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetGlobalPercentageSpeedDifference(const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_global_percentage_speed_difference", percentage);
  }

  /// Method to set a global lane offset displacement from the center line.
  /// Positive values imply a right offset while negative ones mean a left one.
  void SetGlobalLaneOffset(const float offset) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_global_lane_offset", offset);
  }

  /// Method to set the automatic management of the vehicle lights
  void SetUpdateVehicleLights(const carla::rpc::Actor &_actor, const bool do_update) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("update_vehicle_lights", std::move(_actor), do_update);
  }

  /// Method to set collision detection rules between vehicles.
  void SetCollisionDetection(const carla::rpc::Actor &reference_actor, const carla::rpc::Actor &other_actor, const bool detect_collision) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_collision_detection", reference_actor, other_actor, detect_collision);
  }

  /// Method to force lane change on a vehicle.
  /// Direction flag can be set to true for left and false for right.
  void SetForceLaneChange(const carla::rpc::Actor &actor, const bool direction) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_force_lane_change", actor, direction);
  }

  /// Enable/disable automatic lane change on a vehicle.
  void SetAutoLaneChange(const carla::rpc::Actor &actor, const bool enable) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_auto_lane_change", actor, enable);
  }

  /// Method to specify how much distance a vehicle should maintain to
  /// the leading vehicle.
  void SetDistanceToLeadingVehicle(const carla::rpc::Actor &actor, const float distance) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_distance_to_leading_vehicle", actor, distance);
  }

  /// Method to specify the % chance of ignoring collisions with any walker.
  void SetPercentageIgnoreWalkers(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_percentage_ignore_walkers", actor, percentage);
  }

  /// Method to specify the % chance of ignoring collisions with any vehicle.
  void SetPercentageIgnoreVehicles(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_percentage_ignore_vehicles", actor, percentage);
  }

  /// Method to specify the % chance of running a traffic sign.
  void SetPercentageRunningLight(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_percentage_running_light", actor, percentage);
  }

  /// Method to specify the % chance of running any traffic sign.
  void SetPercentageRunningSign(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_percentage_running_sign", actor, percentage);
  }

  /// Method to switch traffic manager into synchronous execution.
  void SetSynchronousMode(const bool mode) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_synchronous_mode", mode);
  }

  /// Method to set tick timeout for synchronous execution.
  void SetSynchronousModeTimeOutInMiliSecond(const double time) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_synchronous_mode_timeout_in_milisecond", time);
  }

  /// Method to provide synchronous tick.
  bool SynchronousTick() {
    DEBUG_ASSERT(_client != nullptr);
    return _client->call("synchronous_tick").as<bool>();
  }

  /// Check if remote traffic manager is alive
  void HealthCheckRemoteTM() {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("health_check_remote_TM");
  }

  /// Method to specify how much distance a vehicle should maintain to
  /// the Global leading vehicle.
  void SetGlobalDistanceToLeadingVehicle(const float distance) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_global_distance_to_leading_vehicle",distance);
  }

  /// Method to set % to keep on the right lane.
  void SetKeepRightPercentage(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("keep_right_rule_percentage", actor, percentage);
  }

  /// Method to set % to randomly do a left lane change.
  void SetRandomLeftLaneChangePercentage(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("random_left_lanechange_percentage", actor, percentage);
  }

  /// Method to set % to randomly do a right lane change.
  void SetRandomRightLaneChangePercentage(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("random_right_lanechange_percentage", actor, percentage);
  }

  /// Method to set hybrid physics mode.
  void SetHybridPhysicsMode(const bool mode_switch) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_hybrid_physics_mode", mode_switch);
  }

  /// Method to set hybrid physics mode.
  void SetHybridPhysicsRadius(const float radius) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_hybrid_physics_radius", radius);
  }

  /// Method to set randomization seed.
  void SetRandomDeviceSeed(const uint64_t seed) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_random_device_seed", seed);
  }

  /// Method to set Open Street Map mode.
  void SetOSMMode(const bool mode_switch) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_osm_mode", mode_switch);
  }

  /// Method to set our own imported path.
  void SetCustomPath(const carla::rpc::Actor &actor, const Path path, const bool empty_buffer) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_path", actor, path, empty_buffer);
  }

  /// Method to remove a list of points.
  void RemoveUploadPath(const ActorId &actor_id, const bool remove_path) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("remove_custom_path", actor_id, remove_path);
  }

  /// Method to update an already set list of points.
  void UpdateUploadPath(const ActorId &actor_id, const Path path) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("update_custom_path", actor_id, path);
  }

  /// Method to set our own imported route.
  void SetImportedRoute(const carla::rpc::Actor &actor, const Route route, const bool empty_buffer) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_imported_route", actor, route, empty_buffer);
  }

  /// Method to remove a route.
  void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("remove_imported_route", actor_id, remove_path);
  }

  /// Method to update an already set list of points.
  void UpdateImportedRoute(const ActorId &actor_id, const Route route) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("update_imported_route", actor_id, route);
  }

  /// Method to set automatic respawn of dormant vehicles.
  void SetRespawnDormantVehicles(const bool mode_switch) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_respawn_dormant_vehicles", mode_switch);
  }

  /// Method to set boundaries for respawning vehicles.
  void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_boundaries_respawn_dormant_vehicles", lower_bound, upper_bound);
  }

  /// Method to set boundaries for respawning vehicles.
  void SetMaxBoundaries(const float lower, const float upper) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_max_boundaries", lower, upper);
  }

  /// Method to get the vehicle's next action.
  Action GetNextAction(const ActorId &actor_id) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("get_next_action", actor_id);
    return Action();
  }

  /// Method to get the vehicle's action buffer.
  ActionBuffer GetActionBuffer(const ActorId &actor_id) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("get_all_actions", actor_id);
    return ActionBuffer();
  }

  void ShutDown() {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("shut_down");
  }

private:

  /// RPC client.
  ::rpc::client *_client = nullptr;

  /// Server port and host.
  std::string tmhost;
  uint16_t    tmport;
};

} // namespace traffic_manager
} // namespace carla
