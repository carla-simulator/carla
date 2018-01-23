// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <chrono>

namespace carla {

  template <typename CLOCK>
  class StopWatchTmpl {
  public:

    using clock = CLOCK;

    StopWatchTmpl() : _start(clock::now()), _end(), _is_running(true) {}

    void Restart() {
      _is_running = true;
      _start = clock::now();
    }

    void Stop() {
      _end = clock::now();
      _is_running = false;
    }

    typename clock::duration GetDuration() const {
      return _is_running ? clock::now() - _start : _end - _start;
    }

    template <class RESOLUTION=std::chrono::milliseconds>
    typename RESOLUTION::rep GetElapsedTime() const {
      return std::chrono::duration_cast<RESOLUTION>(GetDuration()).count();
    }

    bool IsRunning() const {
      return _is_running;
    }

  private:

    typename clock::time_point _start;

    typename clock::time_point _end;

    bool _is_running;
  };

  using StopWatch = StopWatchTmpl<std::chrono::steady_clock>;

  static_assert(carla::StopWatch::clock::is_steady, "The StopWatch's clock must be steady");

} // namespace carla
