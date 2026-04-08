// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/publishers/BasePublisher.h"
#include "carla/ros2/publishers/PublisherImpl.h"

#include "carla/ros2/types/msg/Clock.h"

namespace carla {
namespace ros2 {

  class CarlaClockPublisher : public BasePublisher {
    public:
      struct ClockMsgTraits {
        using msg_type = msg::Clock;
      };

      CarlaClockPublisher() :
        BasePublisher("rt/clock"),
        _impl(std::make_shared<PublisherImpl<ClockMsgTraits>>()) {
          if (!_impl->Init(GetBaseTopicName())) {
            log_warning("CarlaClockPublisher: Init failed for topic: ", GetBaseTopicName());
          }
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
