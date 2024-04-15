// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ObjectsPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

ObjectsPublisher::ObjectsPublisher()
  : PublisherBaseSensor(carla::ros2::types::ActorNameDefinition::CreateFromRoleName("objects")),
    _impl(std::make_shared<ObjectsPublisherImpl>()) {}

bool ObjectsPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(), get_topic_qos());
}

bool ObjectsPublisher::Publish() {
  bool result = _impl->Publish();
  _impl->Message().objects().clear();
  return result;
}

bool ObjectsPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void ObjectsPublisher::AddObject(std::shared_ptr<carla::ros2::types::Object> &object) {
  derived_object_msgs::msg::Object ros_object = object->object();
  if (_impl->Message().objects().empty()) {
    _impl->SetMessageHeader(ros_object.header().stamp(), "map");
  }
  _impl->Message().objects().emplace_back(ros_object);
}

}  // namespace ros2
}  // namespace carla
