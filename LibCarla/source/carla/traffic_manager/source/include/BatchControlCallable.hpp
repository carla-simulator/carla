
#pragma once

#include <chrono>

#include "carla/rpc/Command.h"
#include "carla/rpc/VehicleControl.h"

#include "PipelineCallable.hpp"
#include "SharedData.hpp"


namespace traffic_manager {
    class BatchControlCallable: public PipelineCallable {
        /*
        This class is the thread executable for the last stage in the pipeline.
        This class is tasked with managing communicating actuation signals to
        the simulator in batches.

        Note: Exactly one thread executables of this class should be instantiated
        in the stage. This is because this executable runs in an infinite loop
        throughout the lifetime of the pipeline.
        */

        private:

        SyncQueue<PipelineMessage>* input_queue;

        public:

        BatchControlCallable(
            SyncQueue<PipelineMessage>* input_queue,
            SyncQueue<PipelineMessage>* output_queue,
            SharedData* shared_data
        );
        ~BatchControlCallable();
        PipelineMessage action(PipelineMessage message);       
    };
}