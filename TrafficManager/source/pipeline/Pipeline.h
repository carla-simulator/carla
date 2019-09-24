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
#include "carla/rpc/Command.h"

#include "BatchControlStage.h"
#include "CollisionStage.h"
#include "InMemoryMap.h"
#include "LocalizationStage.h"
#include "MotionPlannerStage.h"
#include "TrafficLightStage.h"

#define EXPECT_TRUE(pred) if (!(pred)) { throw std::runtime_error(# pred); }

namespace traffic_manager {

namespace cc = carla::client;
namespace cr = carla::rpc;
  using ActorPtr = carla::SharedPtr<cc::Actor>;

  /// Function to read hardware concurrency
  uint read_core_count();

  /// Function to spawn specified number of vehicles
  std::vector<ActorPtr> spawn_traffic(
      cc::Client &client,
      cc::World &world,
      uint core_count,
      uint target_amount);

  /// Detroy actors
  void destroy_traffic(
      std::vector<ActorPtr> &actor_list,
      cc::Client &client);

  /// The function of this class is to integrate all the various stages of
  /// the traffic manager appropriately using messengers
  class Pipeline {

  private:

    /// PID controller parameters
    std::vector<float> longitudinal_PID_parameters;
    std::vector<float> longitudinal_highway_PID_parameters;
    std::vector<float> lateral_PID_parameters;
    /// Number of worker threads per stage
    uint pipeline_width;
    /// Target velocities
    float highway_target_velocity;
    float urban_target_velocity;
    /// Reference to list of all actors registered with traffic manager
    std::vector<ActorPtr> &actor_list;
    /// Reference to local map cache
    InMemoryMap &local_map;
    /// Reference to carla's debug helper object
    cc::DebugHelper &debug_helper;
    /// Reference to carla's client connection object
    cc::Client &client_connection;
    /// Reference to carla's world object
    cc::World &world;
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
        std::vector<ActorPtr> &actor_list,
        InMemoryMap &local_map,
        cc::Client &client_connection,
        cc::World &world,
        cc::DebugHelper &debug_helper,
        uint pipeline_width);

    void Start();

    void Stop();

  };

}
