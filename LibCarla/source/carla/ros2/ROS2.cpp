// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/ROS2.h"

#include "carla/Logging.h"
#include "carla/geom/GeoLocation.h"
#include "carla/geom/Vector3D.h"
#include "carla/ros2/ROS2NameRegistry.h"
#include "carla/ros2/ROS2Session.h"
#include "carla/sensor/SensorRegistry.h"
#include "carla/sensor/data/DVSEvent.h"
#include "carla/sensor/data/Image.h"
#include "carla/sensor/data/LidarData.h"
#include "carla/sensor/data/RadarData.h"
#include "carla/sensor/data/SemanticLidarData.h"
#include "carla/sensor/s11n/SensorHeaderSerializer.h"

#include "carla/ros2/impl/DdsDomainParticipantImpl.h"
#include "carla/ros2/publishers/UeWorldPublisher.h"
#include "carla/ros2/services/DestroyObjectService.h"
#include "carla/ros2/services/GetAvailableMapsService.h"
#include "carla/ros2/services/GetBlueprintsService.h"
#include "carla/ros2/services/LoadMapService.h"
#include "carla/ros2/services/SetEpisodeSettingsService.h"
#include "carla/ros2/services/SpawnObjectService.h"

#include <vector>

namespace carla {
namespace ros2 {

// singleton handling
std::shared_ptr<ROS2> ROS2::GetInstance() {
  static std::shared_ptr<ROS2> _instance{nullptr};
  if (_instance == nullptr) {
    _instance = std::shared_ptr<ROS2>(new ROS2());
  }
  return _instance;
}

void ROS2::Enable(carla::rpc::RpcServerInterface *carla_server,
                  carla::streaming::detail::stream_id_type const world_observer_stream_id,
                  ROS2TopicVisibilityDefaultMode topic_visibility_default_mode) {
  _enabled = true;
  _topic_visibility_default_mode = topic_visibility_default_mode;
  _carla_server = carla_server;
  _name_registry = std::make_shared<ROS2NameRegistry>();
  _domain_participant_impl = std::make_shared<DdsDomainParticipantImpl>();
  // take basic actor role definition as this is acting as naming parent of others with /carla/world
  auto world_observer_actor_definition = carla::ros2::types::ActorNameDefinition::CreateFromRoleName("/", _topic_visibility_default_mode);
  _world_observer_sensor_actor_definition = std::make_shared<carla::ros2::types::SensorActorDefinition>(
      *world_observer_actor_definition,
      carla::ros2::types::PublisherSensorType::WorldObserver, 
      world_observer_stream_id);
  log_info("ROS2 enabled");
}

void ROS2::NotifyInitGame() {
  log_info("ROS2 NotifyInitGame");

  // The world is crucial and has to be instanciated immediately
  _world_publisher = std::make_shared<UeWorldPublisher>(*_carla_server, _name_registry, _world_observer_sensor_actor_definition);
  if (!_world_publisher->Init(_domain_participant_impl)) {
    log_error("ROS2::NotifyInitGame[", std::to_string(*_world_observer_sensor_actor_definition),
              "]: Failed to init publisher");
  } else {
    log_debug("ROS2::NotifyInitGame[", std::to_string(*_world_observer_sensor_actor_definition),
                "]: Publisher initialized");
  }

    ProcessDataFromUeSensorPreAction();
}

void ROS2::NotifyBeginEpisode() {
  log_info("ROS2 NotifyBeginEpisode");

  auto spawn_object_service = std::make_shared<carla::ros2::SpawnObjectService>(
      *_carla_server, carla::ros2::types::ActorNameDefinition::CreateFromRoleName("spawn_object"));
  spawn_object_service->Init(_domain_participant_impl);
  _services.push_back(spawn_object_service);

  auto destroy_object_service = std::make_shared<carla::ros2::DestroyObjectService>(
      *_carla_server, carla::ros2::types::ActorNameDefinition::CreateFromRoleName("destroy_object"));
  destroy_object_service->Init(_domain_participant_impl);
  _services.push_back(destroy_object_service);

  auto get_blueprints_service = std::make_shared<carla::ros2::GetBlueprintsService>(
      *_carla_server, carla::ros2::types::ActorNameDefinition::CreateFromRoleName("get_blueprints"));
  get_blueprints_service->Init(_domain_participant_impl);
  _services.push_back(get_blueprints_service);

  auto get_available_maps_service = std::make_shared<carla::ros2::GetAvailableMapsService>(
      *_carla_server, carla::ros2::types::ActorNameDefinition::CreateFromRoleName("get_available_maps"));
  get_available_maps_service->Init(_domain_participant_impl);
  _services.push_back(get_available_maps_service);

  auto load_map_service = std::make_shared<carla::ros2::LoadMapService>(
      *_carla_server, carla::ros2::types::ActorNameDefinition::CreateFromRoleName("load_map"));
  load_map_service->Init(_domain_participant_impl);
  _services.push_back(load_map_service);

  auto set_epsisode_settings_service = std::make_shared<carla::ros2::SetEpisodeSettingsService>(
      *_carla_server, carla::ros2::types::ActorNameDefinition::CreateFromRoleName("set_episode_settings"));
  set_epsisode_settings_service->Init(_domain_participant_impl);
  _services.push_back(set_epsisode_settings_service);
}

void ROS2::NotifyEndEpisode() {
  log_info("ROS2 NotifyEndEpisode");
  _services.clear();
  _name_registry->Clear();
  _world_publisher->Cleanup();
}

void ROS2::NotifyEndGame() {
  log_info("ROS2 NotifyEndGame");
  NotifyEndEpisode();
  _world_publisher.reset();
}

void ROS2::Disable() {
  NotifyEndEpisode();
  NotifyEndGame();
  _world_observer_sensor_actor_definition.reset();
  _domain_participant_impl.reset();
  _name_registry.reset();
  _enabled = false;
  log_info("ROS2 disabled");
}

void ROS2::AddVehicleUe(std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition,
                        carla::ros2::types::VehicleControlCallback vehicle_control_callback,
                        carla::ros2::types::VehicleAckermannControlCallback vehicle_ackermann_control_callback,
                        carla::ros2::types::ActorSetTransformCallback vehicle_set_transform_callback) {
  log_debug("ROS2::AddVehicleUe(", std::to_string(*vehicle_actor_definition), ")");
  _world_publisher->AddVehicleUe(vehicle_actor_definition, vehicle_control_callback,
                                 vehicle_ackermann_control_callback, vehicle_set_transform_callback);
}

void ROS2::AddWalkerUe(std::shared_ptr<carla::ros2::types::WalkerActorDefinition> walker_actor_definition,
                       carla::ros2::types::WalkerControlCallback walker_control_callback) {
  log_debug("ROS2::AddWalkerUe(", std::to_string(*walker_actor_definition), ")");
  _world_publisher->AddWalkerUe(walker_actor_definition, walker_control_callback);
}

void ROS2::AddTrafficLightUe(
    std::shared_ptr<carla::ros2::types::TrafficLightActorDefinition> traffic_light_actor_definition) {
  log_debug("ROS2::AddTrafficLightUe(", std::to_string(*traffic_light_actor_definition), ")");
  _world_publisher->AddTrafficLightUe(traffic_light_actor_definition);
}

void ROS2::AddTrafficSignUe(
    std::shared_ptr<carla::ros2::types::TrafficSignActorDefinition> traffic_sign_actor_definition) {
  log_debug("ROS2::AddTrafficSignUe(", std::to_string(*traffic_sign_actor_definition), ")");
  _world_publisher->AddTrafficSignUe(traffic_sign_actor_definition);
}

void ROS2::AddSensorUe(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                       carla::ros2::types::ActorSetTransformCallback actor_set_transform_callback) {
  log_debug("ROS2::AddSensorUe(", std::to_string(*sensor_actor_definition), ")");
  _world_publisher->AddSensorUe(sensor_actor_definition, actor_set_transform_callback);
}

void ROS2::AddV2XCustomSensorUe(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition, 
  carla::ros2::types::V2XCustomSendCallback v2x_custom_send_callback) {
  log_debug("ROS2::AddV2XCustomSensorUe(", std::to_string(*sensor_actor_definition), ")");
  _world_publisher->AddV2XCustomSensorUe(sensor_actor_definition, v2x_custom_send_callback);
}

void ROS2::AttachActors(ActorId const child, ActorId const parent) {
  log_debug("ROS2::AttachActors[", child, "]: parent=", parent);
  _world_publisher->AttachActors(child, parent);
}

void ROS2::RemoveActor(ActorId const actor) {
  _world_publisher->RemoveActor(actor);
}

void ROS2::ProcessMessages() {
  for (auto service : _services) {
    service->CheckRequest();
  }
  _world_publisher->ProcessMessages();
}

void ROS2::ProcessDataFromUeSensorPreAction() {
  _world_publisher->UpdateSensorDataPreAction();
}

void ROS2::ProcessDataFromUeSensor(carla::streaming::detail::stream_id_type const stream_id,
                                   std::shared_ptr<const carla::streaming::detail::Message> message) {
  _world_publisher->ProcessDataFromUeSensor(stream_id, message);
}

void ROS2::ProcessDataFromUeSensorPostAction() {
  _world_publisher->UpdateSensorDataPostAction();
}

void ROS2::EnableForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  _world_publisher->enable_for_ros(stream_actor_id.actor_id);
}

void ROS2::DisableForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  _world_publisher->disable_for_ros(stream_actor_id.actor_id);
}

bool ROS2::IsEnabledForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  return _world_publisher->is_enabled_for_ros(stream_actor_id.actor_id);
}

uint64_t ROS2::CurrentFrame() const {
  return (_world_publisher != nullptr) ? _world_publisher->CurrentFrame() : 0u;
}

carla::ros2::types::Timestamp const &ROS2::CurrentTimestamp() const {
  static carla::ros2::types::Timestamp const dummy;
  return (_world_publisher != nullptr) ? _world_publisher->CurrentTimestamp() : dummy;
}

}  // namespace ros2
}  // namespace carla
