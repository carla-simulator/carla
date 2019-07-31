
#pragma once

#include <chrono>
#include <cmath>
#include <vector>

#include "PipelineCallable.hpp"
#include "SharedData.hpp"
#include "PIDController.hpp"


namespace traffic_manager {

    class MotionPlannerCallable: public PipelineCallable {
        /*
        This class is the thread executable for the motion planner stage.
        The class is responsible for aggregating information from various stages
        like traffic like state stage, collision detection stage and actuation signals
        from PID controller and makes decisions on how to move the vehicle to follow
        it's trajectory safely.
        */

        private:

        std::vector<float> longitudinal_parameters;
        std::vector<float> lateral_parameters;
        float target_velocity;
        SharedData* shared_data;
        PIDController controller;
            
        public:

        MotionPlannerCallable(
            float target_velocity,
            SyncQueue<PipelineMessage>* input_queue,
            SyncQueue<PipelineMessage>* output_queue,
            SharedData* shared_data,
            std::vector<float> longitudinal_parameters,
            std::vector<float> lateral_parameters
        );
        ~MotionPlannerCallable();
        PipelineMessage action(PipelineMessage message);
    };
}
