// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <chrono>

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
    /// Synchronous mode switch.
    std::atomic<bool> synchronous_mode;
    /// Distance Margin
    std::atomic<float> distance_margin {2.0};

  public:
    Parameters();
    ~Parameters();

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

    /// Method to query target velocity for a vehicle.
    float GetVehicleTargetVelocity(const ActorPtr &actor);

    /// Method to query collision avoidance rule between a pair of vehicles.
    bool GetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor);

    /// Method to query lane change command for a vehicle.
    ChangeLaneInfo GetForceLaneChange(const ActorPtr &actor);

    /// Method to query percentage probability of keep right rule for a vehicle.
    float GetKeepRightPercentage(const ActorPtr &actor);

    /// Method to query auto lane change rule for a vehicle.
    bool GetAutoLaneChange(const ActorPtr &actor);

    /// Method to query distance to leading vehicle for a given vehicle.
    float GetDistanceToLeadingVehicle(const ActorPtr &actor);

    /// Method to get % to run any traffic light.
    float GetPercentageRunningSign(const ActorPtr &actor);

    /// Method to get % to run any traffic light.
    float GetPercentageRunningLight(const ActorPtr &actor);

    /// Method to get % to ignore any vehicle.
    float GetPercentageIgnoreVehicles(const ActorPtr &actor);

    /// Method to get % to ignore any walker.
    float GetPercentageIgnoreWalkers(const ActorPtr &actor);

     /// Method to set distance to leading vehicle for a given vehicle.
    void SetGlobalDistanceToLeadingVehicle(const float dist);

    /// Method to set % to run any traffic sign.
    void SetPercentageRunningSign(const ActorPtr &actor, const float perc);

    /// Method to set % to run any traffic light.
    void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

    /// Method to set % to ignore any vehicle.
    void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc);

    /// Method to set % to ignore any vehicle.
    void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc);

    /// Method to set probabilistic preference to keep on the right lane.
    void SetKeepRightPercentage(const ActorPtr &actor,const float percentage);

    /// Method to get synchronous mode.
    bool GetSynchronousMode();

    /// Method to set synchronous mode.
    void SetSynchronousMode(const bool mode_switch = true);

    /// Get synchronous mode time out
    double GetSynchronousModeTimeOutInMiliSecond();

    /// Set Synchronous mode time out.
    void SetSynchronousModeTimeOutInMiliSecond(const double time);

    /// Synchronous mode time out variable.
    std::chrono::duration<double, std::milli> synchronous_time_out;

  };

} // namespace traffic_manager
} // namespace carla
