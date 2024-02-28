// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/PublisherBaseTransform.h"
#include "carla/rpc/ActorId.h"

namespace carla {
namespace ros2 {

/**
  A Publisher base class for sensors receiving their data directly from UE4 via buffers.
  Extends PublisherBaseTransform by UpdateSensorData() function.
  */
class UePublisherBaseSensor : public PublisherBaseTransform {
public:
  UePublisherBaseSensor(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                        std::shared_ptr<TransformPublisher> transform_publisher)
    : PublisherBaseTransform(sensor_actor_definition, transform_publisher) {}
  virtual ~UePublisherBaseSensor() = default;

  /**
   * Function to update the data for this sensor
   */
  virtual void UpdateSensorData(
      std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
      carla::SharedBufferView buffer_view) = 0;

  builtin_interfaces::msg::Time GetTime(
      std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header) const {
    return carla::ros2::types::Timestamp(sensor_header->timestamp).time();
  }

  std::shared_ptr<carla::ros2::types::SensorActorDefinition> GetSensorActorDefinition() const {
    return std::static_pointer_cast<carla::ros2::types::SensorActorDefinition>(_actor_name_definition);
  }
};
}  // namespace ros2
}  // namespace carla
