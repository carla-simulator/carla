// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>

#include "carla/ros2/publishers/PublisherBaseTransform.h"
#include "carla/ros2/types/SensorActorDefinition.h"
#include "carla/rpc/ActorId.h"


namespace carla {
namespace ros2 {

/**
  A Publisher base class for sensors receiving their data directly from UE4 via buffers.
  Extends PublisherBaseTransform by UpdateSensorData() function.
  Usually sensors are not moving in respect to their parent in the TF tree, so the transform is published as static and only updated if the sensor's position relatively to the parent changes.
 */
class UePublisherBase : public PublisherBaseTransform {
public:
  UePublisherBase(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                        std::shared_ptr<TransformPublisher> transform_publisher)
    : PublisherBaseTransform(sensor_actor_definition, transform_publisher, 
        TransformPublisher::TransformPublisherMode::MODE_STATIC) {}

  virtual ~UePublisherBase() = default;

  /**
   * Function to update the data for this sensor
   */
  virtual void UpdateSensorData(
      std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
      carla::SharedBufferView buffer_view) = 0;

  /**
   * calling UpdateSensorDataPostAction but store frame_id for later use
   */
  void UpdateSensorDataPostAction(uint64_t frame_id) {
    sensor_data_post_action_frame_id = frame_id;
    PublisherBase::UpdateSensorDataPostAction();
  }

  uint64_t GetSensorDataPostActionFrameId() const {
    return sensor_data_post_action_frame_id;
  }

  builtin_interfaces::msg::Time GetTime(
      std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header) const {
    return carla::ros2::types::Timestamp(sensor_header->timestamp).time();
  }

  std::shared_ptr<carla::ros2::types::SensorActorDefinition> GetSensorActorDefinition() const {
    return std::static_pointer_cast<carla::ros2::types::SensorActorDefinition>(_actor_name_definition);
  }

private:
  std::atomic<uint64_t> sensor_data_post_action_frame_id{0u};
  
};
}  // namespace ros2
}  // namespace carla
