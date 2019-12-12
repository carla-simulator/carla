// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

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
    chr::time_point<chr::system_clock, chr::nanoseconds> throughput_clock;
    /// Throughput counter.
    uint64_t throughput_counter;
    /// Inter-update clock.
    chr::time_point<chr::system_clock, chr::nanoseconds> inter_update_clock;
    /// Inter-update duration.
    chr::duration<float> inter_update_duration;

  public:
    PerformanceDiagnostics(std::string name);

    void RegisterUpdate();
  };

} // namespace traffic_manager
