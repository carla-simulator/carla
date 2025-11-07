// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/ObjectsWithCovariancePublisher.h"
#include "carla/ros2/publishers/PublisherInterface.h"
#include "carla/ros2/types/Object.h"
#include "derived_object_msgs/msg/ObjectWithCovariancePubSubTypes.h"

namespace carla {
namespace ros2 {

using ObjectWithCovariancePublisherImpl =
    DdsPublisherImpl<derived_object_msgs::msg::ObjectWithCovariance, derived_object_msgs::msg::ObjectWithCovariancePubSubType>;

class ObjectWithCovariancePublisher : public PublisherInterface {
public:
  ObjectWithCovariancePublisher(ROS2NameRecord &parent_publisher, std::shared_ptr<ObjectsWithCovariancePublisher> objects_publisher);
  virtual ~ObjectWithCovariancePublisher() = default;

  /**
   * Implements Init() function
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant);

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubscribersConnected interface
   */
  bool SubscribersConnected() const override;

  void UpdateObject(std::shared_ptr<carla::ros2::types::Object> &object);

private:
  ROS2NameRecord &_parent_publisher;
  std::shared_ptr<ObjectWithCovariancePublisherImpl> _impl;
  std::shared_ptr<ObjectsWithCovariancePublisher> _objects_publisher;
};
}  // namespace ros2
}  // namespace carla
