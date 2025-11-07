// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/services/DestroyObjectService.h"

#include "carla/ros2/impl/DdsServiceImpl.h"

namespace carla {
namespace ros2 {

DestroyObjectService::DestroyObjectService(
    carla::rpc::RpcServerInterface &carla_server,
    std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition)
  : ServiceBase(carla_server, actor_name_definition), _impl(std::make_shared<DestroyObjectServiceImpl>()) {}

bool DestroyObjectService::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  _impl->SetServiceCallback(std::bind(&DestroyObjectService::DestroyObject, this, std::placeholders::_1));
  return _impl->Init(domain_participant, get_topic_name());
}

void DestroyObjectService::CheckRequest() {
  _impl->CheckRequest();
}

carla_msgs::srv::DestroyObject_Response DestroyObjectService::DestroyObject(
    carla_msgs::srv::DestroyObject_Request const &request) {
  carla_msgs::srv::DestroyObject_Response response;
  response.success(_carla_server.call_destroy_actor(carla::streaming::detail::actor_id_type(request.id())));
  return response;
}

}  // namespace ros2
}  // namespace carla
