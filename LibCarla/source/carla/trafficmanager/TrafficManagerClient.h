// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include <rpc/client.h>

#define TM_TIMEOUT        2000 // In ms
#define TM_DEFAULT_PORT   8000 // TM_SERVER_PORT

namespace carla {
namespace traffic_manager {

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

  /// Method to set a global % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetGlobalPercentageSpeedDifference(const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_global_percentage_speed_difference", percentage);
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

  /// Method to reset all traffic light groups to the initial stage.
  void ResetAllTrafficLights() {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("reset_all_traffic_lights");
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

  /// Method to set probabilistic preference to keep on the right lane.
  void SetKeepRightPercentage(const carla::rpc::Actor &actor, const float percentage) {
    DEBUG_ASSERT(_client != nullptr);
    _client->call("set_percentage_keep_right_rule", actor, percentage);
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
