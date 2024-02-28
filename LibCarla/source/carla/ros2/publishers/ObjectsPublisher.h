// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/PublisherBaseSensor.h"
#include "carla/ros2/types/Object.h"
#include "derived_object_msgs/msg/ObjectArrayPubSubTypes.h"

namespace carla {
namespace ros2 {

using ObjectsPublisherImpl =
    DdsPublisherImpl<derived_object_msgs::msg::ObjectArray, derived_object_msgs::msg::ObjectArrayPubSubType>;

class ObjectsPublisher : public PublisherBaseSensor {
public:
  ObjectsPublisher();
  virtual ~ObjectsPublisher() = default;

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

  void AddObject(std::shared_ptr<carla::ros2::types::Object> &object);

private:
  std::shared_ptr<ObjectsPublisherImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
