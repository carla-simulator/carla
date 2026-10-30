// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#pragma once

#include <atomic>
#include <exception>
#include <memory>
#include <thread>
#include <utility>

namespace carla {
namespace traffic_manager {
namespace detail {

  /// Starting a fresh worker also permits recovery after a failed worker.
  template <typename Function, typename... Args>
  std::unique_ptr<std::thread> CreateWorkerThread(
      std::atomic_bool &run, Function &&function, Args &&...args) {
    run.store(true);
    return std::make_unique<std::thread>(
        std::forward<Function>(function), std::forward<Args>(args)...);
  }

  /// Keep exceptions inside an asynchronous worker's thread entry point.
  template <typename Function>
  std::exception_ptr CaptureWorkerException(Function &&function) noexcept {
#ifndef LIBCARLA_NO_EXCEPTIONS
    try {
#endif
      std::forward<Function>(function)();
#ifndef LIBCARLA_NO_EXCEPTIONS
    } catch (...) {
      return std::current_exception();
    }
#endif
    return nullptr;
  }

} // namespace detail
} // namespace traffic_manager
} // namespace carla
