// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ObjectsPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"
#include <algorithm>

namespace carla {
namespace ros2 {

ObjectsPublisher::ObjectsPublisher(ObjectsPublisher::ObjectMode const update_mode, std::string role_name)
  : PublisherBaseSensor(carla::ros2::types::ActorNameDefinition::CreateFromRoleName(role_name))
  , _impl(std::make_shared<ObjectsPublisherImpl>())
  , _update_mode(update_mode)
{
  _impl->Message().header().frame_id("map");
}

bool ObjectsPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(), get_topic_qos());
}

bool ObjectsPublisher::Publish() {
  bool result = _impl->Publish();
  if (_update_mode == ObjectsPublisher::ObjectMode::DYNAMIC_PUBLISH_ALWAYS) {
      _impl->Message().objects().clear();
  }
  return result;
}

bool ObjectsPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void ObjectsPublisher::UpdateHeader(const builtin_interfaces::msg::Time &stamp) {
  _impl->Message().header().stamp(stamp);
  if ((_update_mode == ObjectsPublisher::ObjectMode::DYNAMIC_PUBLISH_ALWAYS)
     || (_update_mode == ObjectsPublisher::ObjectMode::STATIC_PUBLISH_ONCE)) {
    _impl->SetMessageUpdated();
  }
}

void ObjectsPublisher::UpdateObject(std::shared_ptr<const carla::ros2::types::Object> &object) {
  auto find_res = std::find_if(_impl->Message().objects().begin(), _impl->Message().objects().end(), 
     [object](derived_object_msgs::msg::Object &ros_object){ return ros_object.id()==object->actor_id(); });
  if (find_res != _impl->Message().objects().end()) {
    if ( object->has_dynamic_data_changed(*find_res) )
    {
      derived_object_msgs::msg::Object const ros_object = object->object();
      *find_res=ros_object;
      _impl->SetMessageUpdated();
    }
  }
}

void ObjectsPublisher::AddObject(carla::ros2::types::Object const &object) {
  derived_object_msgs::msg::Object ros_object = object.object();
  _impl->Message().objects().emplace_back(ros_object);
}

void ObjectsPublisher::RemoveObject(uint64_t const object_id) {
  std::remove_if(_impl->Message().objects().begin(), _impl->Message().objects().end(), 
     [object_id](derived_object_msgs::msg::Object &ros_object){ return ros_object.id()==object_id; });
}

}  // namespace ros2
}  // namespace carla
