
#pragma once

#include "carla/client/Vehicle.h"
#include "carla/rpc/TrafficLightState.h"

#include "PipelineCallable.hpp"


namespace traffic_manager {

    class TrafficLightStateCallable: public PipelineCallable {
        /*
        This class is the thread executable for the traffic light stage.
        The class is responsible to detect what is state of the traffic light
        affecting the vehicle in the message.
        */

        public:

        TrafficLightStateCallable(
            SyncQueue<PipelineMessage>* input_queue,
            SyncQueue<PipelineMessage>* output_queue,
            SharedData* shared_data);
        ~TrafficLightStateCallable();

        PipelineMessage action(PipelineMessage message);
    };

}
