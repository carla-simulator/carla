// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/UePublisherBaseSensor.h"
#include "carla/sensor/s11n/IMUSerializer.h"
#include "geometry_msgs/msg/Accel.h"
#include "sensor_msgs/msg/ImuPubSubTypes.h"

namespace carla {
namespace ros2 {

using UeIMUPublisherImpl = DdsPublisherImpl<sensor_msgs::msg::Imu, sensor_msgs::msg::ImuPubSubType>;

class UeIMUPublisher : public UePublisherBaseSensor {
public:
  UeIMUPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                 std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UeIMUPublisher() = default;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubsribersConnected interface
   */
  bool SubsribersConnected() const override;

  /**
   * Implements UePublisherBaseSensor::UpdateSensorData() interface
   */
  void UpdateSensorData(std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
                        const carla::SharedBufferView buffer_view) override;

private:
  using IMUDataConst = carla::sensor::s11n::IMUSerializer::Data const;

  std::shared_ptr<IMUDataConst> data_view_ptr(carla::SharedBufferView buffer_view) {
    return std::shared_ptr<IMUDataConst>(buffer_view, reinterpret_cast<IMUDataConst*>(buffer_view.get()));
  }

  std::shared_ptr<UeIMUPublisherImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
