// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficSignPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"


namespace carla {
namespace ros2 {

TrafficSignPublisher::TrafficSignPublisher(
    std::shared_ptr<carla::ros2::types::TrafficSignActorDefinition> traffic_sign_actor_definition,
    std::shared_ptr<ObjectsPublisher> objects_publisher)
  : PublisherBase(
        std::static_pointer_cast<carla::ros2::types::ActorNameDefinition>(traffic_sign_actor_definition))
#if PUBLISH_INDIVIDUAL_TRAFFIC_SIGN_DATA
    , _traffic_sign_object_publisher(std::make_shared<ObjectPublisher>(*this, objects_publisher))
#else
    , _traffic_sign_objects_publisher(objects_publisher)
#endif
  {}

bool TrafficSignPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  bool success = true;
#if PUBLISH_INDIVIDUAL_TRAFFIC_SIGN_DATA
  success &= _traffic_sign_object_publisher->Init(domain_participant);
#endif
  return success;
}

bool TrafficSignPublisher::Publish() {
  bool success = true;
#if PUBLISH_INDIVIDUAL_TRAFFIC_SIGN_DATA
  success &= _traffic_sign_object_publisher->Publish();
#endif
  return success;
}

bool TrafficSignPublisher::SubscribersConnected() const {
  bool connected = false;
#if PUBLISH_INDIVIDUAL_TRAFFIC_SIGN_DATA
  connected |= _traffic_sign_object_publisher->SubscribersConnected();
#endif
  return connected;
}

void TrafficSignPublisher::UpdateTrafficSign(std::shared_ptr<const carla::ros2::types::Object> &object,
                                             carla::sensor::data::ActorDynamicState const &) {
#if PUBLISH_INDIVIDUAL_TRAFFIC_SIGN_DATA
  _traffic_sign_object_publisher->UpdateObject(object);
#else
  _traffic_sign_objects_publisher->UpdateObject(object);
#endif
}

}  // namespace ros2
}  // namespace carla
