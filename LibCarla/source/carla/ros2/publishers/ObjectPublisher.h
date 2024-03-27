// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/ObjectsPublisher.h"
#include "carla/ros2/publishers/PublisherInterface.h"
#include "carla/ros2/types/Object.h"
#include "derived_object_msgs/msg/ObjectPubSubTypes.h"

namespace carla {
namespace ros2 {

using ObjectPublisherImpl =
    DdsPublisherImpl<derived_object_msgs::msg::Object, derived_object_msgs::msg::ObjectPubSubType>;

class ObjectPublisher : public PublisherInterface {
public:
  ObjectPublisher(ROS2NameRecord &parent_publisher, std::shared_ptr<ObjectsPublisher> objects_publisher);
  virtual ~ObjectPublisher() = default;

  /**
   * Implements Init() function
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant);

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubsribersConnected interface
   */
  bool SubsribersConnected() const override;

  void UpdateObject(std::shared_ptr<carla::ros2::types::Object> &object);

private:
  ROS2NameRecord &_parent_publisher;
  std::shared_ptr<ObjectPublisherImpl> _impl;
  std::shared_ptr<ObjectsPublisher> _objects_publisher;
};
}  // namespace ros2
}  // namespace carla
