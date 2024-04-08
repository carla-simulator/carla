// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/PublisherBaseSensor.h"
#include "carla_msgs/msg/CarlaStatusPubSubTypes.h"

namespace carla {
namespace ros2 {

using CarlaStatusPublisherImpl = DdsPublisherImpl<carla_msgs::msg::CarlaStatus, carla_msgs::msg::CarlaStatusPubSubType>;

class CarlaStatusPublisher : public PublisherBaseSensor {
public:
  CarlaStatusPublisher();
  virtual ~CarlaStatusPublisher() = default;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubscribersConnected interface
   */
  bool SubscribersConnected() const override;

  void UpdateCarlaStatus(const carla_msgs::msg::CarlaStatus& status, const builtin_interfaces::msg::Time& stamp);

private:
  std::shared_ptr<CarlaStatusPublisherImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
