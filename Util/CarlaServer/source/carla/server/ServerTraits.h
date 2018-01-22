// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <chrono>

#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace carla {
namespace server {

  using const_buffer = boost::asio::const_buffer;

  using mutable_buffer = boost::asio::mutable_buffer;

  using error_code = boost::system::error_code;

namespace errc {

  static inline error_code success() {
    return error_code(
        boost::system::errc::success,
        boost::system::system_category());
  }

  static inline error_code try_again() {
    return boost::asio::error::basic_errors::try_again;
  }

  static inline error_code timed_out() {
    return boost::asio::error::basic_errors::timed_out;
  }

  static inline error_code invalid_argument() {
    return boost::asio::error::basic_errors::invalid_argument;
  }

  static inline error_code operation_aborted() {
    return boost::asio::error::basic_errors::operation_aborted;
  }

} // namespace errc

  using time_duration = boost::posix_time::time_duration;

  /// Positive time-out up to milliseconds resolution.
  class timeout_t {
  public:

    static inline timeout_t milliseconds(uint32_t timeout) {
      return std::chrono::milliseconds(timeout);
    }

    constexpr timeout_t() : _milliseconds(0u) {}

    template <typename Rep, typename Period>
    constexpr timeout_t(std::chrono::duration<Rep, Period> duration)
      : _milliseconds(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()) {}

    timeout_t(boost::posix_time::time_duration timeout)
      : timeout_t(std::chrono::milliseconds(timeout.total_milliseconds())) {}

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

    uint64_t _milliseconds;
  };

} // namespace server
} // namespace carla
