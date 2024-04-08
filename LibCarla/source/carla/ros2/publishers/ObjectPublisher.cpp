// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ObjectPublisher.h"

#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

ObjectPublisher::ObjectPublisher(ROS2NameRecord &parent_publisher, std::shared_ptr<ObjectsPublisher> objects_publisher)
  : _parent_publisher(parent_publisher),
    _impl(std::make_shared<ObjectPublisherImpl>()),
    _objects_publisher(objects_publisher) {}

bool ObjectPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(
      domain_participant, _parent_publisher.get_topic_name("object"), DEFAULT_SENSOR_DATA_QOS);
}

bool ObjectPublisher::Publish() {
  return _impl->Publish();
}

bool ObjectPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void ObjectPublisher::UpdateObject(std::shared_ptr<carla::ros2::types::Object> &object) {
  // forward the data to the objects publisher
  _objects_publisher->AddObject(object);
  derived_object_msgs::msg::Object ros_object = object->object();
  _impl->Message() = ros_object;
  _impl->SetMessageHeader(ros_object.header().stamp(), _parent_publisher.frame_id());
}

}  // namespace ros2
}  // namespace carla
