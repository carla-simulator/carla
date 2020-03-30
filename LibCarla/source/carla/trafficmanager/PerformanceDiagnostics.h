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
  using namespace chr;
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
    std::unordered_map<std::string, unsigned long> number_of_calls;

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
      if (number_of_calls.find(snippet_name) == number_of_calls.end()) {
        number_of_calls.insert({snippet_name, 0u});
      }

      TimePoint current_time = chr::system_clock::now();
      TimePoint& print_clock = print_clocks.at(snippet_name);
      TimePoint& snippet_clock = snippet_clocks.at(snippet_name);
      chr::duration<float>& snippet_duration = snippet_durations.at(snippet_name);
      unsigned long& call_count = number_of_calls.at(snippet_name);

      if (begin_or_end) {
        snippet_clock = current_time;
      } else {
        chr::duration<float> measured_duration = current_time - snippet_clock;
        snippet_duration += measured_duration;
        ++call_count;
      }

      chr::duration<float> print_duration = current_time - print_clock;
      if (print_duration.count() > 1.0f) {
        call_count = call_count == 0u ? 1 : call_count;
        std::cout << "Snippet name : " << snippet_name << ", "
                  << "avg. duration : " << 1000 * snippet_duration.count() / call_count << " ms, "
                  << "total duration : " << snippet_duration.count() << " s, "
                  << "total calls : " << call_count << ", "
                  << std::endl;

        snippet_duration = 0s;
        call_count = 0u;

        print_clock = current_time;
      }
    }
  };

} // namespace traffic_manager
} // namespace carla
