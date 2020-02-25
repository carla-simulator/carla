// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <algorithm>
#include <memory>
#include <random>
#include <unordered_set>
#include <vector>

#include "carla/client/Actor.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Map.h"
#include "carla/client/World.h"
#include "carla/geom/Transform.h"
#include "carla/Logging.h"
#include "carla/Memory.h"

#include "carla/trafficmanager/AtomicActorSet.h"
#include "carla/trafficmanager/AtomicMap.h"
#include "carla/trafficmanager/BatchControlStage.h"
#include "carla/trafficmanager/CollisionStage.h"
#include "carla/trafficmanager/InMemoryMap.h"
#include "carla/trafficmanager/LocalizationStage.h"
#include "carla/trafficmanager/MotionPlannerStage.h"
#include "carla/trafficmanager/Parameters.h"
#include "carla/trafficmanager/TrafficLightStage.h"

namespace carla {
namespace traffic_manager {

  namespace cc = carla::client;

  using ActorPtr = carla::SharedPtr<cc::Actor>;
  using TLS = carla::rpc::TrafficLightState;
  using TLGroup = std::vector<carla::SharedPtr<cc::TrafficLight>>;

  /// The function of this class is to integrate all the various stages of
  /// the traffic manager appropriately using messengers.
  class TrafficManager {

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
    cc::Client client_connection;
    /// Carla's world object.
    cc::World world;
    /// Carla's debug helper object.
    cc::DebugHelper debug_helper;
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
    /// Static pointer to singleton object.
    static std::unique_ptr<TrafficManager> singleton_pointer;
    /// Static pointer to singleton client connected to localhost, 2000.
    static std::unique_ptr<cc::Client> singleton_local_client;
    /// Parameterization object.
    Parameters parameters;

    /// Private constructor for singleton lifecycle management.
    TrafficManager(
        std::vector<float> longitudinal_PID_parameters,
        std::vector<float> longitudinal_highway_PID_parameters,
        std::vector<float> lateral_PID_parameters,
        std::vector<float> lateral_highway_PID_parameters,
        float perc_decrease_from_limit,
        cc::Client &client_connection);

    /// To start the TrafficManager.
    void Start();

    /// To stop the TrafficManager.
    void Stop();

  public:

    /// Static method for singleton lifecycle management.
    static TrafficManager& GetInstance(cc::Client &client_connection);

    /// Static method to get unique client connected to (localhost, 2000).
    static cc::Client& GetUniqueLocalClient();

    /// This method registers a vehicle with the traffic manager.
    void RegisterVehicles(const std::vector<ActorPtr> &actor_list);

    /// This method unregisters a vehicle from traffic manager.
    void UnregisterVehicles(const std::vector<ActorPtr> &actor_list);

    /// Set target velocity specific to a vehicle.
    void SetPercentageSpeedDifference(const ActorPtr &actor, const float percentage);

    /// Set global target velocity.
    void SetGlobalPercentageSpeedDifference(float const percentage);

    /// Set collision detection rules between vehicles.
    void SetCollisionDetection(
        const ActorPtr &reference_actor,
        const ActorPtr &other_actor,
        const bool detect_collision);

    /// Method to force lane change on a vehicle.
    /// Direction flag can be set to true for left and false for right.
    void SetForceLaneChange(const ActorPtr &actor, const bool direction);

    /// Enable / disable automatic lane change on a vehicle.
    void SetAutoLaneChange(const ActorPtr &actor, const bool enable);

    /// Method to specify how much distance a vehicle should maintain to
    /// the leading vehicle.
    void SetDistanceToLeadingVehicle(const ActorPtr &actor, const float distance);

    /// Method to specify the % chance of ignoring collisions with other actors
    void SetPercentageIgnoreActors(const ActorPtr &actor, const float perc);

    /// Method to specify the % chance of running a red light
    void SetPercentageRunningLight(const ActorPtr &actor, const float perc);

    /// Method to check if traffic lights are frozen.
    bool CheckAllFrozen(TLGroup tl_to_freeze);

    /// Method to reset all traffic lights.
    void ResetAllTrafficLights();

    /// Return the world object
    const cc::World &GetWorld() { return world; };

    /// Destructor.
    ~TrafficManager();

  };

} // namespace traffic_manager
} // namespace carla
