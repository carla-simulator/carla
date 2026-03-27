// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/ros2/subscribers/BaseSubscriber.h"
#include "carla/ros2/dds/DDSMiddlewareFactory.h"
#include "carla/Logging.h"

namespace carla {
namespace ros2 {

  template<typename S>
  class SubscriberImpl {
  public:
    using msg_type = typename S::msg_type;

    bool Init(std::string topic_name) {
      _middleware = DDSMiddlewareFactory::CreateSubscriber<S>();
      if (!_middleware) {
        log_error("SubscriberImpl: Failed to create middleware subscriber");
        return false;
      }
      return _middleware->Init(topic_name, &_message, &_new_message);
    }

    std::string GetTopicName() {
      if (_middleware) {
        return _middleware->GetTopicName();
      }
      return "";
    }

    bool IsAlive() {
      if (_middleware) {
        return _middleware->IsAlive();
      }
      return false;
    }

    msg_type GetMessage() {
      _new_message = false;
      return _message;
    }

    bool HasNewMessage() { return _new_message; }

  private:
    std::unique_ptr<IDDSSubscriberMiddleware> _middleware;
    msg_type _message;
    bool _new_message { false };
  };

}  // namespace ros2
}  // namespace carla
