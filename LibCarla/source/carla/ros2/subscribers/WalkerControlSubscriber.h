// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/SubscriberBase.h"
#include "carla/ros2/types/WalkerActorDefinition.h"
#include "carla_msgs/msg/CarlaWalkerControlPubSubTypes.h"

namespace carla {
namespace ros2 {

using WalkerControlSubscriberImpl =
    DdsSubscriberImpl<carla_msgs::msg::CarlaWalkerControl, carla_msgs::msg::CarlaWalkerControlPubSubType>;

class WalkerControlSubscriber : public SubscriberBase<carla_msgs::msg::CarlaWalkerControl> {
public:
  explicit WalkerControlSubscriber(ROS2NameRecord& parent,
                                   carla::ros2::types::WalkerControlCallback walker_control_callback);
  virtual ~WalkerControlSubscriber() = default;

  /**
   * Implements SubscriberBase::ProcessMessages()
   */
  void ProcessMessages() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  std::shared_ptr<WalkerControlSubscriberImpl> _impl;
  carla::ros2::types::WalkerControlCallback _walker_control_callback;
};
}  // namespace ros2
}  // namespace carla
