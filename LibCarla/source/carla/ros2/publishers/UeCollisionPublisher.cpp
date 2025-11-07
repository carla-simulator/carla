// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeCollisionPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"
#include "carla/ros2/types/CoordinateSystemTransform.h"
#include "carla/sensor/s11n/CollisionEventSerializer.h"

namespace carla {
namespace ros2 {

UeCollisionPublisher::UeCollisionPublisher(
    std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
    std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBaseSensor(sensor_actor_definition, transform_publisher),
    _impl(std::make_shared<UeCollisionPublisherImpl>()) {}

bool UeCollisionPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(), get_topic_qos());
}

bool UeCollisionPublisher::Publish() {
  return _impl->Publish();
}

bool UeCollisionPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void UeCollisionPublisher::UpdateSensorData(
    std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
    carla::SharedBufferView buffer_view) {
  auto collision_event_data = data(buffer_view);
  _impl->SetMessageHeader(GetTime(sensor_header), frame_id());
  _impl->Message().other_actor_id(collision_event_data.other_actor.id);
  _impl->Message().normal_impulse() =
      CoordinateSystemTransform::TransformLinearAxisMsg(collision_event_data.normal_impulse);
}
}  // namespace ros2
}  // namespace carla
