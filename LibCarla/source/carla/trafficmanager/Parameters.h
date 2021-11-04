// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <chrono>
#include <random>
#include <unordered_map>

#include "carla/client/Actor.h"
#include "carla/client/Vehicle.h"
#include "carla/Memory.h"
#include "carla/rpc/ActorId.h"

#include "carla/trafficmanager/AtomicActorSet.h"
#include "carla/trafficmanager/AtomicMap.h"

namespace carla {
namespace traffic_manager {

namespace cc = carla::client;
namespace cg = carla::geom;
using ActorPtr = carla::SharedPtr<cc::Actor>;
using ActorId = carla::ActorId;
using Path = std::vector<cg::Location>;
using Route = std::vector<uint8_t>;

struct ChangeLaneInfo {
  bool change_lane = false;
  bool direction = false;
};

class Parameters {

private:
  /// Target velocity map for individual vehicles.
  AtomicMap<ActorId, float> percentage_difference_from_speed_limit;
  /// Global target velocity limit % difference.
  float global_percentage_difference_from_limit = 0;
  /// Map containing a set of actors to be ignored during collision detection.
  AtomicMap<ActorId, std::shared_ptr<AtomicActorSet>> ignore_collision;
  /// Map containing distance to leading vehicle command.
  AtomicMap<ActorId, float> distance_to_leading_vehicle;
  /// Map containing force lane change commands.
  AtomicMap<ActorId, ChangeLaneInfo> force_lane_change;
  /// Map containing auto lane change commands.
  AtomicMap<ActorId, bool> auto_lane_change;
  /// Map containing % of running a traffic light.
  AtomicMap<ActorId, float> perc_run_traffic_light;
  /// Map containing % of running a traffic sign.
  AtomicMap<ActorId, float> perc_run_traffic_sign;
  /// Map containing % of ignoring walkers.
  AtomicMap<ActorId, float> perc_ignore_walkers;
  /// Map containing % of ignoring vehicles.
  AtomicMap<ActorId, float> perc_ignore_vehicles;
  /// Map containing % of keep right rule.
  AtomicMap<ActorId, float> perc_keep_right;
  /// Map containing % of random left lane change.
  AtomicMap<ActorId, float> perc_random_left;
  /// Map containing % of random right lane change.
  AtomicMap<ActorId, float> perc_random_right;
  /// Map containing the automatic vehicle lights update flag
  AtomicMap<ActorId, bool> auto_update_vehicle_lights;
  /// Synchronous mode switch.
  std::atomic<bool> synchronous_mode{false};
  /// Distance margin
  std::atomic<float> distance_margin{2.0};
  /// Hybrid physics mode switch.
  std::atomic<bool> hybrid_physics_mode{false};
  /// Automatic respawn mode switch.
  std::atomic<bool> respawn_dormant_vehicles{false};
  /// Minimum distance to respawn vehicles with respect to the hero vehicle.
  std::atomic<float> respawn_lower_bound{100.0};
  /// Maximum distance to respawn vehicles with respect to the hero vehicle.
  std::atomic<float> respawn_upper_bound{1000.0};
  /// Minimum possible distance to respawn vehicles with respect to the hero vehicle.
  float min_lower_bound;
  /// Maximum possible distance to respawn vehicles with respect to the hero vehicle.
  float max_upper_bound;
  /// Hybrid physics radius.
  std::atomic<float> hybrid_physics_radius {70.0};
  /// Parameter specifying Open Street Map mode.
  std::atomic<bool> osm_mode {true};
  /// Parameter specifying if importing a custom path.
  AtomicMap<ActorId, bool> upload_path;
  /// Structure to hold all custom paths.
  AtomicMap<ActorId, Path> custom_path;
  /// Parameter specifying if importing a custom route.
  AtomicMap<ActorId, bool> upload_route;
  /// Structure to hold all custom routes.
  AtomicMap<ActorId, Route> custom_route;

public:
  Parameters();
  ~Parameters();

  ////////////////////////////////// SETTERS /////////////////////////////////////

  /// Set a vehicle's % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage);

  /// Set a global % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  void SetGlobalPercentageSpeedDifference(float const percentage);

  /// Method to set collision detection rules between vehicles.
  void SetCollisionDetection(
      const ActorPtr &reference_actor,
      const ActorPtr &other_actor,
      const bool detect_collision);

  /// Method to force lane change on a vehicle.
  /// Direction flag can be set to true for left and false for right.
  void SetForceLaneChange(const ActorPtr &actor, const bool direction);

  /// Enable/disable automatic lane change on a vehicle.
  void SetAutoLaneChange(const ActorPtr &actor, const bool enable);

  /// Method to specify how much distance a vehicle should maintain to
  /// the leading vehicle.
  void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance);

  /// Method to set % to run any traffic sign.
  void SetPercentageRunningSign(const ActorPtr &actor, const float perc);

  /// Method to set % to run any traffic light.
  void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

  /// Method to set % to ignore any vehicle.
  void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc);

  /// Method to set % to ignore any vehicle.
  void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc);

  /// Method to set % to keep on the right lane.
  void SetKeepRightPercentage(const ActorPtr &actor, const float percentage);

  /// Method to set % to randomly do a left lane change.
  void SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage);

  /// Method to set % to randomly do a right lane change.
  void SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage);

  /// Method to set the automatic vehicle light state update flag.
  void SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update);

  /// Method to set the distance to leading vehicle for all registered vehicles.
  void SetGlobalDistanceToLeadingVehicle(const float dist);

  /// Set Synchronous mode time out.
  void SetSynchronousModeTimeOutInMiliSecond(const double time);

  /// Method to set hybrid physics mode.
  void SetHybridPhysicsMode(const bool mode_switch);

  /// Method to set synchronous mode.
  void SetSynchronousMode(const bool mode_switch = true);

  /// Method to set hybrid physics radius.
  void SetHybridPhysicsRadius(const float radius);

  /// Method to set Open Street Map mode.
  void SetOSMMode(const bool mode_switch);

  /// Method to set if we are automatically respawning vehicles.
  void SetRespawnDormantVehicles(const bool mode_switch);

  /// Method to set boundaries for respawning vehicles.
  void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound);

  /// Method to set limits for boundaries when respawning vehicles.
  void SetMaxBoundaries(const float lower, const float upper);

  /// Method to set our own imported path.
  void SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer);

  /// Method to remove a list of points.
  void RemoveUploadPath(const ActorId &actor_id, const bool remove_path);

  /// Method to update an already set list of points.
  void UpdateUploadPath(const ActorId &actor_id, const Path path);

  /// Method to set our own imported route.
  void SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer);

  /// Method to remove a route.
  void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path);

  /// Method to update an already set route.
  void UpdateImportedRoute(const ActorId &actor_id, const Route route);

  ///////////////////////////////// GETTERS /////////////////////////////////////

  /// Method to retrieve hybrid physics radius.
  float GetHybridPhysicsRadius() const;

  /// Method to query target velocity for a vehicle.
  float GetVehicleTargetVelocity(const ActorId &actor_id, const float speed_limit) const;

  /// Method to query collision avoidance rule between a pair of vehicles.
  bool GetCollisionDetection(const ActorId &reference_actor_id, const ActorId &other_actor_id) const;

  /// Method to query lane change command for a vehicle.
  ChangeLaneInfo GetForceLaneChange(const ActorId &actor_id);

  /// Method to query percentage probability of keep right rule for a vehicle.
  float GetKeepRightPercentage(const ActorId &actor_id);

  /// Method to query percentage probability of a random right lane change for a vehicle.
  float GetRandomLeftLaneChangePercentage(const ActorId &actor_id);

  /// Method to query percentage probability of a random left lane change for a vehicle.
  float GetRandomRightLaneChangePercentage(const ActorId &actor_id);

  /// Method to query auto lane change rule for a vehicle.
  bool GetAutoLaneChange(const ActorId &actor_id) const;

  /// Method to query distance to leading vehicle for a given vehicle.
  float GetDistanceToLeadingVehicle(const ActorId &actor_id) const;

  /// Method to get % to run any traffic light.
  float GetPercentageRunningSign(const ActorId &actor_id) const;

  /// Method to get % to run any traffic light.
  float GetPercentageRunningLight(const ActorId &actor_id) const;

  /// Method to get % to ignore any vehicle.
  float GetPercentageIgnoreVehicles(const ActorId &actor_id) const;

  /// Method to get % to ignore any walker.
  float GetPercentageIgnoreWalkers(const ActorId &actor_id) const;

  /// Method to get if the vehicle lights should be updates automatically
  bool GetUpdateVehicleLights(const ActorId &actor_id) const;

  /// Method to get synchronous mode.
  bool GetSynchronousMode() const;

  /// Get synchronous mode time out
  double GetSynchronousModeTimeOutInMiliSecond() const;

  /// Method to retrieve hybrid physics mode.
  bool GetHybridPhysicsMode() const;

  /// Method to retrieve if we are automatically respawning vehicles.
  bool GetRespawnDormantVehicles() const;

  /// Method to retrieve minimum distance from hero vehicle when respawning vehicles.
  float GetLowerBoundaryRespawnDormantVehicles() const;

  /// Method to retrieve maximum distance from hero vehicle when respawning vehicles.
  float GetUpperBoundaryRespawnDormantVehicles() const;

  /// Method to get Open Street Map mode.
  bool GetOSMMode() const;

  /// Method to get if we are uploading a path.
  bool GetUploadPath(const ActorId &actor_id) const;

  /// Method to get a custom path.
  Path GetCustomPath(const ActorId &actor_id) const;

  /// Method to get if we are uploading a route.
  bool GetUploadRoute(const ActorId &actor_id) const;

  /// Method to get a custom route.
  Route GetImportedRoute(const ActorId &actor_id) const;

  /// Synchronous mode time out variable.
  std::chrono::duration<double, std::milli> synchronous_time_out;
};

} // namespace traffic_manager
} // namespace carla
