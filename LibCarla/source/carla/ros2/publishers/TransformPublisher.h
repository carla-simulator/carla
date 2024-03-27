// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/PublisherBase.h"

#include "carla/ros2/types/Transform.h"
#include "tf2_msgs/msg/TFMessagePubSubTypes.h"

namespace carla {
namespace ros2 {

using TransformPublisherImpl = DdsPublisherImpl<tf2_msgs::msg::TFMessage, tf2_msgs::msg::TFMessagePubSubType>;

class TransformPublisher : public PublisherBase {
public:
  TransformPublisher();
  virtual ~TransformPublisher() = default;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;
  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubsribersConnected interface
   */
  bool SubsribersConnected() const override;

  void AddTransform(const builtin_interfaces::msg::Time &stamp, std::string name, std::string parent,
                    geometry_msgs::msg::Transform const &transform);

private:
  std::shared_ptr<TransformPublisherImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
