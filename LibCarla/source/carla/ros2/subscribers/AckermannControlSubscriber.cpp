// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/AckermannControlSubscriber.h"

#include "carla/Logging.h"
#include "carla/ros2/ROS2CallbackData.h"
#include "carla/ros2/subscribers/AckermannControlConversion.h"
#include "carla/ros2/subscribers/SubscriberImpl.h"
#include "carla/ros2/types/AckermannDriveStamped.h"
#include "carla/ros2/types/AckermannDriveStampedPubSubTypes.h"

namespace carla {
namespace ros2 {

struct AckermannControlTraits {
  using msg_type = ackermann_msgs::msg::AckermannDriveStamped;
  using msg_pubsub_type = ackermann_msgs::msg::AckermannDriveStampedPubSubType;
};

AckermannControlSubscriber::AckermannControlSubscriber(
    void *vehicle, std::string base_topic_name, std::string frame_id)
  : BaseSubscriber(vehicle, std::move(base_topic_name), std::move(frame_id)),
    _impl(std::make_shared<SubscriberImpl<AckermannControlTraits>>()) {
  if (!_impl->Init(this->GetBaseTopicName() + "/ackermann_control_cmd")) {
    log_error("AckermannControlSubscriber failed to initialize on base topic",
              this->GetBaseTopicName());
  }
}

AckermannControlSubscriber::~AckermannControlSubscriber() = default;

ROS2CallbackData AckermannControlSubscriber::GetMessage() {
  auto message = _impl->GetMessage();
  return FromAckermannDrive(
      message.drive().steering_angle(),
      message.drive().steering_angle_velocity(),
      message.drive().speed(),
      message.drive().acceleration(),
      message.drive().jerk());
}

void AckermannControlSubscriber::ProcessMessages(ActorCallback callback) {
  if (_impl->HasNewMessage()) {
    auto control = this->GetMessage();
    callback(this->GetActor(), control);
  }
}

}  // namespace ros2
}  // namespace carla
