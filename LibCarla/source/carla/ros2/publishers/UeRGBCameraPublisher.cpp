// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeRGBCameraPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

UeRGBCameraPublisher::UeRGBCameraPublisher(
    std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
    std::shared_ptr<TransformPublisher> transform_publisher,
    carla::ros2::types::ActorSetTransformCallback actor_set_transform_callback)
  : UePublisherBaseCamera(sensor_actor_definition, transform_publisher)
  , actor_set_transform_subscriber(std::make_shared<ActorSetTransformSubscriber>(
          *this, actor_set_transform_callback))
  {}

bool UeRGBCameraPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  _initialized = UePublisherBaseCamera::Init(domain_participant);
  _initialized &= actor_set_transform_subscriber->Init(domain_participant);
  return _initialized;
}

void UeRGBCameraPublisher::ProcessMessages() {
  if (!_initialized) {
    return;
  }
  actor_set_transform_subscriber->ProcessMessages();
}





}  // namespace ros2
}  // namespace carla
