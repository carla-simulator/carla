// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/AckermannControlSubscriber.h"

#include "carla/ros2/impl/fastdds/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

AckermannControlSubscriber::AckermannControlSubscriber(ROS2NameRecord& parent)
  : SubscriberBase(parent), _impl(std::make_shared<AckermannControlSubscriberImpl>()) {}

bool AckermannControlSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, _parent.get_topic_name("vehicle_ackermann_drive_cmd"), get_topic_qos());
}

const ackermann_msgs::msg::AckermannDriveStamped& AckermannControlSubscriber::GetMessage() {
  return _impl->GetMessage();
};

bool AckermannControlSubscriber::IsAlive() const {
  return _impl->IsAlive();
}

bool AckermannControlSubscriber::HasNewMessage() const {
  return _impl->HasNewMessage();
}
}  // namespace ros2
}  // namespace carla
