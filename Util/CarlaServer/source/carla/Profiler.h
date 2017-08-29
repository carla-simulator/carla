// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#define CARLA_WITH_PROFILER

#ifdef CARLA_WITH_PROFILER

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "carla/Logging.h"
#include "carla/NonCopyable.h"
#include "carla/StopWatch.h"

namespace carla {

  template <typename ... Args>
  static inline void log_profiler(Args &&... args) {
    logging::print(std::cout, "PROFILER:", std::forward<Args>(args)..., '\n');
  }

  class ProfilerData : private NonCopyable {
  public:

    explicit ProfilerData(std::string name) : _name(std::move(name)) {}

    ~ProfilerData() {
      log_profiler(_name, ':', "annotated", _count, "times", std::fixed, std::setprecision(3));
      if (_count > 0u) {
        log_profiler(_name, ':', "average =", Average(), "ms");
        log_profiler(_name, ':', "maximum =", Maximum(), "ms");
        log_profiler(_name, ':', "minimum =", Minimum(), "ms");
      }
    }

    void Annotate(const StopWatch &stop_watch) {
      size_t elapsed_microseconds = stop_watch.GetElapsedTime<std::chrono::microseconds>();
      ++_count;
      _total_microseconds += elapsed_microseconds;
      _max_elapsed = std::max(elapsed_microseconds, _max_elapsed);
      _min_elapsed = std::min(elapsed_microseconds, _min_elapsed);
    }

    float Average() const {
      return ms(_total_microseconds) / static_cast<float>(_count);
    }

    float Maximum() const {
      return ms(_max_elapsed);
    }

    float Minimum() const {
      return ms(_min_elapsed);
    }

  private:

    static inline float ms(size_t microseconds) {
      return 1e-3f * static_cast<float>(microseconds);
    }

    std::string _name;

    size_t _count = 0u;

    size_t _total_microseconds = 0u;

    size_t _max_elapsed = 0u;

    size_t _min_elapsed = -1;
  };

  class ScopedProfiler {
  public:

    explicit ScopedProfiler(ProfilerData &parent) : _profiler(parent) {}

    ~ScopedProfiler() {
      _stop_watch.Stop();
      _profiler.Annotate(_stop_watch);
    }

  private:

    ProfilerData &_profiler;

    StopWatch _stop_watch;
  };

} // namespace carla

#define CARLA_PROFILE_SCOPE(context, name) \
    static thread_local ::carla::ProfilerData carla_profiler_ ## context ## _ ## name ## _data( \
        #context "." #name); \
    ::carla::ScopedProfiler carla_profiler_ ## context ## _ ## name ## _scoped_profiler( \
        carla_profiler_ ## context ## _ ## name ## _data);

#else // CARLA_WITH_PROFILER

#define CARLA_PROFILE_SCOPE(context, name)

#endif // CARLA_WITH_PROFILER
