// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <chrono>

namespace carla {
namespace streaming {
namespace low_level {
namespace tcp {

  /// Positive time-out up to milliseconds resolution.
  class timeout_type {
  public:

    static inline timeout_type seconds(size_t timeout) {
      return std::chrono::seconds(timeout);
    }

    static inline timeout_type milliseconds(size_t timeout) {
      return std::chrono::milliseconds(timeout);
    }

    constexpr timeout_type() : _milliseconds(0u) {}

    template <typename Rep, typename Period>
    timeout_type(std::chrono::duration<Rep, Period> duration)
      : _milliseconds(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()) {}

    timeout_type(boost::posix_time::time_duration timeout)
      : timeout_type(std::chrono::milliseconds(timeout.total_milliseconds())) {}

    timeout_type(const timeout_type &) = default;
    timeout_type &operator=(const timeout_type &) = default;

    boost::posix_time::time_duration to_posix_time() const {
      return boost::posix_time::milliseconds(_milliseconds);
    }

    constexpr auto to_chrono() const {
      return std::chrono::milliseconds(_milliseconds);
    }

    operator boost::posix_time::time_duration() const {
      return to_posix_time();
    }

  private:

    size_t _milliseconds;
  };

} // namespace tcp
} // namespace low_level
} // namespace streaming
} // namespace carla
