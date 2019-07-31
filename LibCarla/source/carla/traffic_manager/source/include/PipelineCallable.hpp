
#pragma once

#include "SyncQueue.hpp"
#include "PipelineMessage.hpp"
#include "SharedData.hpp"

namespace traffic_manager {

    class PipelineCallable {
        /*
        This is the base class for thread executables to be run in pipeline stages.
        The class provides the base infrastructure to manage communication between
        previous and next stages as well providing a method to override where the stage
        specific logic should be placed.
        */

        private:

        SyncQueue<PipelineMessage>* const input_queue;
        SyncQueue<PipelineMessage>* const output_queue;

        protected:

        SharedData* const shared_data;

        /* This method reads and returns a message object from the previous stage. */
        PipelineMessage readQueue();

        /* This method writes a message to the output stage of the stage. */
        void writeQueue(PipelineMessage);

        /*
        This method should be overridden in the child class to provide stage specific
        logic. The method takes a message from the previous stage, performs it's logic
        and returns an output message to be passed on to the next stage.
        */
        virtual PipelineMessage action(PipelineMessage message)=0;

        public:

        PipelineCallable(
            SyncQueue<PipelineMessage>* input_queue,
            SyncQueue<PipelineMessage>* output_queue,
            SharedData* shared_data);
        virtual ~PipelineCallable();

        /*
        This is the method executed by threads of corresponding pipeline stage.
        This method has the logic to co-ordinate reading message from previous stage,
        passing it to action() method, and writing the output message to the output queue.
        */
        void run();
    };
}
