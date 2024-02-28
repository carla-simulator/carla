// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/SubscriberBase.h"
#include "carla_msgs/msg/CarlaVehicleControlPubSubTypes.h"

namespace carla {
namespace ros2 {

using VehicleControlSubscriberImpl =
    DdsSubscriberImpl<carla_msgs::msg::CarlaVehicleControl, carla_msgs::msg::CarlaVehicleControlPubSubType>;

class VehicleControlSubscriber : public SubscriberBase<carla_msgs::msg::CarlaVehicleControl> {
public:
  explicit VehicleControlSubscriber(ROS2NameRecord& parent);
  virtual ~VehicleControlSubscriber() = default;

  /**
   * Implements SubscriberInterface::IsAlive() interface
   */
  bool IsAlive() const override;
  /**
   * Implements SubscriberInterface::HasNewMessage() interface
   */
  bool HasNewMessage() const override;
  /**
   * Implements SubscriberInterface::GetMessage() interface
   */
  const carla_msgs::msg::CarlaVehicleControl& GetMessage() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  std::shared_ptr<VehicleControlSubscriberImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
