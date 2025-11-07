// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/services/ServiceBase.h"
#include "carla_msgs/srv/GetAvailableMapsPubSubTypes.h"

namespace carla {
namespace ros2 {

using GetAvailableMapsServiceImpl =
    DdsServiceImpl<carla_msgs::srv::GetAvailableMaps_Request, carla_msgs::srv::GetAvailableMaps_RequestPubSubType,
                   carla_msgs::srv::GetAvailableMaps_Response, carla_msgs::srv::GetAvailableMaps_ResponsePubSubType>;

class GetAvailableMapsService
  : public ServiceBase<carla_msgs::srv::GetAvailableMaps_Request, carla_msgs::srv::GetAvailableMaps_Response> {
public:
  GetAvailableMapsService(carla::rpc::RpcServerInterface &carla_server,
                       std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition);
  virtual ~GetAvailableMapsService() = default;

  /**
   * Implements ServiceInterface::CheckRequest() interface
   */
  void CheckRequest() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  carla_msgs::srv::GetAvailableMaps_Response GetAvailableMaps(carla_msgs::srv::GetAvailableMaps_Request const &request);

  std::shared_ptr<GetAvailableMapsServiceImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
