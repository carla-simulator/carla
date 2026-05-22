// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaCollisionPublisher.h"

#include "carla/Logging.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/types/CarlaCollisionEvent.h"
#include "carla/ros2/types/CarlaCollisionEventPubSubTypes.h"

namespace carla {
namespace ros2 {

struct CarlaCollisionMsgTraits {
  using msg_type = carla_msgs::msg::CarlaCollisionEvent;
  using msg_pubsub_type = carla_msgs::msg::CarlaCollisionEventPubSubType;
};

CarlaCollisionPublisher::CarlaCollisionPublisher(
    std::string base_topic_name, std::string frame_id)
  : BasePublisher(std::move(base_topic_name), std::move(frame_id)),
    _impl(std::make_shared<PublisherImpl<CarlaCollisionMsgTraits>>()) {
  if (!_impl->Init(GetBaseTopicName())) {
    log_error("CarlaCollisionPublisher: failed to initialise writer for", GetBaseTopicName());
  }
}

CarlaCollisionPublisher::~CarlaCollisionPublisher() = default;

bool CarlaCollisionPublisher::Publish() {
  return _impl->Publish();
}

bool CarlaCollisionPublisher::Write(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    std::uint32_t other_actor_id,
    float impulse_x,
    float impulse_y,
    float impulse_z) {
  auto *message = _impl->GetMessage();
  message->header().stamp().sec(seconds);
  message->header().stamp().nanosec(nanoseconds);
  message->header().frame_id(GetFrameId());

  message->other_actor_id(other_actor_id);
  message->normal_impulse().x(impulse_x);
  message->normal_impulse().y(impulse_y);
  message->normal_impulse().z(impulse_z);
  return true;
}

}  // namespace ros2
}  // namespace carla
