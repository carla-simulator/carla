// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/services/ServiceBase.h"
#include "carla_msgs/srv/SpawnObjectPubSubTypes.h"

namespace carla {
namespace ros2 {

using SpawnObjectServiceImpl =
    DdsServiceImpl<carla_msgs::srv::SpawnObject_Request, carla_msgs::srv::SpawnObject_RequestPubSubType,
                   carla_msgs::srv::SpawnObject_Response, carla_msgs::srv::SpawnObject_ResponsePubSubType>;

class SpawnObjectService
  : public ServiceBase<carla_msgs::srv::SpawnObject_Request, carla_msgs::srv::SpawnObject_Response> {
public:
  SpawnObjectService(ROS2ServerInterface &carla_server,
                     std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition);
  virtual ~SpawnObjectService() = default;

  /**
   * Implements ServiceInterface::CheckRequest() interface
   */
  void CheckRequest() override;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

private:
  carla_msgs::srv::SpawnObject_Response SpawnObject(carla_msgs::srv::SpawnObject_Request const &request);

  std::shared_ptr<SpawnObjectServiceImpl> _impl;
};
}  // namespace ros2
}  // namespace carla
