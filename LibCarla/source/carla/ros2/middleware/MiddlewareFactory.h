// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/ros2/middleware/Middleware.h"
#include "carla/ros2/middleware/IPublisherMiddleware.h"
#include "carla/ros2/middleware/ISubscriberMiddleware.h"
#include "carla/Logging.h"

#if defined(CARLA_ROS2_MIDDLEWARE_FASTDDS) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
#  include "carla/ros2/middleware/fastdds/FastDDSPublisherMiddleware.h"
#  include "carla/ros2/middleware/fastdds/FastDDSSubscriberMiddleware.h"
#endif

#if defined(CARLA_ROS2_MIDDLEWARE_CYCLONEDDS) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
#  include "carla/ros2/middleware/cyclonedds/CycloneDDSPublisherMiddleware.h"
#  include "carla/ros2/middleware/cyclonedds/CycloneDDSSubscriberMiddleware.h"
#endif

#if defined(CARLA_ROS2_MIDDLEWARE_ZENOH) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
#  include "carla/ros2/middleware/zenoh/ZenohPublisherMiddleware.h"
#  include "carla/ros2/middleware/zenoh/ZenohSubscriberMiddleware.h"
#endif

namespace carla {
namespace ros2 {

/// Factory that creates publisher/subscriber middleware based on the active middleware selection.
/// The middleware is set once at startup via SetMiddleware() before any entities are created.
/// After the first entity is created, changing the middleware has undefined behavior.
class MiddlewareFactory {
 public:
  /// Select the middleware for all subsequent publisher/subscriber creation.
  /// Must be called before any publisher or subscriber is initialized.
  static void SetMiddleware(Middleware middleware) {
    GetActiveMiddleware() = middleware;
  }

  /// @return The currently selected middleware.
  static Middleware GetMiddleware() {
    return GetActiveMiddleware();
  }

  /// Check whether a specific middleware was compiled into this binary.
  static bool IsMiddlewareAvailable(Middleware middleware) {
    switch (middleware) {
      case Middleware::FastDDS:
#if defined(CARLA_ROS2_MIDDLEWARE_FASTDDS)
        return true;
#else
        return false;
#endif
      case Middleware::CycloneDDS:
#if defined(CARLA_ROS2_MIDDLEWARE_CYCLONEDDS)
        return true;
#else
        return false;
#endif
      case Middleware::Zenoh:
#if defined(CARLA_ROS2_MIDDLEWARE_ZENOH)
        return true;
#else
        return false;
#endif
    }
    return false;
  }

  /// Result of middleware resolution — whether resolution succeeded and which middleware to use.
  struct MiddlewareResolution {
    bool success;
    Middleware middleware;
  };

  /// Resolve the requested middleware strictly — no fallback to other middleware.
  /// Returns {true, requested} if available, {false, requested} otherwise.
  static MiddlewareResolution ResolveMiddleware(Middleware requested) {
    if (IsMiddlewareAvailable(requested)) {
      return {true, requested};
    }
    return {false, requested};
  }

  /// Return a readable list of middleware implementations compiled into this binary.
  /// Delegates to the free function in Middleware.h.
  static std::string GetAvailableMiddlewareString() {
    return carla::ros2::GetAvailableMiddlewareString();
  }

  /// Create a publisher middleware for traits type T.
  /// T must provide:
  ///   T::msg_type  — the message type
  template<typename T>
  static std::unique_ptr<IPublisherMiddleware> CreatePublisher() {
    switch (GetActiveMiddleware()) {
      case Middleware::FastDDS:
#if defined(CARLA_ROS2_MIDDLEWARE_FASTDDS) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
        return std::unique_ptr<IPublisherMiddleware>(
            new FastDDSPublisherMiddleware<T>());
#else
        log_error("MiddlewareFactory: FastDDS not compiled in");
        return nullptr;
#endif
      case Middleware::CycloneDDS:
#if defined(CARLA_ROS2_MIDDLEWARE_CYCLONEDDS) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
        return std::unique_ptr<IPublisherMiddleware>(
            new CycloneDDSPublisherMiddleware<T>());
#else
        log_error("MiddlewareFactory: CycloneDDS not compiled in");
        return nullptr;
#endif
      case Middleware::Zenoh:
#if defined(CARLA_ROS2_MIDDLEWARE_ZENOH) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
        return std::unique_ptr<IPublisherMiddleware>(
            new ZenohPublisherMiddleware<T>());
#else
        log_error("MiddlewareFactory: Zenoh not compiled in");
        return nullptr;
#endif
    }
    return nullptr;
  }

  /// Create a subscriber middleware for traits type S.
  /// S must provide:
  ///   S::msg_type  — the message type
  template<typename S>
  static std::unique_ptr<ISubscriberMiddleware> CreateSubscriber() {
    switch (GetActiveMiddleware()) {
      case Middleware::FastDDS:
#if defined(CARLA_ROS2_MIDDLEWARE_FASTDDS) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
        return std::unique_ptr<ISubscriberMiddleware>(
            new FastDDSSubscriberMiddleware<S>());
#else
        log_error("MiddlewareFactory: FastDDS not compiled in");
        return nullptr;
#endif
      case Middleware::CycloneDDS:
#if defined(CARLA_ROS2_MIDDLEWARE_CYCLONEDDS) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
        return std::unique_ptr<ISubscriberMiddleware>(
            new CycloneDDSSubscriberMiddleware<S>());
#else
        log_error("MiddlewareFactory: CycloneDDS not compiled in");
        return nullptr;
#endif
      case Middleware::Zenoh:
#if defined(CARLA_ROS2_MIDDLEWARE_ZENOH) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
        return std::unique_ptr<ISubscriberMiddleware>(
            new ZenohSubscriberMiddleware<S>());
#else
        log_error("MiddlewareFactory: Zenoh not compiled in");
        return nullptr;
#endif
    }
    return nullptr;
  }

 private:
  /// Returns reference to the active middleware selection
  /// (function-local static for C++11 thread safety).
  static Middleware& GetActiveMiddleware() {
    static Middleware active_middleware = Middleware::FastDDS;
    return active_middleware;
  }
};

} // namespace ros2
} // namespace carla
