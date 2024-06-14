// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/WalkerControlSubscriber.h"

#include "carla/ros2/impl/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

WalkerControlSubscriber::WalkerControlSubscriber(ROS2NameRecord& parent,
                                                 carla::ros2::types::WalkerControlCallback walker_control_callback)
  : SubscriberBase(parent),
    _impl(std::make_shared<WalkerControlSubscriberImpl>(*this)),
    _walker_control_callback(walker_control_callback) {}

bool WalkerControlSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, get_topic_name("control/walker_control_cmd"), get_topic_qos());
}

void WalkerControlSubscriber::ProcessMessages() {
  while (_impl->HasPublishersConnected() && _impl->HasNewMessage()) {
    _walker_control_callback(carla::ros2::types::WalkerControl(_impl->GetMessage()));
  }
}

}  // namespace ros2
}  // namespace carla
