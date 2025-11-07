// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ObjectsWithCovariancePublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

ObjectsWithCovariancePublisher::ObjectsWithCovariancePublisher()
  : PublisherBaseSensor(carla::ros2::types::ActorNameDefinition::CreateFromRoleName("objects_with_covariance")),
    _impl(std::make_shared<ObjectsWithCovariancePublisherImpl>()) {}

bool ObjectsWithCovariancePublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(), get_topic_qos());
}

bool ObjectsWithCovariancePublisher::Publish() {
  bool result = _impl->Publish();
  _impl->Message().objects().clear();
  return result;
}

bool ObjectsWithCovariancePublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void ObjectsWithCovariancePublisher::UpdateHeader(const builtin_interfaces::msg::Time &stamp) {
  _impl->SetMessageHeader(stamp, "map");
}

void ObjectsWithCovariancePublisher::AddObject(std::shared_ptr<carla::ros2::types::Object> &object) {
  derived_object_msgs::msg::ObjectWithCovariance ros_object_with_covariance = object->object_with_covariance();
  _impl->Message().objects().emplace_back(ros_object_with_covariance);
}

}  // namespace ros2
}  // namespace carla
