// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/publishers/UePublisherBaseCamera.h"

namespace carla {
namespace ros2 {

class UeISCameraPublisher : public UePublisherBaseCamera<sensor_msgs::msg::ImageFromBuffer::allocator_type> {
public:
  UeISCameraPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                      std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UeISCameraPublisher() = default;
};
}  // namespace ros2
}  // namespace carla
