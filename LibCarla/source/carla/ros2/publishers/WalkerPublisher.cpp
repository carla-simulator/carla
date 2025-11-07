// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WalkerPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"
#include "carla/ros2/types/Speed.h"
#include "carla/ros2/types/WalkerControl.h"

namespace carla {
namespace ros2 {

WalkerPublisher::WalkerPublisher(std::shared_ptr<carla::ros2::types::WalkerActorDefinition> walker_actor_definition,
                                 std::shared_ptr<TransformPublisher> transform_publisher,
                                 std::shared_ptr<ObjectsPublisher> objects_publisher,
                                 std::shared_ptr<ObjectsWithCovariancePublisher> objects_with_covariance_publisher)
  : PublisherBaseTransform(std::static_pointer_cast<carla::ros2::types::ActorNameDefinition>(walker_actor_definition),
                           transform_publisher),
    _walker_object_publisher(std::make_shared<ObjectPublisher>(*this, objects_publisher)),
    _walker_object_with_covariance_publisher(std::make_shared<ObjectWithCovariancePublisher>(*this, objects_with_covariance_publisher)) {}

bool WalkerPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _walker_object_publisher->Init(domain_participant) &&
    _walker_object_with_covariance_publisher->Init(domain_participant);
}

bool WalkerPublisher::Publish() {
  return _walker_object_publisher->Publish() &&
    _walker_object_with_covariance_publisher->Publish();
}

bool WalkerPublisher::SubscribersConnected() const {
  return _walker_object_publisher->SubscribersConnected() ||
    _walker_object_with_covariance_publisher->SubscribersConnected();
}

void WalkerPublisher::UpdateWalker(std::shared_ptr<carla::ros2::types::Object> &object,
                                   carla::sensor::data::ActorDynamicState const &) {
  _walker_object_publisher->UpdateObject(object);
  _walker_object_with_covariance_publisher->UpdateObject(object);
}

}  // namespace ros2
}  // namespace carla
