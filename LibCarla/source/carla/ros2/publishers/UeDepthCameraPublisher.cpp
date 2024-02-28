// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeDepthCameraPublisher.h"

#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

UeDepthCameraPublisher::UeDepthCameraPublisher(
    std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
    std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBaseCamera(sensor_actor_definition, transform_publisher) {}
}  // namespace ros2
}  // namespace carla
