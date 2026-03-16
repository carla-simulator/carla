// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/subscribers/BaseSubscriber.h"
#include "carla/ros2/dds/DDSBackendFactory.h"
#include "carla/Logging.h"

namespace carla {
namespace ros2 {

/// Typed subscriber frontend.
/// Holds the message storage (msg_type _message) and a new-message flag.
/// The DDS backend writes received messages directly into _message via a pointer
/// provided at Init(), avoiding an extra copy.
/// Message reading via GetMessage() returns by value (clears the new-message flag).
template<typename S>
class SubscriberImpl {
public:
  using msg_type = typename S::msg_type;

  bool Init(const std::string& topic_name) {
#ifdef LIBCARLA_WITH_GTEST
    if (!_backend) {
#endif
      _backend = DDSBackendFactory::CreateSubscriber<S>();
      if (!_backend) {
        log_error("SubscriberImpl: Failed to create DDS subscriber backend for topic: ", topic_name);
        return false;
      }
#ifdef LIBCARLA_WITH_GTEST
    }
#endif
    return _backend->Init(topic_name, &_message, &_new_message);
  }

  std::string GetTopicName() const {
    return _backend ? _backend->GetTopicName() : "";
  }

  bool IsAlive() const {
    return _backend && _backend->IsAlive();
  }

  msg_type GetMessage() {
    _new_message = false;
    return _message;
  }

  bool HasNewMessage() const {
    return _new_message;
  }

#ifdef LIBCARLA_WITH_GTEST
  void SetBackendForTesting(std::unique_ptr<IDDSSubscriberBackend> backend) {
    _backend = std::move(backend);
  }

  void SimulateMessageReceiptForTesting(const msg_type& msg) {
    _message = msg;
    _new_message = true;
  }
#endif

private:
  std::unique_ptr<IDDSSubscriberBackend> _backend;
  msg_type _message;
  bool     _new_message { false };
};

} // namespace ros2
} // namespace carla
