// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "ackermann_msgs/msg/AckermannDriveStampedPubSubTypes.h"
#include "carla/ros2/subscribers/SubscriberBase.h"
#include "carla/ros2/types/VehicleActorDefinition.h"

namespace carla {
namespace ros2 {

using AckermannControlSubscriberImpl =
    DdsSubscriberImpl<ackermann_msgs::msg::AckermannDriveStamped, ackermann_msgs::msg::AckermannDriveStampedPubSubType>;

class AckermannControlSubscriber : public SubscriberBase<ackermann_msgs::msg::AckermannDriveStamped> {
public:
  explicit AckermannControlSubscriber(
      ROS2NameRecord& parent, carla::ros2::types::VehicleAckermannControlCallback vehicle_ackermann_control_callback);
  virtual ~AckermannControlSubscriber() = default;

  /**
   * Implements SubscriberBase::ProcessMessages()
   */
  void ProcessMessages() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  std::shared_ptr<AckermannControlSubscriberImpl> _impl;
  carla::ros2::types::VehicleAckermannControlCallback _vehicle_ackermann_control_callback;
};

}  // namespace ros2
}  // namespace carla
