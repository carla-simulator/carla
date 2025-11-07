// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ObjectWithCovariancePublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

ObjectWithCovariancePublisher::ObjectWithCovariancePublisher(ROS2NameRecord &parent_publisher, std::shared_ptr<ObjectsWithCovariancePublisher> objects_publisher)
  : _parent_publisher(parent_publisher),
    _impl(std::make_shared<ObjectWithCovariancePublisherImpl>()),
    _objects_publisher(objects_publisher) {}

bool ObjectWithCovariancePublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(
      domain_participant, _parent_publisher.get_topic_name("object_with_covariance"), DEFAULT_SENSOR_DATA_QOS);
}

bool ObjectWithCovariancePublisher::Publish() {
  return _impl->Publish();
}

bool ObjectWithCovariancePublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void ObjectWithCovariancePublisher::UpdateObject(std::shared_ptr<carla::ros2::types::Object> &object) {
  // forward the data to the objects publisher
  _objects_publisher->AddObject(object);
  derived_object_msgs::msg::ObjectWithCovariance ros_object_with_covariance = object->object_with_covariance();
  _impl->Message() = ros_object_with_covariance;
  _impl->SetMessageHeader(ros_object_with_covariance.header().stamp(), _parent_publisher.frame_id());
}

}  // namespace ros2
}  // namespace carla
