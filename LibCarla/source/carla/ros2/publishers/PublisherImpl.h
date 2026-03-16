// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/dds/DDSBackendFactory.h"
#include "carla/Logging.h"

namespace carla {
namespace ros2 {

/// Typed publisher frontend.
/// Holds the message object (providing typed access via GetMessage() for method chaining)
/// and delegates all DDS transport operations to an IDDSPublisherBackend created by
/// DDSBackendFactory. The concrete DDS backend is selected at startup via ROS2::Enable().
template<typename T>
class PublisherImpl {
public:
  using msg_type = typename T::msg_type;

  bool Init(const std::string& topic_name) {
#ifdef LIBCARLA_WITH_GTEST
    if (!_backend) {
#endif
      _backend = DDSBackendFactory::CreatePublisher<T>();
      if (!_backend) {
        log_error("PublisherImpl: Failed to create DDS publisher backend for topic: ", topic_name);
        return false;
      }
#ifdef LIBCARLA_WITH_GTEST
    }
#endif
    return _backend->Init(topic_name);
  }

  bool Publish() {
    if (!_backend) {
      log_error("PublisherImpl::Publish() called with no backend (Init failed or backend not compiled)");
      return false;
    }
    return _backend->Publish(&_message);
  }

  msg_type* GetMessage() {
    return &_message;
  }

  bool IsAlive() const {
    return _backend && _backend->IsAlive();
  }

  std::string GetTopicName() const {
    return _backend ? _backend->GetTopicName() : "";
  }

#ifdef LIBCARLA_WITH_GTEST
  void SetBackendForTesting(std::unique_ptr<IDDSPublisherBackend> backend) {
    _backend = std::move(backend);
  }
#endif

private:
  std::unique_ptr<IDDSPublisherBackend> _backend;
  msg_type _message;
};

} // namespace ros2
} // namespace carla
