// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/SubscriberBase.h"
#include "carla/ros2/types/ActorDefinition.h"
#include "geometry_msgs/msg/PosePubSubTypes.h"

namespace carla {
namespace ros2 {

using ActorSetTransformSubscriberImpl =
    DdsSubscriberImpl<geometry_msgs::msg::Pose, geometry_msgs::msg::PosePubSubType>;

class ActorSetTransformSubscriber : public SubscriberBase<geometry_msgs::msg::Pose> {
public:
  explicit ActorSetTransformSubscriber(ROS2NameRecord& parent,
                                    carla::ros2::types::ActorSetTransformCallback actor_set_transform_callback);
  virtual ~ActorSetTransformSubscriber() = default;

  /**
   * Implements SubscriberBase::ProcessMessages()
   */
  void ProcessMessages() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  std::shared_ptr<ActorSetTransformSubscriberImpl> _impl;
  carla::ros2::types::ActorSetTransformCallback _actor_set_transform_callback;
};
}  // namespace ros2
}  // namespace carla
