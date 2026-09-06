// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaClockPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/types/Clock.h"
#include "carla/ros2/types/ClockPubSubTypes.h"

namespace carla {
namespace ros2 {

struct CarlaClockMsgTraits {
  using msg_type = rosgraph::msg::Clock;
  using msg_pubsub_type = rosgraph::msg::ClockPubSubType;
};

CarlaClockPublisher::CarlaClockPublisher()
  : BasePublisher("rt/clock"),
    _impl(std::make_shared<PublisherImpl<CarlaClockMsgTraits>>()) {
  if (!_impl->Init(GetBaseTopicName())) {
    log_error("CarlaClockPublisher: failed to initialise writer for rt/clock");
  }
}

CarlaClockPublisher::~CarlaClockPublisher() = default;

bool CarlaClockPublisher::Publish() {
  return _impl->Publish();
}

bool CarlaClockPublisher::Write(std::int32_t seconds, std::uint32_t nanoseconds) {
  auto *message = _impl->GetMessage();
  message->clock().sec(seconds);
  message->clock().nanosec(nanoseconds);
  return true;
}

}  // namespace ros2
}  // namespace carla
