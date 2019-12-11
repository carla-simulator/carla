#include "carla/trafficmanager/PerformanceDiagnostics.h"

namespace traffic_manager {

    PerformanceDiagnostics::PerformanceDiagnostics(std::string stage_name)
    : stage_name(stage_name) {

        throughput_clock = chr::system_clock::now();
        throughput_counter = 0;
        inter_update_clock = chr::system_clock::now();
        inter_update_duration = chr::duration<float>(0);
    }

    void PerformanceDiagnostics::RegisterUpdate() {

        auto current_time = chr::system_clock::now();

        chr::duration<float> throughput_count_duration = current_time - throughput_clock;
        if (throughput_count_duration.count() > 1.0f) {

            /* std::cout << (
                "Stage name : " + stage_name
                + ", throughput : "
                + std::to_string(throughput_counter) + " fps"
                + ", average update duration : "
                + std::to_string(inter_update_duration.count() * 1000.0f) + " ms"
                + "\n"
            ); */

            throughput_clock = current_time;
            throughput_counter = 0;
        } else {

            ++throughput_counter;

            chr::duration<float> last_update_duration = current_time - inter_update_clock;
            inter_update_duration = (inter_update_duration + last_update_duration) / 2.0f;
            inter_update_clock = current_time;
        }
    }
}