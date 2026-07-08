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

namespace carla {
namespace ros2 {

// PublisherImpl owns the transport-neutral publish path for a single message
// type. It delegates all DDS plumbing to an IPublisherMiddleware obtained from
// MiddlewareFactory::CreatePublisher<Traits>(), so the concrete middleware
// (FastDDS, CycloneDDS, ...) is selected at runtime without any vendor header
// reaching this template. The Traits struct only has to expose a `msg_type`
// typedef naming a carla::ros2::msg::* POD; the CDR type name/hash/size come
// from the CdrTopicInfo<msg_type> specialisation consumed inside the middleware.
// Concrete publishers hold a std::shared_ptr<PublisherImpl<Traits>>, populate
// the owned message in place via GetMessage(), then call Publish().
template <typename Traits>
class PublisherImpl {
public:
  using msg_type = typename Traits::msg_type;

  bool Init(std::string topic_name) {
#ifdef LIBCARLA_WITH_GTEST
    // A test may inject a fake middleware before Init(); do not overwrite it.
    if (!_middleware) {
#endif
      _middleware = MiddlewareFactory::CreatePublisher<Traits>();
      if (!_middleware) {
        log_error("PublisherImpl::Init failed to create middleware publisher");
        return false;
      }
#ifdef LIBCARLA_WITH_GTEST
    }
#endif
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

  msg_type *GetMessage() { return &_message; }

  bool Publish() {
    if (!_middleware) {
      log_error("PublisherImpl::Publish called before Init");
      return false;
    }
    return _middleware->Publish(&_message);
  }

#ifdef LIBCARLA_WITH_GTEST
  void SetMiddlewareForTesting(std::unique_ptr<IPublisherMiddleware> middleware) {
    _middleware = std::move(middleware);
  }
#endif

private:
  std::unique_ptr<IPublisherMiddleware> _middleware;
  msg_type _message{};
};

}  // namespace ros2
}  // namespace carla
