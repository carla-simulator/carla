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

  void UpdateSensorDataAndCheckPublish(uint64_t frame_id,
      std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
      carla::SharedBufferView buffer_view) {

      UpdateSensorData(sensor_header, buffer_view);
      sensor_data_update_frame_id.store(frame_id);
      if ( sensor_data_post_action_frame_id.load() >= frame_id ) {
        // camera sensors trigger their data streams from the rendering thread
        // therefore, the UpdateSensorDataPostAction() of the world publisher (running in the game thread) 
        // might have already been called for the current frame, which usually triggers the publishing of the sensor data.
        // In this case, we need to force a publish here to make sure the data gets published in a timely manner.
        log_verbose("Sensor Data to ROS data: frame.(", frame_id, ") stream.",
                    std::to_string(*std::static_pointer_cast<carla::ros2::types::SensorActorDefinition>(_actor_name_definition)), 
                    " Late publishing in CheckPublishAfterDataUpdate().");
        Publish();
      }
  }

  /**
   * calling UpdateSensorDataPostAction but store frame_id for later use
   */
  void UpdateSensorDataPostActionAndCheckPublish(uint64_t frame_id) {
    sensor_data_post_action_frame_id.store(frame_id);
    UpdateSensorDataPostAction();
    if (sensor_data_update_frame_id.load() >= frame_id) {
      // If the sensor data stream already updated the data for this frame, we publish in here
      // which is the standard for all UePublisher
      // If not, then either that UePublisher has nothing to publish this frame, or its stream
      // didn't yet update it's data. In both cases we don't need to publish now. 
      // In the later case publishing will be triggered in UpdateSensorDataAndCheckPublish() at a later point in time.
      log_verbose("Sensor Data to ROS data: frame.(", frame_id, ") stream.",
                  std::to_string(*std::static_pointer_cast<carla::ros2::types::SensorActorDefinition>(_actor_name_definition)),
                  " Standard publishing in UpdateSensorDataPostActionAndCheckPublish().");
      Publish();
    }
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
  std::atomic<uint64_t> sensor_data_update_frame_id{0u};
};
}  // namespace ros2
}  // namespace carla
