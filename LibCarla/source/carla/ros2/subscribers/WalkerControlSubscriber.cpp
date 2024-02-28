// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/WalkerControlSubscriber.h"

#include "carla/ros2/impl/fastdds/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

WalkerControlSubscriber::WalkerControlSubscriber(ROS2NameRecord& parent)
  : SubscriberBase(parent), _impl(std::make_shared<WalkerControlSubscriberImpl>()) {}

bool WalkerControlSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, _parent.get_topic_name("walker_control_cmd"), get_topic_qos());
}

const carla_msgs::msg::CarlaWalkerControl& WalkerControlSubscriber::GetMessage() {
  return _impl->GetMessage();
};

bool WalkerControlSubscriber::IsAlive() const {
  return _impl->IsAlive();
}

bool WalkerControlSubscriber::HasNewMessage() const {
  return _impl->HasNewMessage();
}
}  // namespace ros2
}  // namespace carla
