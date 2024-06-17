// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/publishers/UePublisherBaseSensor.h"
#include "carla/ros2/subscribers/UeV2XCustomSubscriber.h"
#include "carla/sensor/data/V2XEvent.h"
#include "carla_msgs/msg/CarlaV2XCustomDataListPubSubTypes.h"

namespace carla {
namespace ros2 {

using UeV2XCustomPublisherImpl = DdsPublisherImpl<carla_msgs::msg::CarlaV2XCustomDataList, carla_msgs::msg::CarlaV2XCustomDataListPubSubType>;

class UeV2XCustomPublisher : public UePublisherBaseSensor {
public:
  UeV2XCustomPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                  carla::ros2::types::V2XCustomSendCallback v2x_custom_send_callback,
                  std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UeV2XCustomPublisher() = default;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubscribersConnected interface
   */
  bool SubscribersConnected() const override;

  /**
   * Process incoming messages
   */
  void ProcessMessages() override;

  /**
   * Implements UePublisherBaseSensor::UpdateSensorData() interface
   */
  void UpdateSensorData(std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
                        const carla::SharedBufferView buffer_view) override;

private:
  using CustomV2XData = carla::sensor::data::CustomV2XData;
  using CustomV2XDataVectorAllocator = carla::sensor::data::SerializerVectorAllocator<CustomV2XData>;

  std::vector<CustomV2XData, CustomV2XDataVectorAllocator> vector_view(const carla::SharedBufferView buffer_view) {
    return carla::sensor::data::buffer_data_accessed_by_vector<CustomV2XData>(
        buffer_view, 0);
  }


  bool _initialized{false};
  std::shared_ptr<UeV2XCustomSubscriber> _subscriber;
  std::shared_ptr<UeV2XCustomPublisherImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
