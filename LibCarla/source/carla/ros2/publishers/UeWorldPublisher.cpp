// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeWorldPublisher.h"

#include "carla/sensor/data/RawEpisodeState.h"
#include "carla/ros2/types/EpisodeSettings.h"

namespace carla {
namespace ros2 {

UeWorldPublisher::UeWorldPublisher(carla::rpc::RpcServerInterface& carla_server,
                                   std::shared_ptr<ROS2NameRegistry> name_registry,
                                   std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition)
  : UePublisherBaseSensor(sensor_actor_definition, std::make_shared<TransformPublisher>()),
    _carla_server(carla_server),
    _name_registry(name_registry),
    _carla_status_publisher(std::make_shared<CarlaStatusPublisher>()),
    _carla_actor_list_publisher(std::make_shared<CarlaActorListPublisher>("actor_list")),
    _clock_publisher(std::make_shared<ClockPublisher>()),
    _map_publisher(std::make_shared<MapPublisher>()),
    _objects_publisher(std::make_shared<ObjectsPublisher>()),
    _objects_with_covariance_publisher(std::make_shared<ObjectsWithCovariancePublisher>()),
    _traffic_lights_publisher(std::make_shared<TrafficLightsPublisher>()),
    _carla_control_subscriber(std::make_shared<CarlaControlSubscriber>(*this, _carla_server)),
    _sync_subscriber(std::make_shared<CarlaSynchronizationWindowSubscriber>(*this, _carla_server)) {
}

bool UeWorldPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  _domain_participant_impl = domain_participant;
  _initialized = _carla_status_publisher->Init(domain_participant) &&
                 _carla_actor_list_publisher->Init(domain_participant) && _clock_publisher->Init(domain_participant) &&
                 _map_publisher->Init(domain_participant) && _objects_publisher->Init(domain_participant) &&
                 _objects_with_covariance_publisher->Init(domain_participant) && _traffic_lights_publisher->Init(domain_participant) &&
                 _transform_publisher->Init(domain_participant) &&
                 _carla_control_subscriber->Init(domain_participant) && _sync_subscriber->Init(domain_participant);
  return _initialized;
}

bool UeWorldPublisher::Publish() {
  if (!_initialized) {
    return false;
  }
  return _clock_publisher->Publish() && _map_publisher->Publish();
}

void UeWorldPublisher::ProcessMessages() {
  if (!_initialized) {
    return;
  }

  _carla_control_subscriber->ProcessMessages();
  _sync_subscriber->ProcessMessages();
  for (auto& vehicle : _vehicles) {
    vehicle.second._vehicle_controller->ProcessMessages();
    vehicle.second._vehicle_ackermann_controller->ProcessMessages();
    vehicle.second._actor_set_transform_subscriber->ProcessMessages();
    vehicle.second._vehicle_publisher->ProcessMessages();
  }
  for (auto& walker : _walkers) {
    walker.second._walker_controller->ProcessMessages();
  }

  UpdateAndPublishStatus();
}

void UeWorldPublisher::UpdateSensorDataPostAction() {
  if (!_initialized) {
    return;
  }

  UpdateAndPublishStatus();

  _transform_publisher->Publish();
  _carla_actor_list_publisher->Publish();
  _objects_publisher->Publish();
  _objects_with_covariance_publisher->Publish();
  _traffic_lights_publisher->Publish();
}

void UeWorldPublisher::AddVehicleUe(
    std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition,
    carla::ros2::types::VehicleControlCallback vehicle_control_callback,
    carla::ros2::types::VehicleAckermannControlCallback vehicle_ackermann_control_callback,
    carla::ros2::types::ActorSetTransformCallback vehicle_set_transform_callback) {
  if (!_initialized) {
    return;
  }

  auto object = std::make_shared<carla::ros2::types::Object>(vehicle_actor_definition);
  auto object_result = _objects.insert({vehicle_actor_definition->id, object});
  if (!object_result.second) {
    object_result.first->second = object;
  }
  _objects_changed = true;

  auto vehicle_publisher =
      std::make_shared<VehiclePublisher>(vehicle_actor_definition, _transform_publisher, _objects_publisher, _objects_with_covariance_publisher, _carla_server);
  UeVehicle ue_vehicle(vehicle_publisher);
  ue_vehicle._vehicle_controller =
      std::make_shared<VehicleControlSubscriber>(*vehicle_publisher, std::move(vehicle_control_callback));
  ue_vehicle._vehicle_ackermann_controller =
      std::make_shared<AckermannControlSubscriber>(*vehicle_publisher, std::move(vehicle_ackermann_control_callback));
  ue_vehicle._actor_set_transform_subscriber =
      std::make_shared<ActorSetTransformSubscriber>(*vehicle_publisher, std::move(vehicle_set_transform_callback));

  auto vehicle_result = _vehicles.insert({vehicle_actor_definition->id, ue_vehicle});
  if (!vehicle_result.second) {
    vehicle_result.first->second = std::move(ue_vehicle);
  }
  vehicle_result.first->second.Init(_domain_participant_impl);
}

void UeWorldPublisher::UeVehicle::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant)
{
  if ( _vehicle_publisher->is_enabled_for_ros() ) {
    _vehicle_publisher->Init(domain_participant);
    _vehicle_controller->Init(domain_participant);
    _vehicle_ackermann_controller->Init(domain_participant);
    _actor_set_transform_subscriber->Init(domain_participant);
  }
}

void UeWorldPublisher::AddWalkerUe(std::shared_ptr<carla::ros2::types::WalkerActorDefinition> walker_actor_definition,
                                   carla::ros2::types::WalkerControlCallback walker_control_callback) {
  if (!_initialized) {
    return;
  }
  auto object = std::make_shared<carla::ros2::types::Object>(walker_actor_definition);
  auto object_result = _objects.insert({walker_actor_definition->id, object});
  if (!object_result.second) {
    object_result.first->second = object;
  }
  _objects_changed = true;

  auto walker_publisher =
      std::make_shared<WalkerPublisher>(walker_actor_definition, _transform_publisher, _objects_publisher, _objects_with_covariance_publisher);
  UeWalker ue_walker(walker_publisher);
  ue_walker._walker_controller =
      std::make_shared<WalkerControlSubscriber>(*walker_publisher, std::move(walker_control_callback));

  auto walker_result = _walkers.insert({walker_actor_definition->id, ue_walker});
  if (!walker_result.second) {
    walker_result.first->second = std::move(ue_walker);
  }

  walker_result.first->second.Init(_domain_participant_impl);
}

void UeWorldPublisher::UeWalker::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant)
{
  if ( _walker_publisher->is_enabled_for_ros() ) {
    _walker_publisher->Init(domain_participant);
    _walker_controller->Init(domain_participant);
  }
}

void UeWorldPublisher::AddTrafficLightUe(
    std::shared_ptr<carla::ros2::types::TrafficLightActorDefinition> traffic_light_actor_definition) {
  auto object = std::make_shared<carla::ros2::types::Object>(traffic_light_actor_definition);
  if (!_initialized) {
    return;
  }
  auto object_result = _objects.insert({traffic_light_actor_definition->id, object});
  if (!object_result.second) {
    object_result.first->second = object;
  }
  _objects_changed = true;

  auto traffic_light_publisher = std::make_shared<TrafficLightPublisher>(traffic_light_actor_definition,
                                                                         _objects_publisher, _objects_with_covariance_publisher, _traffic_lights_publisher);
  UeTrafficLight ue_traffic_light(traffic_light_publisher);
  auto traffic_light_result = _traffic_lights.insert({traffic_light_actor_definition->id, ue_traffic_light});
  if (!traffic_light_result.second) {
    traffic_light_result.first->second = std::move(ue_traffic_light);
  }

  traffic_light_result.first->second.Init(_domain_participant_impl);
}

void UeWorldPublisher::UeTrafficLight::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant)
{
  if ( _traffic_light_publisher->is_enabled_for_ros() ) {
    _traffic_light_publisher->Init(domain_participant);
  }
}

void UeWorldPublisher::AddTrafficSignUe(
    std::shared_ptr<carla::ros2::types::TrafficSignActorDefinition> traffic_sign_actor_definition) {
  if (!_initialized) {
    return;
  }
  auto object = std::make_shared<carla::ros2::types::Object>(traffic_sign_actor_definition);
  auto object_result = _objects.insert({traffic_sign_actor_definition->id, object});
  if (!object_result.second) {
    object_result.first->second = object;
  }
  _objects_changed = true;

  auto traffic_sign_publisher =
      std::make_shared<TrafficSignPublisher>(traffic_sign_actor_definition, _objects_publisher, _objects_with_covariance_publisher);
  UeTrafficSign ue_traffic_sign(traffic_sign_publisher);
  auto traffic_sign_result = _traffic_signs.insert({traffic_sign_actor_definition->id, ue_traffic_sign});
  if (!traffic_sign_result.second) {
    traffic_sign_result.first->second = std::move(ue_traffic_sign);
  }

  traffic_sign_result.first->second.Init(_domain_participant_impl);
}

void UeWorldPublisher::UeTrafficSign::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant)
{
  if ( _traffic_sign_publisher->is_enabled_for_ros() ) {
    _traffic_sign_publisher->Init(domain_participant);
  }
}

void UeWorldPublisher::RemoveActor(ActorId actor) {
  if (!_initialized) {
    return;
  }
  _objects.erase(actor);
  _objects_changed = true;
  auto vehicle_iter = _vehicles.find(actor);
  if ( vehicle_iter != _vehicles.end() ) {
    log_debug("ROS2::RemoveVehicleUe(", std::to_string(
      *std::static_pointer_cast<carla::ros2::types::VehicleActorDefinition>(vehicle_iter->second._vehicle_publisher->_actor_name_definition)), ")");
    _vehicles.erase(vehicle_iter);
  }
  auto walker_iter = _walkers.find(actor);
  if ( walker_iter != _walkers.end() ) {
    log_debug("ROS2::RemoveWalkerUe(", std::to_string(
      *std::static_pointer_cast<carla::ros2::types::WalkerActorDefinition>(walker_iter->second._walker_publisher->_actor_name_definition)), ")");
    _walkers.erase(walker_iter);
  }
  auto traffic_light_iter = _traffic_lights.find(actor);
  if ( traffic_light_iter != _traffic_lights.end() ) {
    log_debug("ROS2::RemoveTrafficLightUe(", std::to_string(
      *std::static_pointer_cast<carla::ros2::types::TrafficLightActorDefinition>(traffic_light_iter->second._traffic_light_publisher->_actor_name_definition)), ")");
    _traffic_lights.erase(traffic_light_iter);
  }
  _traffic_lights_publisher->RemoveTrafficLight(actor);

  auto traffic_sign_iter = _traffic_signs.find(actor);
  if ( traffic_sign_iter != _traffic_signs.end() ) {
    log_debug("ROS2::RemoveTrafficSignUe(", std::to_string(
      *std::static_pointer_cast<carla::ros2::types::TrafficSignActorDefinition>(traffic_sign_iter->second._traffic_sign_publisher->_actor_name_definition)), ")");
    _traffic_signs.erase(traffic_sign_iter);
  }
}

void UeWorldPublisher::UpdateAndPublishStatus() {
  auto const synchronization_window_status = _carla_server.call_get_synchronization_window_status();
  if (_frame_changed || synchronization_window_status.Get().first) {
    _frame_changed = false;
    carla_msgs::msg::CarlaStatus status;
    status.frame(_frame);
    carla::ros2::types::EpisodeSettings carla_episode_settings(_carla_server.call_get_episode_settings().Get());
    status.episode_settings( carla_episode_settings.episode_settings());
    status.header().stamp(_timestamp.time());
    status.header().frame_id("");
    status.synchronous_mode_participant_states().reserve(synchronization_window_status.Get().second.size());
    double synchronization_target_game_time_min = std::numeric_limits<double>::max();
    for ( auto const &synchronization_window_participant_state: synchronization_window_status.Get().second) {
      carla_msgs::msg::CarlaSynchronizationWindowParticipantState participant_state;
      participant_state.client_id(synchronization_window_participant_state.client_id);
      participant_state.participant_id(synchronization_window_participant_state.participant_id);
      carla::ros2::types::Timestamp target_game_time(synchronization_window_participant_state.target_game_time);
      participant_state.target_game_time(target_game_time.Stamp());
      status.synchronous_mode_participant_states().push_back(participant_state);
      if ( target_game_time.Stamp() > 0. ) {
        synchronization_target_game_time_min = std::min(synchronization_target_game_time_min, target_game_time.Stamp());
      }
    }
    
    status.game_running(synchronization_target_game_time_min > _timestamp.Stamp());
    _carla_status_publisher->UpdateCarlaStatus(status);

    _carla_status_publisher->Publish();
  }
}

void UeWorldPublisher::UpdateSensorData(
    std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
    carla::SharedBufferView buffer_view) {
  if (!_initialized) {
    return;
  }
  _frame_changed = true;
  _frame = sensor_header->frame;
  _timestamp = carla::ros2::types::Timestamp(sensor_header->timestamp);
  _clock_publisher->UpdateData(_timestamp.time());
  _objects_publisher->UpdateHeader(_timestamp.time());
  _objects_with_covariance_publisher->UpdateHeader(_timestamp.time());

  _episode_header = *header_view(buffer_view);

  if (_episode_header.simulation_state & carla::sensor::s11n::EpisodeStateSerializer::MapChange) {
    _map_publisher->UpdateData(_carla_server.call_get_map_data().Get());
  }

  for (auto const& actor_dynamic_state : buffer_data_2_vector(buffer_view)) {
    auto object_it = _objects.find(actor_dynamic_state.id);
    std::shared_ptr<carla::ros2::types::Object> object = nullptr;
    bool object_enabled_for_ros = false;
    if (object_it != _objects.end()) {
      object = object_it->second;
    }

    if (object != nullptr) {
      carla::ros2::types::Transform transform(actor_dynamic_state.transform, actor_dynamic_state.quaternion);
      auto vehicle_it = _vehicles.find(actor_dynamic_state.id);
      if (vehicle_it != _vehicles.end()) {
        UeVehicle& ue_vehicle = vehicle_it->second;
        auto publisher = ue_vehicle._vehicle_publisher;
        if ( publisher->is_enabled_for_ros() ) {
          object_enabled_for_ros = true;
          publisher->UpdateTransform(_timestamp, transform);
          publisher->UpdateVehicle(object, actor_dynamic_state);
          publisher->Publish();
        }
      }

      auto walker_it = _walkers.find(actor_dynamic_state.id);
      if (walker_it != _walkers.end()) {
        UeWalker& ue_walker = walker_it->second;
        auto publisher = ue_walker._walker_publisher;
        if ( publisher->is_enabled_for_ros() ) {
          object_enabled_for_ros = true;
          publisher->UpdateTransform(_timestamp, transform);
          publisher->UpdateWalker(object, actor_dynamic_state);
          publisher->Publish();
        }
      }

      auto traffic_sign_it = _traffic_signs.find(actor_dynamic_state.id);
      if (traffic_sign_it != _traffic_signs.end()) {
        UeTrafficSign& ue_traffic_sign = traffic_sign_it->second;
        auto publisher = ue_traffic_sign._traffic_sign_publisher;
        if ( publisher->is_enabled_for_ros() ) {
          object_enabled_for_ros = true;
          publisher->UpdateTrafficSign(object, actor_dynamic_state);
          publisher->Publish();
        }
      }

      auto traffic_light_it = _traffic_lights.find(actor_dynamic_state.id);
      if (traffic_light_it != _traffic_lights.end()) {
        UeTrafficLight& ue_traffic_light = traffic_light_it->second;
        auto publisher = ue_traffic_light._traffic_light_publisher;
        if ( publisher->is_enabled_for_ros() ) {
          object_enabled_for_ros = true;
          publisher->UpdateTrafficLight(object, actor_dynamic_state);
          publisher->Publish();
        }
      }
    }

    if ( object_enabled_for_ros ) {
      object->UpdateObject(_timestamp, actor_dynamic_state);
    }
  }

  if (_objects_changed) {
    _objects_changed = false;
    carla_msgs::msg::CarlaActorList actor_list;
    for (auto const& object : _objects) {
      actor_list.actors().push_back(object.second->carla_actor_info(_name_registry));
    }
    _carla_actor_list_publisher->UpdateCarlaActorList(actor_list);
  }
}

void UeWorldPublisher::enable_for_ros(carla::streaming::detail::actor_id_type actor_id) {
  if ( actor_id == _actor_name_definition->id ) {
    // the world publisher itself always enabled
    return;
  }
  auto vehicle_it = _vehicles.find(actor_id);
  if (vehicle_it != _vehicles.end()) {
    vehicle_it->second._vehicle_publisher->enable_for_ros();
  }
  auto walker_it = _walkers.find(actor_id);
  if (walker_it != _walkers.end()) {
    walker_it->second._walker_publisher->enable_for_ros();
  }
  auto traffic_sign_it = _traffic_signs.find(actor_id);
  if (traffic_sign_it != _traffic_signs.end()) {
    traffic_sign_it->second._traffic_sign_publisher->enable_for_ros();
  }
  auto traffic_light_it = _traffic_lights.find(actor_id);
  if (traffic_light_it != _traffic_lights.end()) {
    traffic_light_it->second._traffic_light_publisher->enable_for_ros();
  }
}

void UeWorldPublisher::disable_for_ros(carla::streaming::detail::actor_id_type actor_id) {
  if ( actor_id == _actor_name_definition->id ) {
    // the world publisher itself always enabled
    return;
  }
  auto vehicle_it = _vehicles.find(actor_id);
  if (vehicle_it != _vehicles.end()) {
    vehicle_it->second._vehicle_publisher->disable_for_ros();
  }
  auto walker_it = _walkers.find(actor_id);
  if (walker_it != _walkers.end()) {
    walker_it->second._walker_publisher->disable_for_ros();
  }
  auto traffic_sign_it = _traffic_signs.find(actor_id);
  if (traffic_sign_it != _traffic_signs.end()) {
    traffic_sign_it->second._traffic_sign_publisher->disable_for_ros();
  }
  auto traffic_light_it = _traffic_lights.find(actor_id);
  if (traffic_light_it != _traffic_lights.end()) {
    traffic_light_it->second._traffic_light_publisher->disable_for_ros();
  }
}

bool UeWorldPublisher::is_enabled_for_ros(carla::streaming::detail::actor_id_type actor_id) const {
  if ( actor_id == _actor_name_definition->id ) {
    // the world publisher itself always enabled
    return true;
  }
  auto vehicle_it = _vehicles.find(actor_id);
  if (vehicle_it != _vehicles.end()) {
    return vehicle_it->second._vehicle_publisher->is_enabled_for_ros();
  }
  auto walker_it = _walkers.find(actor_id);
  if (walker_it != _walkers.end()) {
    return walker_it->second._walker_publisher->is_enabled_for_ros();
  }
  auto traffic_sign_it = _traffic_signs.find(actor_id);
  if (traffic_sign_it != _traffic_signs.end()) {
    return traffic_sign_it->second._traffic_sign_publisher->is_enabled_for_ros();
  }
  auto traffic_light_it = _traffic_lights.find(actor_id);
  if (traffic_light_it != _traffic_lights.end()) {
    return traffic_light_it->second._traffic_light_publisher->is_enabled_for_ros();
  }
  return false;
}

}  // namespace ros2
}  // namespace carla
