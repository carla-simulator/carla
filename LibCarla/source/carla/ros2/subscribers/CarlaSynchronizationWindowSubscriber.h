// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/SubscriberBaseSynchronizationClient.h"
#include "carla/rpc/RpcServerInterface.h"
#include "carla/rpc/ServerSynchronizationTypes.h"
#include "carla_msgs/msg/CarlaSynchronizationWindowPubSubTypes.h"

namespace carla {
namespace ros2 {

using CarlaSynchronizationWindowSubscriberImpl =
    DdsSubscriberImpl<carla_msgs::msg::CarlaSynchronizationWindow,
                      carla_msgs::msg::CarlaSynchronizationWindowPubSubType>;

class CarlaSynchronizationWindowSubscriber : public SubscriberBaseSynchronizationClient<carla_msgs::msg::CarlaSynchronizationWindow> {
public:
  explicit CarlaSynchronizationWindowSubscriber(ROS2NameRecord &parent, carla::rpc::RpcServerInterface &carla_server);
  virtual ~CarlaSynchronizationWindowSubscriber();

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
    return get_topic_name("control/synchronization_window");
  }

  std::shared_ptr<CarlaSynchronizationWindowSubscriberImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
