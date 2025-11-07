// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/SubscriberBaseSynchronizationClient.h"
#include "carla/rpc/RpcServerInterface.h"
#include "carla_msgs/msg/CarlaControlPubSubTypes.h"

namespace carla {
namespace ros2 {

using CarlaControlSubscriberImpl =
    DdsSubscriberImpl<carla_msgs::msg::CarlaControl, carla_msgs::msg::CarlaControlPubSubType>;

class CarlaControlSubscriber : public SubscriberBaseSynchronizationClient<carla_msgs::msg::CarlaControl> {
public:
  explicit CarlaControlSubscriber(ROS2NameRecord &parent, carla::rpc::RpcServerInterface &carla_server);
  virtual ~CarlaControlSubscriber();

  /**
   * Implements SubscriberBase::ProcessMessages()
   */
  void ProcessMessages() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  /**
   * Implements SubscriberBaseSynchronizationClient::ThisAsSynchronizationClient() interface
   */
  carla::rpc::synchronization_client_id_type ThisAsSynchronizationClient() override {
    return get_topic_name("control/carla_control");
  }

  std::shared_ptr<CarlaControlSubscriberImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
