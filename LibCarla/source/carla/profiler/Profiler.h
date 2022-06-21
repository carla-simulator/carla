// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifndef LIBCARLA_ENABLE_PROFILER
#  define CARLA_PROFILE_SCOPE(context, profiler_name)
#  define CARLA_PROFILE_FPS(context, profiler_name)
#else

#include "carla/StopWatch.h"

#include <algorithm>
#include <limits>
#include <string>

namespace carla {
namespace profiler {
namespace detail {

  class ProfilerData {
  public:

    explicit ProfilerData(std::string name, bool print_fps = false)
      : _name(std::move(name)),
        _print_fps(print_fps) {}

    ~ProfilerData();

    void Annotate(const StopWatch &stop_watch) {
      const auto elapsed_microseconds = stop_watch.GetElapsedTime<std::chrono::microseconds>();
      ++_count;
      _total_microseconds += elapsed_microseconds;
      _max_elapsed = std::max(elapsed_microseconds, _max_elapsed);
      _min_elapsed = std::min(elapsed_microseconds, _min_elapsed);
    }

    float average() const {
      return ms(_total_microseconds) / static_cast<float>(_count);
    }

    float maximum() const {
      return ms(_max_elapsed);
    }

    float minimum() const {
      return ms(_min_elapsed);
    }

  private:

    static inline float ms(size_t microseconds) {
      return 1e-3f * static_cast<float>(microseconds);
    }

    static inline float fps(float milliseconds) {
      return milliseconds > 0.0f ? (1e3f / milliseconds) : std::numeric_limits<float>::max();
    }

    const std::string _name;

    const bool _print_fps;

    size_t _count = 0u;

    size_t _total_microseconds = 0u;

    size_t _max_elapsed = 0u;

    size_t _min_elapsed = std::numeric_limits<size_t>::max();
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

} // namespace detail
} // namespace profiler
} // namespace carla

#ifdef LIBCARLA_WITH_GTEST
#  define LIBCARLA_GTEST_GET_TEST_NAME() std::string(::testing::UnitTest::GetInstance()->current_test_info()->name())
#else
#  define LIBCARLA_GTEST_GET_TEST_NAME() std::string("")
#endif // LIBCARLA_WITH_GTEST

#define CARLA_PROFILE_SCOPE(context, profiler_name) \
    static thread_local ::carla::profiler::detail::ProfilerData carla_profiler_ ## context ## _ ## profiler_name ## _data( \
        LIBCARLA_GTEST_GET_TEST_NAME() + "." #context "." #profiler_name); \
    ::carla::profiler::detail::ScopedProfiler carla_profiler_ ## context ## _ ## profiler_name ## _scoped_profiler( \
        carla_profiler_ ## context ## _ ## profiler_name ## _data);

#define CARLA_PROFILE_FPS(context, profiler_name) \
    { \
      static thread_local ::carla::StopWatch stop_watch; \
      stop_watch.Stop(); \
      static thread_local bool first_time = true; \
      if (!first_time) { \
        static thread_local ::carla::profiler::detail::ProfilerData profiler_data( \
            LIBCARLA_GTEST_GET_TEST_NAME() + "." #context "." #profiler_name, true); \
        profiler_data.Annotate(stop_watch); \
      } else { \
        first_time = false; \
      } \
      stop_watch.Restart(); \
    }

#endif // LIBCARLA_ENABLE_PROFILER
