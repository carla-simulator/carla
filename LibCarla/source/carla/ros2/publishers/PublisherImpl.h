// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/ros2/dds/DDSMiddlewareFactory.h"
#include "carla/Logging.h"

namespace carla {
namespace ros2 {

  template<typename T>
  class PublisherImpl {
  public:
    using msg_type = typename T::msg_type;

    bool Init(std::string topic_name) {
      _middleware = DDSMiddlewareFactory::CreatePublisher<T>();
      if (!_middleware) {
        log_error("PublisherImpl: Failed to create middleware publisher");
        return false;
      }
      return _middleware->Init(topic_name);
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

    msg_type* GetMessage() {
      return &_message;
    }

    bool Publish() {
      if (!_middleware) {
        log_error("PublisherImpl::Publish called before Init");
        return false;
      }
      return _middleware->Publish(&_message);
    }

  private:
    std::unique_ptr<IDDSPublisherMiddleware> _middleware;
    msg_type _message;
  };

}  // namespace ros2
}  // namespace carla
