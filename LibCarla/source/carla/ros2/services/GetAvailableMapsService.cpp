// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/services/GetAvailableMapsService.h"

#include <algorithm>

#include "carla/actors/BlueprintLibrary.h"
#include "carla/ros2/impl/DdsServiceImpl.h"

namespace carla {
namespace ros2 {

GetAvailableMapsService::GetAvailableMapsService(
    carla::rpc::RpcServerInterface &carla_server,
    std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition)
  : ServiceBase(carla_server, actor_name_definition), _impl(std::make_shared<GetAvailableMapsServiceImpl>()) {}

bool GetAvailableMapsService::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  _impl->SetServiceCallback(std::bind(&GetAvailableMapsService::GetAvailableMaps, this, std::placeholders::_1));
  return _impl->Init(domain_participant, get_topic_name());
}

void GetAvailableMapsService::CheckRequest() {
  _impl->CheckRequest();
}

carla_msgs::srv::GetAvailableMaps_Response GetAvailableMapsService::GetAvailableMaps(
    carla_msgs::srv::GetAvailableMaps_Request const &request) {
  carla_msgs::srv::GetAvailableMaps_Response response;

  for ( auto const &map_name: _carla_server.call_get_available_maps().Get()) {
    response.maps().push_back(map_name);
  }
  return response;
}

}  // namespace ros2
}  // namespace carla
