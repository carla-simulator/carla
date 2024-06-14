// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/UeV2XCustomSubscriber.h"

#include "carla/ros2/impl/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

UeV2XCustomSubscriber::UeV2XCustomSubscriber(ROS2NameRecord& parent,
                                             carla::ros2::types::V2XCustomSendCallback v2x_custom_send_callback)
  : SubscriberBase(parent),
    _impl(std::make_shared<UeV2XCustomSubscriberImpl>(*this)),
    _v2x_custom_send_callback(v2x_custom_send_callback) {}

bool UeV2XCustomSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, get_topic_name("send"), get_topic_qos());
}

void UeV2XCustomSubscriber::ProcessMessages() {
  while (_impl->HasPublishersConnected() && _impl->HasNewMessage()) {
    std::string const message = _impl->GetMessage().data();
    _v2x_custom_send_callback(message);
  }
}

}  // namespace ros2
}  // namespace carla
