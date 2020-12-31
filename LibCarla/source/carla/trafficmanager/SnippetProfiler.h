// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <sstream>
#include <mutex>

namespace carla {
namespace traffic_manager {

namespace chr = std::chrono;
using namespace chr;
using TimePoint = chr::time_point<chr::system_clock, chr::nanoseconds>;

// This class can be used to measure execution time, total call duration and number of calls
// of any code snippet by assigning it a name.
class SnippetProfiler {

private:
  std::unordered_map<std::string, TimePoint> print_clocks;
  std::unordered_map<std::string, TimePoint> snippet_clocks;
  std::unordered_map<std::string, chr::duration<float>> snippet_durations;
  std::unordered_map<std::string, unsigned long> number_of_calls;

public:
  SnippetProfiler(){};

  void MeasureExecutionTime(std::string snippet_name, bool begin_or_end) {
    TimePoint current_time = chr::system_clock::now();

    if (print_clocks.find(snippet_name) == print_clocks.end()) {
      print_clocks.insert({snippet_name, current_time});
    }
    if (snippet_clocks.find(snippet_name) == snippet_clocks.end()) {
      snippet_clocks.insert({snippet_name, current_time});
    }
    if (snippet_durations.find(snippet_name) == snippet_durations.end()) {
      snippet_durations.insert({snippet_name, chr::duration<float>()});
    }
    if (number_of_calls.find(snippet_name) == number_of_calls.end()) {
      number_of_calls.insert({snippet_name, 0u});
    }

    TimePoint &print_clock = print_clocks.at(snippet_name);
    TimePoint &snippet_clock = snippet_clocks.at(snippet_name);
    chr::duration<float> &snippet_duration = snippet_durations.at(snippet_name);
    unsigned long &call_count = number_of_calls.at(snippet_name);

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


class TicToc {

private:
  std::chrono::high_resolution_clock::time_point t0_, last_print_;
  size_t cnt_;
  std::unordered_map<std::string, double> time_;
  std::string name_;
  
public:
  TicToc(const std::string & name=""):t0_(std::chrono::high_resolution_clock::now()), last_print_(std::chrono::high_resolution_clock::now()), cnt_(0), name_(name){};
  void tic() {
    t0_ = std::chrono::high_resolution_clock::now();
  }
  void toc(const std::string & name) {
    using namespace std::chrono;
    auto current_time = high_resolution_clock::now();
    if (!time_.count(name))
      time_[name] = 0.;
    time_[name] += duration_cast<duration<double>>(current_time - t0_).count();
    t0_ = current_time;
  }
  void finish(double print_interval=5, double merge_below_percent=0.5) {
    using namespace std::chrono;
    cnt_ += 1;
    auto current_time = high_resolution_clock::now();
    if (duration_cast<duration<double>>(current_time-last_print_).count() > print_interval)
    {
      std::ostringstream ss;
      ss << "-----" << name_ << "-----" << std::endl;
      double total = 0;
      for(auto & v: time_)
        total += v.second;
      double merged = 0;
      for(auto & v: time_) {
        if (v.second < total * merge_below_percent / 100.)
          merged += v.second;
        else
          ss << std::setw (30) << v.first << "    " << v.second / cnt_ << std::endl;
      }
      if (merged > 0)
        ss << std::setw (30) << "<merged>" << "    " << merged / cnt_ << std::endl;
      ss << std::setw (30) << "total" << "    " << total / cnt_ << std::endl;
      ss << "------------" << std::endl;
      {
//         static std::mutex mutex;
//         const std::lock_guard<std::mutex> lock(mutex);
        std::cout << ss.str();
        std::cout.flush();
      }
      cnt_ = 0;
      last_print_ = current_time;
      time_.clear();
    }
  }
};
