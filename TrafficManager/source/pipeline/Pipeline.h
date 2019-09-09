#pragma once

#include <vector>
#include <memory>
#include <random>

#include "carla/Memory.h"
#include "carla/client/Actor.h"
#include "carla/client/World.h"
#include "carla/client/Map.h"
#include "carla/client/BlueprintLibrary.h"

#include "InMemoryMap.h"
#include "LocalizationStage.h"
#include "CollisionStage.h"
#include "TrafficLightStage.h"
#include "MotionPlannerStage.h"
#include "BatchControlStage.h"

#define EXPECT_TRUE(pred) if (!(pred)) { throw std::runtime_error(# pred); }

namespace traffic_manager {

  int read_core_count();

  std::vector<carla::SharedPtr<carla::client::Actor>> spawn_traffic(
      carla::client::World &world,
      int core_count,
      int target_amount);

  class Pipeline {

  private:

    std::vector<float> longitudinal_PID_parameters;
    std::vector<float> longitudinal_highway_PID_parameters;
    std::vector<float> lateral_PID_parameters;
    float urban_target_velocity;
    float highway_target_velocity;
    int pipeline_width;

    std::vector<carla::SharedPtr<carla::client::Actor>>& actor_list;
    InMemoryMap& local_map;
    carla::client::DebugHelper& debug_helper;
    carla::client::Client& client_connection;

    std::shared_ptr<LocalizationToCollisionMessenger> localization_collision_messenger;
    std::shared_ptr<LocalizationToTrafficLightMessenger> localization_traffic_light_messenger;
    std::shared_ptr<CollisionToPlannerMessenger> collision_planner_messenger;
    std::shared_ptr<LocalizationToPlannerMessenger> localization_planner_messenger;
    std::shared_ptr<TrafficLightToPlannerMessenger> traffic_light_planner_messenger;
    std::shared_ptr<PlannerToControlMessenger> planner_control_messenger;

    std::unique_ptr<LocalizationStage> localization_stage;
    std::unique_ptr<CollisionStage> collision_stage;
    std::unique_ptr<TrafficLightStage> traffic_light_stage;
    std::unique_ptr<MotionPlannerStage> planner_stage;
    std::unique_ptr<BatchControlStage> control_stage;

  public:

    Pipeline(
        std::vector<float> longitudinal_PID_parameters,
        std::vector<float> longitudinal_highway_PID_parameters,
        std::vector<float> lateral_PID_parameters,
        float urban_target_velocity,
        float highway_target_velocity,
        std::vector<carla::SharedPtr<carla::client::Actor>>& actor_list,
        InMemoryMap& local_map,
        carla::client::Client& client_connection,
        carla::client::DebugHelper& debug_helper,
        int pipeline_width
      );

    void start();

    void stop();

  };

}
