// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/SubscriberBase.h"
#include "carla/ros2/types/SensorActorDefinition.h"
#include "carla_msgs/msg/CarlaV2XByteArrayPubSubTypes.h"

namespace carla {
namespace ros2 {

using UeV2XCustomSubscriberImpl =
    DdsSubscriberImpl<carla_msgs::msg::CarlaV2XByteArray, carla_msgs::msg::CarlaV2XByteArrayPubSubType>;

class UeV2XCustomSubscriber : public SubscriberBase<carla_msgs::msg::CarlaV2XByteArray> {
public:
  explicit UeV2XCustomSubscriber(ROS2NameRecord& parent,
                                 carla::ros2::types::V2XCustomSendCallback v2x_custom_send_callback);
  virtual ~UeV2XCustomSubscriber() = default;

  /**
   * Implements SubscriberBase::ProcessMessages()
   */
  void ProcessMessages() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  std::shared_ptr<UeV2XCustomSubscriberImpl> _impl;
  carla::ros2::types::V2XCustomSendCallback _v2x_custom_send_callback;
};
}  // namespace ros2
}  // namespace carla
