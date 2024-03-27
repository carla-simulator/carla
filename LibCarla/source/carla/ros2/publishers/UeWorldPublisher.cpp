// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeWorldPublisher.h"

#include "carla/sensor/data/RawEpisodeState.h"

namespace carla {
namespace ros2 {

UeWorldPublisher::UeWorldPublisher(carla::rpc::RpcServerInterface& carla_server, std::shared_ptr<ROS2NameRegistry> name_registry,
                                   std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition)
  : UePublisherBaseSensor(sensor_actor_definition, std::make_shared<TransformPublisher>()),
    _carla_server(carla_server),
    _name_registry(name_registry),
    _carla_status_publisher(std::make_shared<CarlaStatusPublisher>()),
    _carla_actor_list_publisher(std::make_shared<CarlaActorListPublisher>()),
    _clock_publisher(std::make_shared<ClockPublisher>()),
    _map_publisher(std::make_shared<MapPublisher>()),
    _objects_publisher(std::make_shared<ObjectsPublisher>()),
    _traffic_lights_publisher(std::make_shared<TrafficLightsPublisher>()) {}

bool UeWorldPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  _domain_participant_impl = domain_participant;
  _initialized = _carla_status_publisher->Init(domain_participant) &&
                 _carla_actor_list_publisher->Init(domain_participant) && _clock_publisher->Init(domain_participant) &&
                 _map_publisher->Init(domain_participant) && _objects_publisher->Init(domain_participant) &&
                 _traffic_lights_publisher->Init(domain_participant) && _transform_publisher->Init(domain_participant);
  return _initialized;
}

bool UeWorldPublisher::Publish() {
  if (!_initialized) {
    return false;
  }
  return _clock_publisher->Publish() && _map_publisher->Publish();
}

void UeWorldPublisher::PreTickAction() {
  if (!_initialized) {
    return;
  }
  for (auto& vehicle : _vehicles) {
    if (vehicle.second._vehicle_controller->IsAlive()) {
      if (vehicle.second._vehicle_controller->HasNewMessage()) {
        vehicle.second._vehicle_control_callback(
            carla::ros2::types::VehicleControl(vehicle.second._vehicle_controller->GetMessage()));
      }
    }
    if (vehicle.second._vehicle_ackermann_controller->IsAlive()) {
      if (vehicle.second._vehicle_ackermann_controller->HasNewMessage()) {
        vehicle.second._vehicle_ackermann_control_callback(
            carla::ros2::types::VehicleAckermannControl(vehicle.second._vehicle_ackermann_controller->GetMessage()));
      }
    }
  }
}

void UeWorldPublisher::PostTickAction() {
  if (!_initialized) {
    return;
  }
  _transform_publisher->Publish();

  if (_carla_status_publisher->SubsribersConnected()) {
    _carla_status_publisher->Publish();
  }
  if (_carla_actor_list_publisher->SubsribersConnected()) {
    _carla_actor_list_publisher->Publish();
  }
  if (_objects_publisher->SubsribersConnected()) {
    _objects_publisher->Publish();
  }
  if (_traffic_lights_publisher->SubsribersConnected()) {
    _traffic_lights_publisher->Publish();
  }
}

void UeWorldPublisher::AddVehicleUe(
    std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition,
    carla::ros2::types::VehicleControlCallback vehicle_control_callback,
    carla::ros2::types::VehicleAckermannControlCallback vehicle_ackermann_control_callback) {
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
      std::make_shared<VehiclePublisher>(vehicle_actor_definition, _transform_publisher, _objects_publisher);
  UeVehicle ue_vehicle(vehicle_publisher);
  ue_vehicle._vehicle_controller = std::make_shared<VehicleControlSubscriber>(*vehicle_publisher);
  ue_vehicle._vehicle_ackermann_controller = std::make_shared<AckermannControlSubscriber>(*vehicle_publisher);
  ue_vehicle._vehicle_control_callback = std::move(vehicle_control_callback);
  ue_vehicle._vehicle_ackermann_control_callback = std::move(vehicle_ackermann_control_callback);
  auto vehicle_result = _vehicles.insert({vehicle_actor_definition->id, ue_vehicle});
  if (!vehicle_result.second) {
    vehicle_result.first->second = std::move(ue_vehicle);
  }

  vehicle_result.first->second._vehicle_publisher->Init(_domain_participant_impl);
  vehicle_result.first->second._vehicle_controller->Init(_domain_participant_impl);
  vehicle_result.first->second._vehicle_ackermann_controller->Init(_domain_participant_impl);
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
      std::make_shared<WalkerPublisher>(walker_actor_definition, _transform_publisher, _objects_publisher);
  UeWalker ue_walker(walker_publisher);
  ue_walker._walker_controller = std::make_shared<WalkerControlSubscriber>(*walker_publisher);
  ue_walker._walker_control_callback = std::move(walker_control_callback);
  auto walker_result = _walkers.insert({walker_actor_definition->id, ue_walker});
  if (!walker_result.second) {
    walker_result.first->second = std::move(ue_walker);
  }

  walker_result.first->second._walker_publisher->Init(_domain_participant_impl);
  walker_result.first->second._walker_controller->Init(_domain_participant_impl);
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
                                                                         _objects_publisher, _traffic_lights_publisher);
  UeTrafficLight ue_traffic_light(traffic_light_publisher);
  auto traffic_light_result = _traffic_lights.insert({traffic_light_actor_definition->id, ue_traffic_light});
  if (!traffic_light_result.second) {
    traffic_light_result.first->second = std::move(ue_traffic_light);
  }

  traffic_light_result.first->second._traffic_light_publisher->Init(_domain_participant_impl);
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
      std::make_shared<TrafficSignPublisher>(traffic_sign_actor_definition, _objects_publisher);
  UeTrafficSign ue_traffic_sign(traffic_sign_publisher);
  auto traffic_sign_result = _traffic_signs.insert({traffic_sign_actor_definition->id, ue_traffic_sign});
  if (!traffic_sign_result.second) {
    traffic_sign_result.first->second = std::move(ue_traffic_sign);
  }

  traffic_sign_result.first->second._traffic_sign_publisher->Init(_domain_participant_impl);
}

void UeWorldPublisher::RemoveActor(ActorId actor) {
  if (!_initialized) {
    return;
  }
  _objects.erase(actor);
  _objects_changed = true;
  _vehicles.erase(actor);
  _walkers.erase(actor);
  _traffic_lights.erase(actor);
  _traffic_lights_publisher->RemoveTrafficLight(actor);
  _traffic_signs.erase(actor);
}

void UeWorldPublisher::UpdateSensorData(
    std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
    carla::SharedBufferView buffer_view) {
  if (!_initialized) {
    return;
  }
  _frame = sensor_header->frame;
  _timestamp = carla::ros2::types::Timestamp(sensor_header->timestamp);
  _clock_publisher->UpdateData(_timestamp.time());

  _episode_header = *header_view(buffer_view);

  carla_msgs::msg::CarlaStatus status;
  status.frame(_frame);
  status.fixed_delta_seconds(_episode_header.delta_seconds);
  auto episode_settings = _carla_server.call_get_episode_settings();
  status.synchronous_mode(episode_settings.Get().synchronous_mode);
  _carla_status_publisher->UpdateCarlaStatus(status, _timestamp.time());

  if (_episode_header.simulation_state & carla::sensor::s11n::EpisodeStateSerializer::MapChange) {
    _map_publisher->UpdateData(_carla_server.call_get_map_data().Get());
  }

  for (auto const& actor_dynamic_state : buffer_data_2_vector(buffer_view)) {
    auto object_it = _objects.find(actor_dynamic_state.id);
    std::shared_ptr<carla::ros2::types::Object> object = nullptr;
    if (object_it != _objects.end()) {
      object = object_it->second;
      object->UpdateObject(_timestamp, actor_dynamic_state);
    }

    if (object != nullptr) {
      carla::ros2::types::Transform transform(actor_dynamic_state.transform, actor_dynamic_state.quaternion);
      auto vehicle_it = _vehicles.find(actor_dynamic_state.id);
      if (vehicle_it != _vehicles.end()) {
        UeVehicle& ue_vehicle = vehicle_it->second;
        auto publisher = ue_vehicle._vehicle_publisher;
        publisher->UpdateTransform(_timestamp, transform);
        publisher->UpdateVehicle(object, actor_dynamic_state);
        if (publisher->SubsribersConnected()) {
          publisher->Publish();
        }
      }

      auto walker_it = _walkers.find(actor_dynamic_state.id);
      if (walker_it != _walkers.end()) {
        UeWalker& ue_walker = walker_it->second;
        auto publisher = ue_walker._walker_publisher;
        publisher->UpdateTransform(_timestamp, transform);
        publisher->UpdateWalker(object, actor_dynamic_state);
        if (publisher->SubsribersConnected()) {
          publisher->Publish();
        }
      }

      auto traffic_sign_it = _traffic_signs.find(actor_dynamic_state.id);
      if (traffic_sign_it != _traffic_signs.end()) {
        UeTrafficSign& ue_traffic_sign = traffic_sign_it->second;
        auto publisher = ue_traffic_sign._traffic_sign_publisher;
        publisher->UpdateTrafficSign(object, actor_dynamic_state);
        if (publisher->SubsribersConnected()) {
          publisher->Publish();
        }
      }

      auto traffic_light_it = _traffic_lights.find(actor_dynamic_state.id);
      if (traffic_light_it != _traffic_lights.end()) {
        UeTrafficLight& ue_traffic_light = traffic_light_it->second;
        auto publisher = ue_traffic_light._traffic_light_publisher;
        publisher->UpdateTrafficLight(object, actor_dynamic_state);
        if (publisher->SubsribersConnected()) {
          publisher->Publish();
        }
      }
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

}  // namespace ros2
}  // namespace carla
