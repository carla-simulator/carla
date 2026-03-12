// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/UePublisherBaseCamera.h"
#include "carla/ros2/subscribers/ActorSetTransformSubscriber.h"
#include "carla/ros2/types/SensorActorDefinition.h"
#include "sensor_msgs/msg/CameraInfoPubSubTypes.h"
#include "sensor_msgs/msg/ImagePubSubTypes.h"

namespace carla {
namespace ros2 {

class UeRGBCameraPublisher : public UePublisherBaseCamera<sensor_msgs::msg::ImageFromBuffer::allocator_type> {
public:
  UeRGBCameraPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                       std::shared_ptr<TransformPublisher> transform_publisher,
                       carla::ros2::types::ActorSetTransformCallback actor_set_transform_callback);
  virtual ~UeRGBCameraPublisher() = default;

    /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Process incoming messages.
   */
  void ProcessMessages() override;

private:
  std::shared_ptr<ActorSetTransformSubscriber> actor_set_transform_subscriber;
  bool _initialized{false};
};
}  // namespace ros2
}  // namespace carla
