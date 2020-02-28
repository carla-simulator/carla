// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

#include "carla/Logging.h"

namespace carla {
namespace traffic_manager {

  namespace chr = std::chrono;

  using TimePoint = chr::time_point<chr::system_clock, chr::nanoseconds>;

  class PerformanceDiagnostics {

  private:
    /// Stage name.
    std::string stage_name;
    /// Throughput clock.
    TimePoint throughput_clock;
    /// Throughput counter.
    uint64_t throughput_counter;
    /// Inter-update clock.
    TimePoint inter_update_clock;
    /// Inter-update duration.
    chr::duration<float> inter_update_duration;

  public:
    PerformanceDiagnostics(std::string name);

    void RegisterUpdate(bool begin_or_end);
  };

  class SnippetProfiler {

    private:

    std::unordered_map<std::string, TimePoint> print_clocks;
    std::unordered_map<std::string, TimePoint> snippet_clocks;
    std::unordered_map<std::string, chr::duration<float>> snippet_durations;

    public:

    SnippetProfiler(){};

    void MeasureExecutionTime(std::string snippet_name, bool begin_or_end) {

      if (print_clocks.find(snippet_name) == print_clocks.end()) {
        print_clocks.insert({snippet_name, chr::system_clock::now()});
      }
      if (snippet_clocks.find(snippet_name) == snippet_clocks.end()) {
        snippet_clocks.insert({snippet_name, TimePoint()});
      }
      if (snippet_durations.find(snippet_name) == snippet_durations.end()) {
        snippet_durations.insert({snippet_name, chr::duration<float>()});
      }

      TimePoint current_time = chr::system_clock::now();
      TimePoint& print_clock = print_clocks.at(snippet_name);
      TimePoint& snippet_clock = snippet_clocks.at(snippet_name);
      chr::duration<float>& snippet_duration = snippet_durations.at(snippet_name);

      if (begin_or_end) {
        snippet_clock = current_time;
      } else {
        chr::duration<float> measured_duration = current_time - snippet_clock;
        snippet_duration = (measured_duration + snippet_duration) / 2.0f;
      }

      chr::duration<float> print_duration = current_time - print_clock;
      if (print_duration.count() > 1.0f) {
        std::cout << "Snippet name : " << snippet_name << ", "
                 << "avg. duration : " << 1000 * snippet_duration.count()
                 << " ms" << std::endl;
        print_clock = current_time;
      }
    }
  };

} // namespace traffic_manager
} // namespace carla
