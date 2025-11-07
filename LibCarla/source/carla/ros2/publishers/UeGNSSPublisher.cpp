// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeGNSSPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

UeGNSSPublisher::UeGNSSPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                                 std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBaseSensor(sensor_actor_definition, transform_publisher),
    _impl(std::make_shared<UeGNSSPublisherImpl>()) {}

bool UeGNSSPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(), get_topic_qos());
}

bool UeGNSSPublisher::Publish() {
  return _impl->Publish();
}
bool UeGNSSPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void UeGNSSPublisher::UpdateSensorData(
    std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
    carla::SharedBufferView buffer_view) {
  auto gnss_data = data(buffer_view);

  _impl->SetMessageHeader(GetTime(sensor_header), frame_id());
  _impl->Message().latitude(gnss_data.latitude);
  _impl->Message().longitude(gnss_data.longitude);
  _impl->Message().altitude(gnss_data.altitude);
}
}  // namespace ros2
}  // namespace carla
