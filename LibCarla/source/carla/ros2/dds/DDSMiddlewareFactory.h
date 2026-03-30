// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/ros2/dds/DDSMiddleware.h"
#include "carla/ros2/dds/IDDSPublisherMiddleware.h"
#include "carla/ros2/dds/IDDSSubscriberMiddleware.h"
#include "carla/Logging.h"

#if defined(CARLA_ROS2_DDS_FASTDDS) && !defined(CARLA_ROS2_DDS_TESTING)
#  include "carla/ros2/dds/fastdds/FastDDSPublisherMiddleware.h"
#  include "carla/ros2/dds/fastdds/FastDDSSubscriberMiddleware.h"
#endif

#if defined(CARLA_ROS2_DDS_CYCLONEDDS) && !defined(CARLA_ROS2_DDS_TESTING)
#  include "carla/ros2/dds/cyclonedds/CycloneDDSPublisherMiddleware.h"
#  include "carla/ros2/dds/cyclonedds/CycloneDDSSubscriberMiddleware.h"
#endif

namespace carla {
namespace ros2 {

/// Factory that creates DDS publisher/subscriber middleware based on the active middleware selection.
/// The middleware is set once at startup via SetMiddleware() before any DDS entities are created.
/// After the first entity is created, changing the middleware has undefined behavior.
class DDSMiddlewareFactory {
 public:
  /// Select the DDS middleware for all subsequent publisher/subscriber creation.
  /// Must be called before any publisher or subscriber is initialized.
  static void SetMiddleware(DDSMiddleware middleware) {
    GetActiveMiddleware() = middleware;
  }

  /// @return The currently selected DDS middleware.
  static DDSMiddleware GetMiddleware() {
    return GetActiveMiddleware();
  }

  /// Check whether a specific middleware was compiled into this binary.
  static bool IsMiddlewareAvailable(DDSMiddleware middleware) {
    switch (middleware) {
      case DDSMiddleware::FastDDS:
#if defined(CARLA_ROS2_DDS_FASTDDS)
        return true;
#else
        return false;
#endif
      case DDSMiddleware::CycloneDDS:
#if defined(CARLA_ROS2_DDS_CYCLONEDDS)
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
    DDSMiddleware middleware;
  };

  /// Resolve the requested middleware strictly — no fallback to other middleware.
  /// Returns {true, requested} if available, {false, requested} otherwise.
  static MiddlewareResolution ResolveMiddleware(DDSMiddleware requested) {
    if (IsMiddlewareAvailable(requested)) {
      return {true, requested};
    }
    return {false, requested};
  }

  /// Return a readable list of middleware implementations compiled into this binary.
  /// Delegates to the free function in DDSMiddleware.h.
  static std::string GetAvailableMiddlewareString() {
    return carla::ros2::GetAvailableMiddlewareString();
  }

  /// Create a publisher middleware for traits type T.
  /// T must provide:
  ///   T::msg_type  — the message type
  template<typename T>
  static std::unique_ptr<IDDSPublisherMiddleware> CreatePublisher() {
    switch (GetActiveMiddleware()) {
      case DDSMiddleware::FastDDS:
#if defined(CARLA_ROS2_DDS_FASTDDS) && !defined(CARLA_ROS2_DDS_TESTING)
        return std::unique_ptr<IDDSPublisherMiddleware>(
            new FastDDSPublisherMiddleware<T>());
#else
        log_error("DDSMiddlewareFactory: FastDDS not compiled in");
        return nullptr;
#endif
      case DDSMiddleware::CycloneDDS:
#if defined(CARLA_ROS2_DDS_CYCLONEDDS) && !defined(CARLA_ROS2_DDS_TESTING)
        return std::unique_ptr<IDDSPublisherMiddleware>(
            new CycloneDDSPublisherMiddleware<T>());
#else
        log_error("DDSMiddlewareFactory: CycloneDDS not compiled in");
        return nullptr;
#endif
    }
    return nullptr;
  }

  /// Create a subscriber middleware for traits type S.
  /// S must provide:
  ///   S::msg_type  — the message type
  template<typename S>
  static std::unique_ptr<IDDSSubscriberMiddleware> CreateSubscriber() {
    switch (GetActiveMiddleware()) {
      case DDSMiddleware::FastDDS:
#if defined(CARLA_ROS2_DDS_FASTDDS) && !defined(CARLA_ROS2_DDS_TESTING)
        return std::unique_ptr<IDDSSubscriberMiddleware>(
            new FastDDSSubscriberMiddleware<S>());
#else
        log_error("DDSMiddlewareFactory: FastDDS not compiled in");
        return nullptr;
#endif
      case DDSMiddleware::CycloneDDS:
#if defined(CARLA_ROS2_DDS_CYCLONEDDS) && !defined(CARLA_ROS2_DDS_TESTING)
        return std::unique_ptr<IDDSSubscriberMiddleware>(
            new CycloneDDSSubscriberMiddleware<S>());
#else
        log_error("DDSMiddlewareFactory: CycloneDDS not compiled in");
        return nullptr;
#endif
    }
    return nullptr;
  }

 private:
  /// Returns reference to the active middleware selection
  /// (function-local static for C++11 thread safety).
  static DDSMiddleware& GetActiveMiddleware() {
    static DDSMiddleware active_middleware = DDSMiddleware::FastDDS;
    return active_middleware;
  }
};

} // namespace ros2
} // namespace carla
