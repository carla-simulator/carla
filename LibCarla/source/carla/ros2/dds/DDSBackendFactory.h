// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/ros2/dds/DDSBackend.h"
#include "carla/ros2/dds/IDDSPublisherBackend.h"
#include "carla/ros2/dds/IDDSSubscriberBackend.h"
#include "carla/Logging.h"

#if defined(CARLA_ROS2_DDS_FASTDDS)
#  include "carla/ros2/dds/fastdds/FastDDSPublisherBackend.h"
#  include "carla/ros2/dds/fastdds/FastDDSSubscriberBackend.h"
#endif

#if defined(CARLA_ROS2_DDS_CYCLONEDDS)
#  include "carla/ros2/dds/cyclonedds/CycloneDDSPublisherBackend.h"
#  include "carla/ros2/dds/cyclonedds/CycloneDDSSubscriberBackend.h"
#endif

namespace carla {
namespace ros2 {

/// Factory that creates DDS publisher/subscriber backends based on the active backend selection.
/// The backend is set once at startup via SetBackend() before any DDS entities are created.
/// After the first entity is created, changing the backend has undefined behavior.
class DDSBackendFactory {
public:
  /// Select the DDS backend for all subsequent publisher/subscriber creation.
  /// Must be called before any publisher or subscriber is initialized.
  static void SetBackend(DDSBackend backend) {
    GetActiveBackend() = backend;
  }

  /// @return The currently selected DDS backend.
  static DDSBackend GetBackend() {
    return GetActiveBackend();
  }

  /// Check whether a specific backend was compiled into this binary.
  static bool IsBackendAvailable(DDSBackend backend) {
    switch (backend) {
      case DDSBackend::FastDDS:
#if defined(CARLA_ROS2_DDS_FASTDDS)
        return true;
#else
        return false;
#endif
      case DDSBackend::CycloneDDS:
#if defined(CARLA_ROS2_DDS_CYCLONEDDS)
        return true;
#else
        return false;
#endif
    }
    return false;
  }

  /// Result of backend resolution — whether resolution succeeded and which backend to use.
  struct BackendResolution {
    bool success;
    DDSBackend backend;
  };

  /// Resolve the requested backend strictly — no fallback to other backends.
  /// Returns {true, requested} if available, {false, requested} otherwise.
  static BackendResolution ResolveBackend(DDSBackend requested) {
    if (IsBackendAvailable(requested))
      return {true, requested};
    return {false, requested};
  }

  /// Return a readable list of backends compiled into this binary.
  /// Delegates to the free function in DDSBackend.h.
  static std::string GetAvailableBackendsString() {
    return carla::ros2::GetAvailableBackendsString();
  }

  /// Create a publisher backend for traits type T.
  /// T must provide:
  ///   T::msg_type  — the POD message type
  template<typename T>
  static std::unique_ptr<IDDSPublisherBackend> CreatePublisher() {
    switch (GetActiveBackend()) {
      case DDSBackend::FastDDS:
#if defined(CARLA_ROS2_DDS_FASTDDS)
        return std::unique_ptr<IDDSPublisherBackend>(new FastDDSPublisherBackend<T>());
#else
        log_error("DDSBackendFactory: FastDDS backend not compiled in (rebuild with --included-dds=fastdds)");
        return nullptr;
#endif
      case DDSBackend::CycloneDDS:
#if defined(CARLA_ROS2_DDS_CYCLONEDDS)
        return std::unique_ptr<IDDSPublisherBackend>(new CycloneDDSPublisherBackend<T>());
#else
        log_error("DDSBackendFactory: CycloneDDS backend not compiled in (rebuild with --included-dds=cyclonedds)");
        return nullptr;
#endif
    }
    return nullptr;
  }

  /// Create a subscriber backend for traits type S.
  /// S must provide:
  ///   S::msg_type  — the POD message type
  template<typename S>
  static std::unique_ptr<IDDSSubscriberBackend> CreateSubscriber() {
    switch (GetActiveBackend()) {
      case DDSBackend::FastDDS:
#if defined(CARLA_ROS2_DDS_FASTDDS)
        return std::unique_ptr<IDDSSubscriberBackend>(new FastDDSSubscriberBackend<S>());
#else
        log_error("DDSBackendFactory: FastDDS backend not compiled in (rebuild with --included-dds=fastdds)");
        return nullptr;
#endif
      case DDSBackend::CycloneDDS:
#if defined(CARLA_ROS2_DDS_CYCLONEDDS)
        return std::unique_ptr<IDDSSubscriberBackend>(new CycloneDDSSubscriberBackend<S>());
#else
        log_error("DDSBackendFactory: CycloneDDS backend not compiled in (rebuild with --included-dds=cyclonedds)");
        return nullptr;
#endif
    }
    return nullptr;
  }

private:
  /// Returns reference to the active backend selection (function-local static for C++11 thread safety).
  static DDSBackend& GetActiveBackend() {
    static DDSBackend active_backend = DDSBackend::FastDDS;
    return active_backend;
  }
};

} // namespace ros2
} // namespace carla
