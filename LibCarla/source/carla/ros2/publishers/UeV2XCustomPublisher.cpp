// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeV2XCustomPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

UeV2XCustomPublisher::UeV2XCustomPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                                 carla::ros2::types::V2XCustomSendCallback v2x_custom_send_callback,
                                 std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBaseSensor(sensor_actor_definition, transform_publisher),
    _subscriber(std::make_shared<UeV2XCustomSubscriber>(*this, v2x_custom_send_callback)),
    _impl(std::make_shared<UeV2XCustomPublisherImpl>()) {}

bool UeV2XCustomPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  _initialized = _impl->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(), get_topic_qos()) && _subscriber->Init(domain_participant);
  return _initialized;
}

bool UeV2XCustomPublisher::Publish() {
  return _impl->Publish();
}
bool UeV2XCustomPublisher::SubscribersConnected() const {
  return _impl->SubscribersConnected();
}

void UeV2XCustomPublisher::ProcessMessages() {
  if (!_initialized) {
    return;
  }
  _subscriber->ProcessMessages();
}

void UeV2XCustomPublisher::UpdateSensorData(
    std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
    carla::SharedBufferView buffer_view) {

  auto custom_v2x_data_vector = vector_view(buffer_view);

  if ( _impl->WasMessagePublished() ) {
    _impl->Message().data().clear();
  }

  for (carla::sensor::data::CustomV2XData const &custom_v2x_data: custom_v2x_data_vector) {
    carla_msgs::msg::CarlaV2XCustomData carla_v2x_custom_data;
    carla_v2x_custom_data.power(custom_v2x_data.Power);
    carla_msgs::msg::CarlaV2XCustomMessage carla_v2x_custom_message;
    carla_v2x_custom_message.header().protocol_version() = custom_v2x_data.Message.header.protocolVersion;
    carla_v2x_custom_message.header().message_id() = custom_v2x_data.Message.header.messageID;
    carla_v2x_custom_message.header().station_id().value() = custom_v2x_data.Message.header.stationID;
    carla_v2x_custom_message.data() = custom_v2x_data.Message.message;
    carla_v2x_custom_data.message() = carla_v2x_custom_message;
    _impl->Message().data().push_back(carla_v2x_custom_data);
  }
  _impl->SetMessageUpdated();
}

}  // namespace ros2
}  // namespace carla
