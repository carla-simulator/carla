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

#include "AtomicActorSet.h"
#include "BatchControlStage.h"
#include "CarlaDataAccessLayer.h"
#include "CollisionStage.h"
#include "InMemoryMap.h"
#include "LocalizationStage.h"
#include "MotionPlannerStage.h"
#include "TrafficLightStage.h"

namespace traffic_manager {

namespace cc = carla::client;

  using ActorPtr = carla::SharedPtr<cc::Actor>;

  /// The function of this class is to integrate all the various stages of
  /// the traffic manager appropriately using messengers.
  class Pipeline {

  private:

    /// PID controller parameters.
    std::vector<float> longitudinal_PID_parameters;
    std::vector<float> longitudinal_highway_PID_parameters;
    std::vector<float> lateral_PID_parameters;
    /// Target velocities.
    float highway_target_velocity;
    float urban_target_velocity;
    /// Set of all actors registered with traffic manager.
    AtomicActorSet registered_actors;
    /// Pointer to local map cache.
    std::shared_ptr<InMemoryMap> local_map;
    /// Carla's debug helper object.
    cc::DebugHelper debug_helper;
    /// Carla's client connection object.
    cc::Client client_connection;
    /// Carla's world object.
    cc::World world;
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

  public:

    Pipeline(std::vector<float> longitudinal_PID_parameters,
             std::vector<float> longitudinal_highway_PID_parameters,
             std::vector<float> lateral_PID_parameters,
             float urban_target_velocity,
             float highway_target_velocity,
             cc::Client &client_connection);

    /// This method registers a vehicle with the traffic manager.
    void RegisterVehicles(std::vector<ActorPtr> actor_list);

    /// This method unregisters a vehicle from traffic manager.
    void UnregisterVehicles(std::vector<ActorPtr> actor_list);

    /// To start the pipeline.
    void Start();

    /// To stop the pipeline.
    void Stop();

  };

}
