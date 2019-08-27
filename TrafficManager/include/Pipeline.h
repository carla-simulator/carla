#pragma once

#include <vector>
#include <memory>
#include <random>

#include "carla/Memory.h"
#include "carla/client/Actor.h"
#include "carla/client/World.h"
#include "carla/client/Map.h"
#include "carla/client/BlueprintLibrary.h"

#include "SyncQueue.h"
#include "FeederCallable.h"
#include "LocalizationCallable.h"
#include "CollisionCallable.h"
#include "TrafficLightStateCallable.h"
#include "MotionPlannerCallable.h"
#include "BatchControlCallable.h"
#include "PipelineStage.h"

#define EXPECT_TRUE(pred) if (!(pred)) { throw std::runtime_error(# pred); }

namespace traffic_manager {

  int read_core_count();

  std::vector<carla::SharedPtr<carla::client::Actor>> spawn_traffic(
      carla::client::World &world,
      int core_count,
      int target_amount);

  class Pipeline {

  private:

    int NUMBER_OF_STAGES = 6;

    std::vector<float> longitudinal_PID_parameters;
    std::vector<float> lateral_PID_parameters;
    float target_velocity;
    int pipeline_width;

    traffic_manager::SharedData &shared_data;
    std::vector<std::shared_ptr<SyncQueue<PipelineMessage>>> message_queues;
    std::vector<std::shared_ptr<PipelineCallable>> callables;
    std::vector<std::shared_ptr<PipelineStage>> stages;

  public:

    Pipeline(
        std::vector<float> longitudinal_PID_parameters,
        std::vector<float> lateral_PID_parameters,
        float target_velocity,
        int pipeline_width,
        SharedData &shared_data);

    void setup();

    void start();

    void stop();

  };

}
