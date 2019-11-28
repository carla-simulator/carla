#pragma once

#include <chrono>
#include <string>

#include "carla/Logging.h"

namespace traffic_manager {

namespace chr = std::chrono;

    class PerformanceDiagnostics {

    private:
        /// Stage name.
        std::string stage_name;
        /// Throughput clock.
        chr::time_point<chr::_V2::system_clock, chr::nanoseconds> throughput_clock;
        /// Throughput counter.
        uint throughput_counter;
        /// Inter-update clock.
        chr::time_point<chr::_V2::system_clock, chr::nanoseconds> inter_update_clock;
        /// Inter-update duration.
        chr::duration<float> inter_update_duration;

    public:
        PerformanceDiagnostics(std::string name);

        void RegisterUpdate();
    };
}
