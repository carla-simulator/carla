// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/publishers/PublisherBase.h"
#include "carla/rpc/RpcServerInterface.h"
#include "carla_msgs/msg/CarlaWorldInfoPubSubTypes.h"

namespace carla {
namespace ros2 {

using WorldInfoPublisherImpl = DdsPublisherImpl<carla_msgs::msg::CarlaWorldInfo, carla_msgs::msg::CarlaWorldInfoPubSubType>;

class WorldInfoPublisher : public PublisherBase {
public:
  WorldInfoPublisher(carla::rpc::RpcServerInterface &carla_server);
  virtual ~WorldInfoPublisher() = default;

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

  /**
   * Query the not streamed data from the server before processing the sensor data.
   */
  void UpdateSensorDataPreAction() override;
  
  /**
   * Indicate that the map has updated and the server should be quieried for map updates.
   */
  void SetMapUpdated() { _map_updated=true; } 

private:
  std::shared_ptr<WorldInfoPublisherImpl> _impl;
  bool _map_updated=false;
  carla::rpc::RpcServerInterface &_carla_server;
};
}  // namespace ros2
}  // namespace carla
