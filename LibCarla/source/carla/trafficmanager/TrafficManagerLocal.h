// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <vector>

#include "carla/client/detail/Simulator.h"
#include "carla/client/detail/EpisodeProxy.h"

#include "carla/trafficmanager/AtomicActorSet.h"
#include "carla/trafficmanager/BatchControlStage.h"
#include "carla/trafficmanager/CollisionStage.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/LocalizationStage.h"
#include "carla/trafficmanager/MotionPlannerStage.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/TrafficLightStage.h"

#include "carla/trafficmanager/TrafficManagerBase.h"
#include "carla/trafficmanager/TrafficManagerServer.h"

namespace carla {
namespace traffic_manager {

  using ActorPtr = carla::SharedPtr<carla::client::Actor>;
  using TLGroup = std::vector<carla::SharedPtr<carla::client::TrafficLight>>;

  /// The function of this class is to integrate all the various stages of
  /// the traffic manager appropriately using messengers.
  class TrafficManagerLocal : public TrafficManagerBase {

  private:

    /// PID controller parameters.
    std::vector<float> longitudinal_PID_parameters;
    std::vector<float> longitudinal_highway_PID_parameters;
    std::vector<float> lateral_PID_parameters;
    std::vector<float> lateral_highway_PID_parameters;

    /// Set of all actors registered with traffic manager.
    AtomicActorSet registered_actors;

    /// Pointer to local map cache.
    std::shared_ptr<InMemoryMap> local_map;

    /// Carla's client connection object.
    carla::client::detail::EpisodeProxy episodeProxyTM;

    /// Carla's debug helper object.
    carla::client::DebugHelper debug_helper;

    /// Pointers to messenger objects connecting stage pairs.
    std::shared_ptr<CollisionToPlannerMessenger> collision_planner_messenger;
    std::shared_ptr<LocalizationToCollisionMessenger> localization_collision_messenger;
    std::shared_ptr<LocalizationToTrafficLightMessenger> localization_traffic_light_messenger;
    std::shared_ptr<LocalizationToPlannerMessenger> localization_planner_messenger;
    std::shared_ptr<PlannerToControlMessenger> planner_control_messenger;
    std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_planner_messenger;

    /// Pointers to the stage objects of traffic manager.
    std::unique_ptr<CollisionStage> collision_stage;
    std::unique_ptr<BatchControlStage> control_stage;
    std::unique_ptr<LocalizationStage> localization_stage;
    std::unique_ptr<MotionPlannerStage> planner_stage;
    std::unique_ptr<TrafficLightStage> traffic_light_stage;

    /// Parameterization object.
    Parameters parameters;

    /// Traffic manager server instance.
    TrafficManagerServer server;

    /// Method to check if all traffic lights are frozen in a group.
    bool CheckAllFrozen(TLGroup tl_to_freeze);

  public:

    /// Private constructor for singleton lifecycle management.
    TrafficManagerLocal(
      std::vector<float> longitudinal_PID_parameters,
      std::vector<float> longitudinal_highway_PID_parameters,
      std::vector<float> lateral_PID_parameters,
      std::vector<float> lateral_highway_PID_parameters,
      float perc_decrease_from_limit,
      carla::client::detail::EpisodeProxy &episodeProxy,
      uint16_t &RPCportTM);

    /// Destructor.
    virtual ~TrafficManagerLocal();

    /// To start the TrafficManager.
    void Start();

    /// To stop the TrafficManager.
    void Stop();

    /// To release the traffic manager.
    void Release();

    /// To reset the traffic manager.
    void Reset();

    /// This method registers a vehicle with the traffic manager.
    void RegisterVehicles(const std::vector<ActorPtr> &actor_list);

    /// This method unregisters a vehicle from traffic manager.
    void UnregisterVehicles(const std::vector<ActorPtr> &actor_list);

    /// Method to set a vehicle's % decrease in velocity with respect to the speed limit.
    /// If less than 0, it's a % increase.
    void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage);

    /// Methos to set a global % decrease in velocity with respect to the speed limit.
    /// If less than 0, it's a % increase.
    void SetGlobalPercentageSpeedDifference(float const percentage);

    /// Method to set collision detection rules between vehicles.
    void SetCollisionDetection(const ActorPtr &reference_actor, const ActorPtr &other_actor, const bool detect_collision);

    /// Method to force lane change on a vehicle.
    /// Direction flag can be set to true for left and false for right.
    void SetForceLaneChange(const ActorPtr &actor, const bool direction);

    /// Enable/disable automatic lane change on a vehicle.
    void SetAutoLaneChange(const ActorPtr &actor, const bool enable);

    /// Method to specify how much distance a vehicle should maintain to
    /// the leading vehicle.
    void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance);

    /// Method to specify the % chance of ignoring collisions with any walker.
    void SetPercentageIgnoreWalkers(const ActorPtr &actor, const float perc);

    /// Method to specify the % chance of ignoring collisions with any vehicle.
    void SetPercentageIgnoreVehicles(const ActorPtr &actor, const float perc);

    /// Method to specify the % chance of running any traffic light.
    void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

    /// Method to specify the % chance of running any traffic sign.
    void SetPercentageRunningSign(const ActorPtr &actor, const float perc);

    /// Method to switch traffic manager into synchronous execution.
    void SetSynchronousMode(bool mode);

    /// Method to set Tick timeout for synchronous execution.
    void SetSynchronousModeTimeOutInMiliSecond(double time);

    /// Method to provide synchronous tick.
    bool SynchronousTick();

    /// Method to reset all traffic light groups to the initial stage.
    void ResetAllTrafficLights();

    /// Get CARLA episode information.
    carla::client::detail::EpisodeProxy& GetEpisodeProxy();

    /// Get list of all registered vehicles.
    std::vector<ActorId> GetRegisteredVehiclesIDs();

    /// Method to specify how much distance a vehicle should maintain to
    /// the Global leading vehicle.
    void SetGlobalDistanceToLeadingVehicle(const float distance);

    /// Method to set probabilistic preference to keep on the right lane.
    void SetKeepRightPercentage(const ActorPtr &actor, const float percentage);

    /// Method to set hybrid physics mode.
    void SetHybridPhysicsMode(const bool mode_switch);

    /// Method to set hybrid physics radius.
    void SetHybridPhysicsRadius(const float radius);

  };

} // namespace traffic_manager
} // namespace carla
