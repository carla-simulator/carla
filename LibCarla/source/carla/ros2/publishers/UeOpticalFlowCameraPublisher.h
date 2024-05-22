// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/UePublisherBaseCamera.h"

namespace carla {
namespace ros2 {

class UeOpticalFlowCameraPublisher : public UePublisherBaseCamera<sensor_msgs::msg::Image::allocator_type> {
public:
  UeOpticalFlowCameraPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                               std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UeOpticalFlowCameraPublisher() = default;

protected:
  /**
   * Overrides UePublisherBaseCamera::SetImageDataFromBuffer()
   */
  void SetImageDataFromBuffer(const carla::SharedBufferView buffer_view) override;
};
}  // namespace ros2
}  // namespace carla