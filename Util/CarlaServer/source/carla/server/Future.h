// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <future>
#include <type_traits>

#include "carla/server/ServerTraits.h"

namespace carla {
namespace server {
/// Utils to handle std::future.
namespace future {

  template <typename T>
  static inline bool is_running(const std::future<T> &future) {
    return future.valid() &&
          (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready);
  }

  template <typename T>
  static inline bool is_ready(const std::future<T> &future, timeout_t timeout) {
    return future.valid() &&
          (future.wait_for(timeout.to_chrono()) == std::future_status::ready);
  }

  template <typename T>
  static inline bool is_ready(const std::future<T> &future) {
    return is_ready(future, timeout_t::milliseconds(0));
  }

  template <typename T>
  static inline bool wait_and_get(std::future<T> &future, T &result, timeout_t timeout) {
    if (is_ready(future, timeout)) {
      result = std::move(future.get());
      return true;
    } else {
      return false;
    }
  }

  template <typename T>
  static inline bool try_get(std::future<T> &future, T &result) {
    return wait_and_get(future, result, timeout_t::milliseconds(0));
  }

} // namespace future
} // namespace server
} // namespace carla
