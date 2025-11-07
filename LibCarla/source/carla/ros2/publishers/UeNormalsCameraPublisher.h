// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/publishers/UePublisherBaseCamera.h"
#include "sensor_msgs/msg/CameraInfoPubSubTypes.h"
#include "sensor_msgs/msg/ImagePubSubTypes.h"

namespace carla {
namespace ros2 {

class UeNormalsCameraPublisher : public UePublisherBaseCamera<sensor_msgs::msg::ImageFromBuffer::allocator_type> {
public:
  UeNormalsCameraPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                           std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UeNormalsCameraPublisher() = default;
};
}  // namespace ros2
}  // namespace carla
