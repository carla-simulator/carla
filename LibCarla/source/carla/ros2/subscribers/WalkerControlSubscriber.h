// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/SubscriberBase.h"
#include "carla_msgs/msg/CarlaWalkerControlPubSubTypes.h"

namespace carla {
namespace ros2 {

using WalkerControlSubscriberImpl =
    DdsSubscriberImpl<carla_msgs::msg::CarlaWalkerControl, carla_msgs::msg::CarlaWalkerControlPubSubType>;

class WalkerControlSubscriber : public SubscriberBase<carla_msgs::msg::CarlaWalkerControl> {
public:
  explicit WalkerControlSubscriber(ROS2NameRecord& parent);
  virtual ~WalkerControlSubscriber() = default;

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
  const carla_msgs::msg::CarlaWalkerControl& GetMessage() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  std::shared_ptr<WalkerControlSubscriberImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
