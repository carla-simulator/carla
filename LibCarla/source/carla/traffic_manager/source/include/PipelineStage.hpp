
#pragma once

#include <thread>

#include "PipelineCallable.hpp"


namespace traffic_manager {

    class PipelineStage {
        /*
        This is the class for stage objects in the pipeline.
        Construct objects of this type by passing the appropriate callable object
        and threading information to create the pipeline stage.
        */

        private:

        std::vector<std::thread> threads;

        /* This method runs the threads of the stage's callable. */
        void runThreads();

        protected:

        const int pool_size;
        PipelineCallable& thread_callable;

        public:

        /* pass the number of threads and the callable object to execute. */
        PipelineStage(
            int pool_size,
            PipelineCallable& thread_callable);
        virtual ~PipelineStage();

        /* This method starts the threads for the stage. */
        void start();
    };

}