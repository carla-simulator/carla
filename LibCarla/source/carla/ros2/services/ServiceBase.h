// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include "carla/ros2/ROS2NameRecord.h"
#include "carla/ros2/ROS2QoS.h"
#include "carla/ros2/services/ServiceInterface.h"
#include "carla/rpc/RpcServerInterface.h"

namespace carla {
namespace ros2 {

/**
  A Service base class.
  */
template <typename REQUEST_TYPE, typename REQUEST_PUB_TYPE, typename RESPONSE_TYPE, typename RESPONSE_PUB_TYPE>
class DdsServiceImpl;

template <typename REQUEST_TYPE, typename RESPONSE_TYPE>
class ServiceBase : public ServiceInterface, public ROS2NameRecord {
public:
  ServiceBase(carla::rpc::RpcServerInterface &carla_server,
              std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition)
    : ROS2NameRecord(actor_name_definition), _carla_server(carla_server) {}
  virtual ~ServiceBase() = default;

  /**
   * Initialze the service
   */
  virtual bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) = 0;

protected:
  carla::rpc::RpcServerInterface &_carla_server;
};
}  // namespace ros2
}  // namespace carla
