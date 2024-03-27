// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/UePublisherBaseSensor.h"
#include "carla/sensor/s11n/CollisionEventSerializer.h"
#include "carla_msgs/msg/CarlaCollisionEventPubSubTypes.h"

namespace carla {
namespace ros2 {

using UeCollisionPublisherImpl =
    DdsPublisherImpl<carla_msgs::msg::CarlaCollisionEvent, carla_msgs::msg::CarlaCollisionEventPubSubType>;

class UeCollisionPublisher : public UePublisherBaseSensor {
public:
  UeCollisionPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                       std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UeCollisionPublisher() = default;

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
                        carla::SharedBufferView buffer_view) override;

private:
  using CollisionEventDataConst = carla::sensor::s11n::CollisionEventSerializer::Data const;

  std::shared_ptr<CollisionEventDataConst> data_view_ptr(carla::SharedBufferView buffer_view) {
    return std::shared_ptr<CollisionEventDataConst>(
        buffer_view, reinterpret_cast<CollisionEventDataConst*>(buffer_view.get()->data()));
  }

  std::shared_ptr<UeCollisionPublisherImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
