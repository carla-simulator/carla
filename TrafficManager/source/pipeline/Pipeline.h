#pragma once

#include <algorithm>
#include <memory>
#include <vector>
#include <random>

#include "carla/client/Actor.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Map.h"
#include "carla/client/World.h"
#include "carla/geom/Transform.h"
#include "carla/Memory.h"
#include "carla/Logging.h"
#include "carla/rpc/ActorDescription.h"
#include "carla/rpc/Command.h"

#include "BatchControlStage.h"
#include "CollisionStage.h"
#include "InMemoryMap.h"
#include "LocalizationStage.h"
#include "MotionPlannerStage.h"
#include "TrafficLightStage.h"

#define EXPECT_TRUE(pred) if (!(pred)) { throw std::runtime_error(# pred); }

namespace traffic_manager {

  /// Function to read hardware concurrency
  int read_core_count();

  /// Function to spawn specified number of vehicles
  std::vector<carla::SharedPtr<carla::client::Actor>> spawn_traffic(
      carla::client::World &world,
      int core_count,
      int target_amount);

  /// Delete actors in a list
  void delete_traffic(
    std::vector<carla::SharedPtr<carla::client::Actor>>& actor_list,
    carla::client::World& world
  );

  /// The function of this class is to integrate all the various stages of
  /// the traffic manager appropriately using messengers
  class Pipeline {

  private:

    /// PID controller parameters
    std::vector<float> longitudinal_PID_parameters;
    std::vector<float> longitudinal_highway_PID_parameters;
    std::vector<float> lateral_PID_parameters;
    /// Number of worker threads per stage
    int pipeline_width;
    /// Target velocities
    float highway_target_velocity;
    float urban_target_velocity;
    /// Reference to list of all actors registered with traffic manager
    std::vector<carla::SharedPtr<carla::client::Actor>> &actor_list;
    /// Reference to local map cache
    InMemoryMap &local_map;
    /// Reference to carla's debug helper object
    carla::client::DebugHelper &debug_helper;
    /// Reference to carla's client connection object
    carla::client::Client &client_connection;
    /// Reference to carla's world object
    carla::client::World &world;
    /// Pointers to messenger objects connecting stage pairs
    std::shared_ptr<CollisionToPlannerMessenger> collision_planner_messenger;
    std::shared_ptr<LocalizationToCollisionMessenger> localization_collision_messenger;
    std::shared_ptr<LocalizationToTrafficLightMessenger> localization_traffic_light_messenger;
    std::shared_ptr<LocalizationToPlannerMessenger> localization_planner_messenger;
    std::shared_ptr<PlannerToControlMessenger> planner_control_messenger;
    std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_planner_messenger;
    /// Pointers to stage objects of traffic manager
    std::unique_ptr<CollisionStage> collision_stage;
    std::unique_ptr<BatchControlStage> control_stage;
    std::unique_ptr<LocalizationStage> localization_stage;
    std::unique_ptr<MotionPlannerStage> planner_stage;
    std::unique_ptr<TrafficLightStage> traffic_light_stage;

    

  public:

    Pipeline(
        std::vector<float> longitudinal_PID_parameters,
        std::vector<float> longitudinal_highway_PID_parameters,
        std::vector<float> lateral_PID_parameters,
        float urban_target_velocity,
        float highway_target_velocity,
        std::vector<carla::SharedPtr<carla::client::Actor>> &actor_list,
        InMemoryMap &local_map,
        carla::client::Client &client_connection,
        carla::client::World &world,
        carla::client::DebugHelper &debug_helper,
        int pipeline_width);

    void Start();

    void Stop();

  };

}
