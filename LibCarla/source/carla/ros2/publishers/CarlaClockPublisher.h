// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/Clock.h"
#include "carla/ros2/types/ClockPubSubTypes.h"

namespace carla {
namespace ros2 {

  class CarlaClockPublisher : public BasePublisher {
    public:
      struct ClockMsgTraits {
        using msg_type = rosgraph::msg::Clock;
        using msg_pubsub_type = rosgraph::msg::ClockPubSubType;
      };

      CarlaClockPublisher() :
        BasePublisher("rt/clock"),
        _impl(std::make_shared<PublisherImpl<ClockMsgTraits>>()) {
          _impl->Init(GetBaseTopicName());
      }

      bool Publish() {
        return _impl->Publish();
      }

      bool Write(int32_t seconds, uint32_t nanoseconds);

    private:
      std::shared_ptr<PublisherImpl<ClockMsgTraits>> _impl;
  };

}  // namespace ros2
}  // namespace carla
