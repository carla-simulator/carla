// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include "carla/client/Actor.h"
#include "carla/trafficmanager/SimpleWaypoint.h"

namespace carla {
namespace traffic_manager {

using ActorPtr = carla::SharedPtr<carla::client::Actor>;
using Path = std::vector<cg::Location>;
using Route = std::vector<uint8_t>;
using WaypointPtr = carla::SharedPtr<carla::client::Waypoint>;
using Action = std::pair<RoadOption, WaypointPtr>;
using ActionBuffer = std::vector<Action>;


/// The function of this class is to integrate all the various stages of
/// the traffic manager appropriately using messengers.
class TrafficManagerBase {

public:
  /// To start the traffic manager.
  virtual void Start() = 0;

  /// To stop the traffic manager.
  virtual void Stop() = 0;

  /// To release the traffic manager.
  virtual void Release() = 0;

  /// To reset the traffic manager.
  virtual void Reset() = 0;

  /// Protected constructor for singleton lifecycle management.
  TrafficManagerBase() {};

  /// Destructor.
  virtual ~TrafficManagerBase() {};

  /// This method registers a vehicle with the traffic manager.
  virtual void RegisterVehicles(const std::vector<ActorPtr> &actor_list) = 0;

  /// This method unregisters a vehicle from traffic manager.
  virtual void UnregisterVehicles(const std::vector<ActorPtr> &actor_list) = 0;

  /// Set a vehicle's % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  virtual void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage) = 0;

  /// Method to set a lane offset displacement from the center line.
  /// Positive values imply a right offset while negative ones mean a left one.
  virtual void SetLaneOffset(const ActorPtr &actor, const float offset) = 0;

  /// Set a vehicle's exact desired velocity.
  virtual void SetDesiredSpeed(const ActorPtr &actor, const float value) = 0;

  /// Set a global % decrease in velocity with respect to the speed limit.
  /// If less than 0, it's a % increase.
  virtual void SetGlobalPercentageSpeedDifference(float const percentage) = 0;

  /// Method to set a global lane offset displacement from the center line.
  /// Positive values imply a right offset while negative ones mean a left one.
  virtual void SetGlobalLaneOffset(float const offset) = 0;

  /// Method to set the automatic management of the vehicle lights
  virtual void SetUpdateVehicleLights(const ActorPtr &actor, const bool do_update) = 0;

  /// Method to set collision detection rules between vehicles.
  virtual void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision) = 0;

  /// Method to force lane change on a vehicle.
  /// Direction flag can be set to true for left and false for right.
  virtual void SetForceLaneChange(const ActorPtr &actor, const bool direction) = 0;

  /// Enable/disable automatic lane change on a vehicle.
  virtual void SetAutoLaneChange(const ActorPtr &actor, const bool enable) = 0;

  /// Method to specify how much distance a vehicle should maintain to
  /// the leading vehicle.
  virtual void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance) = 0;

  /// Method to specify the % chance of ignoring collisions with any walker.
  virtual void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc) = 0;

  /// Method to specify the % chance of ignoring collisions with any vehicle.
  virtual void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc) = 0;

  /// Method to specify the % chance of running any traffic light.
  virtual void SetPercentageRunningLight(const ActorPtr &actor, const float perc) = 0;

  /// Method to specify the % chance of running any traffic sign.
  virtual void SetPercentageRunningSign(const ActorPtr &actor, const float perc) = 0;

  /// Method to switch traffic manager into synchronous execution.
  virtual void SetSynchronousMode(bool mode) = 0;

  /// Method to set Tick timeout for synchronous execution.
  virtual void SetSynchronousModeTimeOutInMiliSecond(double time) = 0;

  /// Method to provide synchronous tick
  virtual bool SynchronousTick() = 0;

  /// Get carla episode information
  virtual  carla::client::detail::EpisodeProxy& GetEpisodeProxy() = 0;

  /// Method to set Global Distance to Leading Vehicle.
  virtual void SetGlobalDistanceToLeadingVehicle(const float dist) = 0;

  /// Method to set % to keep on the right lane.
  virtual void SetKeepRightPercentage(const ActorPtr &actor,const float percentage) = 0;

  /// Method to set % to randomly do a left lane change.
  virtual void SetRandomLeftLaneChangePercentage(const ActorPtr &actor, const float percentage) = 0;

  /// Method to set % to randomly do a right lane change.
  virtual void SetRandomRightLaneChangePercentage(const ActorPtr &actor, const float percentage) = 0;

  /// Method to set hybrid physics mode.
  virtual void SetHybridPhysicsMode(const bool mode_switch) = 0;

  /// Method to set hybrid physics radius.
  virtual void SetHybridPhysicsRadius(const float radius) = 0;

  /// Method to set randomization seed.
  virtual void SetRandomDeviceSeed(const uint64_t seed) = 0;

  /// Method to set Open Street Map mode.
  virtual void SetOSMMode(const bool mode_switch) = 0;

  /// Method to set our own imported path.
  virtual void SetCustomPath(const ActorPtr &actor, const Path path, const bool empty_buffer) = 0;

  /// Method to remove a path.
  virtual void RemoveUploadPath(const ActorId &actor_id, const bool remove_path) = 0;

  /// Method to update an already set path.
  virtual void UpdateUploadPath(const ActorId &actor_id, const Path path) = 0;

  /// Method to set our own imported route.
  virtual void SetImportedRoute(const ActorPtr &actor, const Route route, const bool empty_buffer) = 0;

  /// Method to remove a route.
  virtual void RemoveImportedRoute(const ActorId &actor_id, const bool remove_path) = 0;

  /// Method to update an already set route.
  virtual void UpdateImportedRoute(const ActorId &actor_id, const Route route) = 0;

  /// Method to set automatic respawn of dormant vehicles.
  virtual void SetRespawnDormantVehicles(const bool mode_switch) = 0;

  /// Method to set boundaries for respawning vehicles.
  virtual void SetBoundariesRespawnDormantVehicles(const float lower_bound, const float upper_bound) = 0;

  /// Method to set limits for boundaries when respawning vehicles.
  virtual void SetMaxBoundaries(const float lower, const float upper) = 0;

  /// Method to get the vehicle's next action.
  virtual Action GetNextAction(const ActorId &actor_id) = 0;

  /// Method to get the vehicle's action buffer.
  virtual ActionBuffer GetActionBuffer(const ActorId &actor_id) = 0;

  virtual void ShutDown() = 0;

protected:

};

} // namespace traffic_manager
} // namespace carla
