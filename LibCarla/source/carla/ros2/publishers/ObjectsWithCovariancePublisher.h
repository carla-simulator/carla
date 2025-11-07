// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/PublisherBaseSensor.h"
#include "carla/ros2/types/Object.h"
#include "derived_object_msgs/msg/ObjectWithCovarianceArrayPubSubTypes.h"

namespace carla {
namespace ros2 {

using ObjectsWithCovariancePublisherImpl =
    DdsPublisherImpl<derived_object_msgs::msg::ObjectWithCovarianceArray, derived_object_msgs::msg::ObjectWithCovarianceArrayPubSubType>;

class ObjectsWithCovariancePublisher : public PublisherBaseSensor {
public:
  ObjectsWithCovariancePublisher();
  virtual ~ObjectsWithCovariancePublisher() = default;

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

  void UpdateHeader(const builtin_interfaces::msg::Time &stamp);

  void AddObject(std::shared_ptr<carla::ros2::types::Object> &object);

private:
  std::shared_ptr<ObjectsWithCovariancePublisherImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
