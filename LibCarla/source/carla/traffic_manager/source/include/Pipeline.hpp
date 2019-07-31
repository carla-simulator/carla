
#pragma once

#include <vector>
#include <memory>

#include "carla/Memory.h"
#include "carla/client/Actor.h"
#include "carla/client/Map.h"
#include "carla/client/Client.h"

#include "CarlaDataAccessLayer.hpp"
#include "InMemoryMap.hpp"
#include "SyncQueue.hpp"
#include "FeederCallable.hpp"
#include "LocalizationCallable.hpp"
#include "CollisionCallable.hpp"
#include "TrafficLightStateCallable.hpp"
#include "MotionPlannerCallable.hpp"
#include "BatchControlCallable.hpp"
#include "PipelineStage.hpp"

namespace traffic_manager {

    class Pipeline {

        private:

        int NUMBER_OF_STAGES = 6;

        std::vector<carla::SharedPtr<carla::client::Actor>> actor_list;
        carla::SharedPtr<carla::client::Map> world_map;
        float target_velocity;
        std::vector<float> longitudinal_PID_parameters;
        std::vector<float> lateral_PID_parameters;
        int pipeline_width;
        carla::client::Client& client;
        carla::client::DebugHelper debug_helper;

        traffic_manager::SharedData shared_data;
        std::vector<std::shared_ptr<SyncQueue<PipelineMessage>>> message_queues;
        std::vector<std::shared_ptr<PipelineCallable>> callables;
        std::vector<std::shared_ptr<PipelineStage>> stages;

        public:

        Pipeline(
            std::vector<carla::SharedPtr<carla::client::Actor>> actor_list,
            carla::SharedPtr<carla::client::Map> world_map,
            float target_velocity,
            std::vector<float> longitudinal_PID_parameters,
            std::vector<float> lateral_PID_parameters,
            int pipeline_width,
            carla::client::Client& client,
            carla::client::DebugHelper debug_helper
        );

        void setup();
        void start();
    };
}
