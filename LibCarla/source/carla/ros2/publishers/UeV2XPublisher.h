// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/publishers/UePublisherBaseSensor.h"
#include "carla/sensor/data/V2XEvent.h"
#include "carla_msgs/msg/CarlaV2XDataListPubSubTypes.h"

namespace carla {
namespace ros2 {

using UeV2XPublisherImpl = DdsPublisherImpl<carla_msgs::msg::CarlaV2XDataList, carla_msgs::msg::CarlaV2XDataListPubSubType>;

class UeV2XPublisher : public UePublisherBaseSensor {
public:
  UeV2XPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                  std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UeV2XPublisher() = default;

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
   * Implements UePublisherBaseSensor::UpdateSensorData() interface
   */
  void UpdateSensorData(std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
                        const carla::SharedBufferView buffer_view) override;

private:
  using CAMData = carla::sensor::data::CAMData;
  using CAMDataVectorAllocator = carla::sensor::data::SerializerVectorAllocator<CAMData>;

  std::vector<CAMData, CAMDataVectorAllocator> vector_view(const carla::SharedBufferView buffer_view) {
    return carla::sensor::data::buffer_data_accessed_by_vector<CAMData>(
        buffer_view, 0);
  }


  bool _initialized{false};
  std::shared_ptr<UeV2XPublisherImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
