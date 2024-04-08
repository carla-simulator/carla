// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficSignPublisher.h"

#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

TrafficSignPublisher::TrafficSignPublisher(
    std::shared_ptr<carla::ros2::types::TrafficSignActorDefinition> traffic_sign_actor_definition,
    std::shared_ptr<ObjectsPublisher> objects_publisher)
  : PublisherBaseSensor(
        std::static_pointer_cast<carla::ros2::types::ActorNameDefinition>(traffic_sign_actor_definition)),
    _traffic_sign_object_publisher(std::make_shared<ObjectPublisher>(*this, objects_publisher)) {}

bool TrafficSignPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _traffic_sign_object_publisher->Init(domain_participant);
}

bool TrafficSignPublisher::Publish() {
  return _traffic_sign_object_publisher->Publish();
}

bool TrafficSignPublisher::SubscribersConnected() const {
  return _traffic_sign_object_publisher->SubscribersConnected();
}

void TrafficSignPublisher::UpdateTrafficSign(std::shared_ptr<carla::ros2::types::Object> &object,
                                             carla::sensor::data::ActorDynamicState const &) {
  _traffic_sign_object_publisher->UpdateObject(object);
}

}  // namespace ros2
}  // namespace carla
