// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/SubscriberBase.h"
#include "carla/ros2/types/VehicleActorDefinition.h"
#include "carla_msgs/msg/CarlaVehicleControlPubSubTypes.h"

namespace carla {
namespace ros2 {

using VehicleControlSubscriberImpl =
    DdsSubscriberImpl<carla_msgs::msg::CarlaVehicleControl, carla_msgs::msg::CarlaVehicleControlPubSubType>;

class VehicleControlSubscriber : public SubscriberBase<carla_msgs::msg::CarlaVehicleControl> {
public:
  explicit VehicleControlSubscriber(ROS2NameRecord& parent,
                                    carla::ros2::types::VehicleControlCallback vehicle_control_callback);
  virtual ~VehicleControlSubscriber() = default;

  /**
   * Implements SubscriberBase::ProcessMessages()
   */
  void ProcessMessages() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  std::shared_ptr<VehicleControlSubscriberImpl> _impl;
  carla::ros2::types::VehicleControlCallback _vehicle_control_callback;
};
}  // namespace ros2
}  // namespace carla
