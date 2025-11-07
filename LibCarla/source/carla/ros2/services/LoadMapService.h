// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/services/ServiceBase.h"
#include "carla_msgs/srv/LoadMapPubSubTypes.h"

namespace carla {
namespace ros2 {

using LoadMapServiceImpl =
    DdsServiceImpl<carla_msgs::srv::LoadMap_Request, carla_msgs::srv::LoadMap_RequestPubSubType,
                   carla_msgs::srv::LoadMap_Response, carla_msgs::srv::LoadMap_ResponsePubSubType>;

class LoadMapService
  : public ServiceBase<carla_msgs::srv::LoadMap_Request, carla_msgs::srv::LoadMap_Response> {
public:
  LoadMapService(carla::rpc::RpcServerInterface &carla_server,
                       std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition);
  virtual ~LoadMapService() = default;

  /**
   * Implements ServiceInterface::CheckRequest() interface
   */
  void CheckRequest() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  carla_msgs::srv::LoadMap_Response LoadMap(carla_msgs::srv::LoadMap_Request const &request);

  std::shared_ptr<LoadMapServiceImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
