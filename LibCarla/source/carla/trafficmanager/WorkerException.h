// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#pragma once

#include <exception>
#include <utility>

namespace carla {
namespace traffic_manager {
namespace detail {

  /// Keep exceptions inside an asynchronous worker's thread entry point.
  template <typename Function>
  std::exception_ptr CaptureWorkerException(Function &&function) noexcept {
    try {
      std::forward<Function>(function)();
    } catch (...) {
      return std::current_exception();
    }
    return nullptr;
  }

} // namespace detail
} // namespace traffic_manager
} // namespace carla
