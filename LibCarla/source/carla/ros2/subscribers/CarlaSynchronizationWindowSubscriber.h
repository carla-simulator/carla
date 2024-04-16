// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/subscribers/SubscriberBase.h"
#include "carla/rpc/RpcServerInterface.h"
#include "carla/rpc/ServerSynchronizationTypes.h"
#include "carla_msgs/msg/CarlaSynchronizationWindowPubSubTypes.h"

namespace carla {
namespace ros2 {

using CarlaSynchronizationWindowSubscriberImpl =
    DdsSubscriberImpl<carla_msgs::msg::CarlaSynchronizationWindow,
                      carla_msgs::msg::CarlaSynchronizationWindowPubSubType>;

class CarlaSynchronizationWindowSubscriber : public SubscriberBase<carla_msgs::msg::CarlaSynchronizationWindow> {
public:
  explicit CarlaSynchronizationWindowSubscriber(ROS2NameRecord &parent, carla::rpc::RpcServerInterface &carla_server);
  virtual ~CarlaSynchronizationWindowSubscriber();

  /**
   * Implements SubscriberBase::ProcessMessages()
   */
  void ProcessMessages() override;
  /**
   * Implements SubscriberBase::PublisherConnected()
   */
  void PublisherConnected(carla::rpc::synchronization_client_id_type const &publisher) override;

  /**
   * Implements SubscriberBase::PublisherDisconnected()
   */
  void PublisherDisconnected(carla::rpc::synchronization_client_id_type const &publisher) override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  std::shared_ptr<CarlaSynchronizationWindowSubscriberImpl> _impl;
  carla::rpc::RpcServerInterface &_carla_server;
  std::map<carla::rpc::synchronization_client_id_type, carla::rpc::synchronization_participant_id_type>
      _carla_synchronization_window_participants;
  carla::rpc::synchronization_target_game_time _carla_synchronization_target_game_time{
      carla::rpc::NO_SYNC_TARGET_GAME_TIME};
};
}  // namespace ros2
}  // namespace carla
