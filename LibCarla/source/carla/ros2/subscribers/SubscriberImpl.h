// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/Logging.h"
#include "carla/ros2/middleware/MiddlewareFactory.h"
#include "carla/ros2/subscribers/BaseSubscriber.h"

namespace carla {
namespace ros2 {

// SubscriberImpl mirrors PublisherImpl on the subscriber side. It delegates all
// DDS plumbing to an ISubscriberMiddleware obtained from
// MiddlewareFactory::CreateSubscriber<Traits>(); the middleware writes arriving
// samples directly into the SubscriberImpl-owned _message storage and raises the
// _new_message flag passed down in Init(). The Traits struct only has to expose a
// `msg_type` typedef naming a carla::ros2::msg::* POD. The next ProcessMessages
// call on the owning BaseSubscriber drains a fresh sample via GetMessage().
template <typename Traits>
class SubscriberImpl {
public:
  using msg_type = typename Traits::msg_type;

  bool Init(std::string topic_name) {
    if (!EnsureMiddleware()) {
      return false;
    }
    return _middleware->Init(topic_name, &_message, &_new_message);
  }

  /// Init with an explicit per-topic QoS profile (see QosProfile.h).
  bool Init(std::string topic_name, const QosProfile& qos) {
    if (!EnsureMiddleware()) {
      return false;
    }
    return _middleware->Init(topic_name, &_message, &_new_message, qos);
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

#ifdef LIBCARLA_WITH_GTEST
  void SetMiddlewareForTesting(std::unique_ptr<ISubscriberMiddleware> middleware) {
    _middleware = std::move(middleware);
  }

  void SimulateMessageReceiptForTesting(const msg_type &msg) {
    _message = msg;
    _new_message = true;
  }
#endif

private:
  bool EnsureMiddleware() {
#ifdef LIBCARLA_WITH_GTEST
    // A test may inject a fake middleware before Init(); do not overwrite it.
    if (!_middleware) {
#endif
      _middleware = MiddlewareFactory::CreateSubscriber<Traits>();
      if (!_middleware) {
        log_error("SubscriberImpl::Init failed to create middleware subscriber");
        return false;
      }
#ifdef LIBCARLA_WITH_GTEST
    }
#endif
    return true;
  }

  std::unique_ptr<ISubscriberMiddleware> _middleware;
  msg_type _message{};
  bool _new_message{false};
};

}  // namespace ros2
}  // namespace carla
