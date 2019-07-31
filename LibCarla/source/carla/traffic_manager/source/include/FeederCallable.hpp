
#pragma once

#include <chrono>
#include <thread>

#include "carla/client/Actor.h"

#include "PipelineCallable.hpp"


namespace traffic_manager {

    class Feedercallable: public PipelineCallable {
        /*
        This class is the thread executable for the first stage of the pipeline.
        The class is responsible for feeding the pipeline indefinitely with
        the seed messages for every registered vehicle in a cyclic order.

        The class is also responsible for adjusting the speed of the pipeline to
        scale according to the number of vehicles registered with the pipeline.

        Note: Exactly one instance of this thread should be run in the stage.
        This is because the action method loops indefinitely throughout the lifetime
        of the pipeline.
        */

        public:

        Feedercallable(
            SyncQueue<PipelineMessage>* input_queue,
            SyncQueue<PipelineMessage>* output_queue,
            SharedData* shared_data);
        ~Feedercallable();

        PipelineMessage action (PipelineMessage message);
    };
}
