// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/PerformanceDiagnostics.h"

namespace carla {
namespace traffic_manager {

  PerformanceDiagnostics::PerformanceDiagnostics(std::string stage_name)
  : stage_name(stage_name) {

    throughput_clock = chr::system_clock::now();
    throughput_counter = 0u;
    inter_update_clock = chr::system_clock::now();
    inter_update_duration = chr::duration<float>(0);
  }

  void PerformanceDiagnostics::RegisterUpdate() {

    const auto current_time = chr::system_clock::now();

    const chr::duration<float> throughput_count_duration = current_time - throughput_clock;
    if (throughput_count_duration.count() > 1.0f) {

      throughput_clock = current_time;
      throughput_counter = 0u;
    } else {

      ++throughput_counter;

      const chr::duration<float> last_update_duration = current_time - inter_update_clock;
      inter_update_duration = (inter_update_duration + last_update_duration) / 2.0f;
      inter_update_clock = current_time;
    }
  }

} // namespace traffic_manager
} // namespace carla
