// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficLightPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

TrafficLightPublisher::TrafficLightPublisher(
    std::shared_ptr<carla::ros2::types::TrafficLightActorDefinition> traffic_light_actor_definition,
    std::shared_ptr<ObjectsPublisher> objects_publisher,
    std::shared_ptr<TrafficLightsPublisher> traffic_lights_publisher)
  : PublisherBase(
        std::static_pointer_cast<carla::ros2::types::ActorNameDefinition>(traffic_light_actor_definition))
#if PUBLISH_INDIVIDUAL_TRAFFIC_LIGHT_DATA
    , _traffic_light_info_publisher(std::make_shared<TrafficLightInfoPublisherImpl>())
    , _traffic_light_status_publisher(std::make_shared<TrafficLightStatusPublisherImpl>())
    , _traffic_light_object_publisher(std::make_shared<ObjectPublisher>(*this, objects_publisher))
#else
    , _traffic_light_objects_publisher(objects_publisher)
#endif
    , _traffic_lights_publisher(traffic_lights_publisher) {
   _traffic_light_status.header().frame_id("map");
   _traffic_light_status.state(carla_msgs::msg::CarlaTrafficLightStatus_Constants::UNKNOWN);
}

bool TrafficLightPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  bool success = true;
#if PUBLISH_INDIVIDUAL_TRAFFIC_LIGHT_DATA
 success &= _traffic_light_info_publisher->Init(domain_participant, get_topic_name("traffic_light_info"),
                                   PublisherBase::get_topic_qos());
success &= _traffic_light_status_publisher->Init(domain_participant, get_topic_name("traffic_light_status"),
                                     PublisherBase::get_topic_qos());  
success &= _traffic_light_object_publisher->Init(domain_participant);
#endif
  return success;
}

bool TrafficLightPublisher::Publish() {
  bool success = true;
#if PUBLISH_INDIVIDUAL_TRAFFIC_LIGHT_DATA
  success &= _traffic_light_info_publisher->Publish();
  success &= _traffic_light_status_publisher->Publish();
  success &= _traffic_light_object_publisher->Publish();
#endif
  return success;
}

bool TrafficLightPublisher::SubscribersConnected() const {
  bool connected = false;
#if PUBLISH_INDIVIDUAL_TRAFFIC_LIGHT_DATA
  connected |= _traffic_light_info_publisher->SubscribersConnected();
  connected |= _traffic_light_status_publisher->SubscribersConnected();
  connected |= _traffic_light_object_publisher->SubscribersConnected();
#endif
  return connected;
}

void TrafficLightPublisher::UpdateTrafficLight(std::shared_ptr<const carla::ros2::types::Object> &object,
                                               carla::sensor::data::ActorDynamicState const &actor_dynamic_state) {
  if ( (!_traffic_light_info_initialized) || (_traffic_light_info.transform() != object->Transform().pose())) {
    _traffic_light_info_initialized = true;
    _traffic_light_info.id(object->actor_id());
    _traffic_light_info.transform(object->Transform().pose());
    // trigger volume
    auto traffic_light_actor_definition = std::dynamic_pointer_cast<carla::ros2::types::TrafficLightActorDefinition>(_actor_name_definition);
    if (traffic_light_actor_definition!=nullptr)
    {
      auto global_location = traffic_light_actor_definition->trigger_volume.location;
      object->Transform().TransformPoint(global_location);
      _traffic_light_info.trigger_volume().center().x(global_location.x);
      _traffic_light_info.trigger_volume().center().y(global_location.y);
      _traffic_light_info.trigger_volume().center().z(global_location.z);
      auto const ros_extent = traffic_light_actor_definition->trigger_volume.extent * 2.;
      _traffic_light_info.trigger_volume().size().x(ros_extent.x);
      _traffic_light_info.trigger_volume().size().y(ros_extent.y);
      _traffic_light_info.trigger_volume().size().z(ros_extent.z);
    }
    else
    {
      log_error("TrafficLightPublisher::UpdateTrafficLight(", std::to_string(*_actor_name_definition), 
              ") actor definition should be of type carla::ros2::types::TrafficLightActorDefinition");
    }
#if PUBLISH_INDIVIDUAL_TRAFFIC_LIGHT_DATA
    _traffic_light_info_publisher->Message()=_traffic_light_info;
    _traffic_light_info_publisher->SetMessageUpdated();
#endif
    _traffic_lights_publisher->UpdateTrafficLightInfo(_traffic_light_info);
  }

  if (_traffic_light_status.state() != carla::ros2::types::GetTrafficLightState(actor_dynamic_state)) {
    _traffic_light_status.id(_traffic_light_info.id());
    _traffic_light_status.state(carla::ros2::types::GetTrafficLightState(actor_dynamic_state));
    _traffic_light_status.header().stamp(object->Timestamp().time());
#if PUBLISH_INDIVIDUAL_TRAFFIC_LIGHT_DATA
    _traffic_light_status_publisher->Message()=_traffic_light_status;
    _traffic_light_status_publisher->SetMessageUpdated();
#endif
    _traffic_lights_publisher->UpdateTrafficLightStatus(_traffic_light_status);
  }
#if PUBLISH_INDIVIDUAL_TRAFFIC_LIGHT_DATA
  _traffic_light_object_publisher->UpdateObject(object);
#else
  _traffic_light_objects_publisher->UpdateObject(object);
#endif
}

}  // namespace ros2
}  // namespace carla
