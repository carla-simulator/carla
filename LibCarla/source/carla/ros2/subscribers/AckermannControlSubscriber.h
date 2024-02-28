// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "ackermann_msgs/msg/AckermannDriveStampedPubSubTypes.h"
#include "carla/ros2/subscribers/SubscriberBase.h"

namespace carla {
namespace ros2 {

using AckermannControlSubscriberImpl =
    DdsSubscriberImpl<ackermann_msgs::msg::AckermannDriveStamped, ackermann_msgs::msg::AckermannDriveStampedPubSubType>;

class AckermannControlSubscriber : public SubscriberBase<ackermann_msgs::msg::AckermannDriveStamped> {
public:
  explicit AckermannControlSubscriber(ROS2NameRecord& parent);
  virtual ~AckermannControlSubscriber() = default;

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
  const ackermann_msgs::msg::AckermannDriveStamped& GetMessage() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  std::shared_ptr<AckermannControlSubscriberImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
