// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/// @file Time.h
/// @brief Time utilities for CARLA

#ifndef CARLA_TIME_H
#define CARLA_TIME_H
/// @brief Time utilities for CARLA
///
/// Provides time duration handling with millisecond resolution and automatic
/// conversion between std::chrono and boost::posix_time.

#include "carla/Debug.h"
#include <chrono>
#include <cstddef>

#if __has_include(<boost/date_time/posix_time/posix_time_types.hpp>)
#include <boost/date_time/posix_time/posix_time_types.hpp>
#endif

namespace carla {

  /// @brief Positive time duration up to milliseconds resolution.
  ///
  /// Automatically casts between std::chrono::duration and
  /// boost::posix_time::time_duration when boost is available.
  ///
  /// @example
  /// ```cpp
  /// time_duration timeout = time_duration::milliseconds(100);
  /// auto chrono_duration = timeout.to_chrono();
  /// ```
  class time_duration {
  public:
    /// @brief Create duration from seconds.
    /// @param timeout Number of seconds.
    /// @return Time duration object.
    static inline time_duration seconds(size_t timeout) {
      return std::chrono::seconds(timeout);
    }

    /// @brief Create duration from milliseconds.
    /// @param timeout Number of milliseconds.
    /// @return Time duration object.
    static inline time_duration milliseconds(size_t timeout) {
      return std::chrono::milliseconds(timeout);
    }

    /// @brief Default constructor (zero duration).
    constexpr time_duration() noexcept : _milliseconds(0u) {}

    /// @brief Construct from std::chrono::duration.
    /// @tparam Rep Clock representation type.
    /// @tparam Period Clock period type.
    /// @param duration Duration to convert.
    template <typename Rep, typename Period>
    time_duration(std::chrono::duration<Rep, Period> duration)
      : _milliseconds([=]() {
          const auto count = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
          DEBUG_ASSERT(count >= 0);
          return static_cast<size_t>(count);
        }()) {}

    time_duration(const time_duration &) = default;
    time_duration &operator=(const time_duration &) = default;

#if __has_include(<boost/date_time/posix_time/posix_time_types.hpp>)
    /// @brief Construct from boost::posix_time::time_duration.
    /// @param timeout Boost time duration.
    time_duration(boost::posix_time::time_duration timeout)
      : time_duration(std::chrono::milliseconds(timeout.total_milliseconds()))
    {
    }

    /// @brief Convert to boost::posix_time::time_duration.
    /// @return Boost time duration.
    boost::posix_time::time_duration to_posix_time() const {
      return boost::posix_time::milliseconds(_milliseconds);
    }
#endif

    /// @brief Convert to std::chrono::milliseconds.
    /// @return Chrono duration in milliseconds.
    constexpr auto to_chrono() const {
      return std::chrono::milliseconds(_milliseconds);
    }

    /// @brief Implicit conversion to boost::posix_time::time_duration.
    /// @return Boost time duration.
    operator boost::posix_time::time_duration() const {
      return to_posix_time();
    }

    /// @brief Get duration in milliseconds.
    /// @return Duration as milliseconds.
    constexpr size_t milliseconds() const noexcept {
      return _milliseconds;
    }

  private:
    size_t _milliseconds;  ///< Duration in milliseconds
  };

} // namespace carla

#endif // CARLA_TIME_H
