#pragma once

#include <algorithm>
#include <memory>
#include <vector>
#include <random>

#include "carla/client/Actor.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Map.h"
#include "carla/client/World.h"
#include "carla/Memory.h"
#include "carla/Logging.h"

#include "BatchControlStage.h"
#include "CollisionStage.h"
#include "InMemoryMap.h"
#include "LocalizationStage.h"
#include "MotionPlannerStage.h"
#include "TrafficLightStage.h"

#define EXPECT_TRUE(pred) if (!(pred)) { throw std::runtime_error(# pred); }

namespace traffic_manager {

  int read_core_count();

  std::vector<carla::SharedPtr<carla::client::Actor>> spawn_traffic(
      carla::client::World &world,
      int core_count,
      int target_amount);

  /// Work pipeline of traffic manager
  class Pipeline {

  private:

    std::vector<float> longitudinal_PID_parameters;
    std::vector<float> longitudinal_highway_PID_parameters;
    std::vector<float> lateral_PID_parameters;
    int pipeline_width;
    float highway_target_velocity;
    float urban_target_velocity;
    

    std::vector<carla::SharedPtr<carla::client::Actor>> &actor_list;
    InMemoryMap &local_map;
    carla::client::DebugHelper &debug_helper;
    carla::client::Client &client_connection;
    carla::client::World &world;

    std::shared_ptr<CollisionToPlannerMessenger> collision_planner_messenger;
    std::shared_ptr<LocalizationToCollisionMessenger> localization_collision_messenger;
    std::shared_ptr<LocalizationToTrafficLightMessenger> localization_traffic_light_messenger;
    std::shared_ptr<LocalizationToPlannerMessenger> localization_planner_messenger;
    std::shared_ptr<PlannerToControlMessenger> planner_control_messenger;
    std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_planner_messenger;

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
