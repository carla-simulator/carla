// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <chrono>

namespace carla {

  /// Positive time duration up to milliseconds resolution. Automatically casts
  /// between std::chrono::duration and boost::posix_time::time_duration.
  class time_duration {
  public:

    static inline time_duration seconds(size_t timeout) {
      return std::chrono::seconds(timeout);
    }

    static inline time_duration milliseconds(size_t timeout) {
      return std::chrono::milliseconds(timeout);
    }

    constexpr time_duration() noexcept : _milliseconds(0u) {}

    template <typename Rep, typename Period>
    time_duration(std::chrono::duration<Rep, Period> duration)
      : _milliseconds([=]() {
          const auto count = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
          DEBUG_ASSERT(count >= 0);
          return static_cast<size_t>(count);
        }()) {}

    time_duration(boost::posix_time::time_duration timeout)
      : time_duration(std::chrono::milliseconds(timeout.total_milliseconds())) {}

    time_duration(const time_duration &) = default;
    time_duration &operator=(const time_duration &) = default;

    boost::posix_time::time_duration to_posix_time() const {
      return boost::posix_time::milliseconds(_milliseconds);
    }

    constexpr auto to_chrono() const {
      return std::chrono::milliseconds(_milliseconds);
    }

    operator boost::posix_time::time_duration() const {
      return to_posix_time();
    }

    constexpr size_t milliseconds() const noexcept {
      return _milliseconds;
    }

  private:

    size_t _milliseconds;
  };

} // namespace carla
