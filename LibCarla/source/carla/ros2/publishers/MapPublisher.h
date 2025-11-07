// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/publishers/PublisherBase.h"
#include "std_msgs/msg/StringPubSubTypes.h"

namespace carla {
namespace ros2 {

using MapPublisherImpl = DdsPublisherImpl<std_msgs::msg::String, std_msgs::msg::StringPubSubType>;

class MapPublisher : public PublisherBase {
public:
  MapPublisher();
  virtual ~MapPublisher() = default;

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

  void UpdateData(std::string const &data);

private:
  std::shared_ptr<MapPublisherImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
