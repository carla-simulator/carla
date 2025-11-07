// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/ActorSetTransformSubscriber.h"

#include "carla/ros2/impl/DdsSubscriberImpl.h"

namespace carla {
namespace ros2 {

ActorSetTransformSubscriber::ActorSetTransformSubscriber(ROS2NameRecord& parent,
                                                   carla::ros2::types::ActorSetTransformCallback actor_set_transform_callback)
  : SubscriberBase(parent),
    _impl(std::make_shared<ActorSetTransformSubscriberImpl>(*this)),
    _actor_set_transform_callback(actor_set_transform_callback) {}

bool ActorSetTransformSubscriber::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->Init(domain_participant, get_topic_name("control/set_transform"), get_topic_qos());
}

void ActorSetTransformSubscriber::ProcessMessages() {
  while (_impl->HasPublishersConnected() && _impl->HasNewMessage()) {
    _actor_set_transform_callback(carla::ros2::types::Transform(_impl->GetMessage()));
  }
}

}  // namespace ros2
}  // namespace carla
