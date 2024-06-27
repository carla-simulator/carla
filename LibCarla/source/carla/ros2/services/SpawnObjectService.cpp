// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/services/SpawnObjectService.h"

#include <algorithm>
#include <random>

#include "carla/actors/BlueprintLibrary.h"
#include "carla/ros2/impl/DdsServiceImpl.h"
#include "carla/ros2/types/Transform.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/disable-ue4-macros.h>
#include <compiler/enable-ue4-macros.h>
#include "Carla/Server/CarlaServer.h"
#endif

namespace carla {
namespace ros2 {

SpawnObjectService::SpawnObjectService(carla::rpc::RpcServerInterface &carla_server,
                                       std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition)
  : ServiceBase(carla_server, actor_name_definition), _impl(std::make_shared<SpawnObjectServiceImpl>()) {}

bool SpawnObjectService::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  _impl->SetServiceCallback(std::bind(&SpawnObjectService::SpawnObject, this, std::placeholders::_1));
  return _impl->Init(domain_participant, get_topic_name());
}

void SpawnObjectService::CheckRequest() {
  _impl->CheckRequest();
}

carla_msgs::srv::SpawnObject_Response SpawnObjectService::SpawnObject(
    carla_msgs::srv::SpawnObject_Request const &request) {
  carla_msgs::srv::SpawnObject_Response response;

  carla::geom::Transform transform;
  if (request.random_pose()) {
    std::vector<geom::Transform> spawn_points;
    auto map_info = _carla_server.call_get_map_info();
    std::vector<geom::Transform> result;
    std::sample(map_info.Get().recommended_spawn_points.begin(), map_info.Get().recommended_spawn_points.end(),
                std::back_inserter(result), 1, std::mt19937{std::random_device{}()});
    if (result.empty()) {
      response.error_string("SpawnObjectService: failed to retrieve random spawn point");
      response.id(0);
      return response;
    }
    transform = *result.begin();
  } else {
    carla::ros2::types::Transform ros_transform(request.transform());
    transform = ros_transform.GetTransform();
  }

  auto blueprints =
      carla::actors::BlueprintLibrary(_carla_server.call_get_actor_definitions().Get()).Filter(request.type());
  if (blueprints->empty()) {
    response.error_string("SpawnObjectService: failed to retrieve matching blueprint");
    response.id(0);
    return response;
  } else {
    std::vector<carla::actors::ActorBlueprint> blueprint_result;
    std::sample(blueprints->begin(), blueprints->end(), std::back_inserter(blueprint_result), 1,
                std::mt19937{std::random_device{}()});
    if (blueprint_result.size() == 0) {
      response.error_string("SpawnObjectService: failed to retrieve random matching blueprint");
      response.id(0);
      return response;
    }
    auto blueprint = *blueprint_result.begin();
    blueprint.SetAttribute("role_name", request.id());
    for (auto const &attribute : request.attributes()) {
      blueprint.SetAttribute(attribute.key(), attribute.value());
    }

    auto actor_description = blueprint.MakeActorDescription();
    
    carla::rpc::ActorAttributeValue attribute_value;
    attribute_value.id = "enabled_for_ros";
    attribute_value.type = carla::rpc::ActorAttributeType::Bool;
    attribute_value.value = "1";
    actor_description.attributes.push_back(attribute_value);

    carla::rpc::Response<carla::rpc::Actor> result;
    carla::streaming::detail::actor_id_type const parent = request.attach_to();
    if (parent == 0) {
      result = _carla_server.call_spawn_actor(actor_description, transform);
    } else {
      result = _carla_server.call_spawn_actor_with_parent(actor_description, transform, parent,
                                                          carla::rpc::AttachmentType::Rigid, "");
    }
    if (result.HasError()) {
      response.id(0);
      response.error_string(result.GetError().What());
      return response;
    } else {
      response.id(int32_t(result.Get().id));
      return response;
    }
  }
}

}  // namespace ros2
}  // namespace carla
