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

#include "carla/ros2/publishers/CarlaActorListPublisher.h"
#include "carla/ros2/publishers/TransformPublisher.h"
#include "carla/ros2/publishers/UeCollisionPublisher.h"
#include "carla/ros2/publishers/UeDVSCameraPublisher.h"
#include "carla/ros2/publishers/UeDepthCameraPublisher.h"
#include "carla/ros2/publishers/UeGNSSPublisher.h"
#include "carla/ros2/publishers/UeIMUPublisher.h"
#include "carla/ros2/publishers/UeISCameraPublisher.h"
#include "carla/ros2/publishers/UeLidarPublisher.h"
#include "carla/ros2/publishers/UeNormalsCameraPublisher.h"
#include "carla/ros2/publishers/UeOpticalFlowCameraPublisher.h"
#include "carla/ros2/publishers/UeRGBCameraPublisher.h"
#include "carla/ros2/publishers/UeRadarPublisher.h"
#include "carla/ros2/publishers/UeSSCameraPublisher.h"
#include "carla/ros2/publishers/UeSemanticLidarPublisher.h"
#include "carla/ros2/publishers/UeWorldPublisher.h"
#include "carla/ros2/publishers/UeV2XPublisher.h"
#include "carla/ros2/publishers/UeV2XCustomPublisher.h"
#include "carla/ros2/publishers/VehiclePublisher.h"

#include "carla/ros2/services/DestroyObjectService.h"
#include "carla/ros2/services/GetAvailableMapsService.h"
#include "carla/ros2/services/GetBlueprintsService.h"
#include "carla/ros2/services/LoadMapService.h"
#include "carla/ros2/services/SetEpisodeSettingsService.h"
#include "carla/ros2/services/SpawnObjectService.h"

#include "carla/ros2/subscribers/AckermannControlSubscriber.h"
#include "carla/ros2/subscribers/VehicleControlSubscriber.h"

#include "carla/ros2/types/Acceleration.h"
#include "carla/ros2/types/AngularVelocity.h"
#include "carla/ros2/types/Quaternion.h"
#include "carla/ros2/types/Speed.h"
#include "carla/ros2/types/VehicleAckermannControl.h"
#include "carla/ros2/types/VehicleControl.h"

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
                  TopicVisibilityDefaultMode topic_visibility_default_mode) {
  _enabled = true;
  _topic_visibility_default_mode = topic_visibility_default_mode;
  _carla_server = carla_server;
  _name_registry = std::make_shared<ROS2NameRegistry>();
  _dispatcher = _carla_server->GetDispatcher();
  _domain_participant_impl = std::make_shared<DdsDomainParticipantImpl>();
  // take basic actor role definition as this is acting as naming parent of others with /carla/world
  auto world_observer_actor_definition = carla::ros2::types::ActorNameDefinition::CreateFromRoleName("/", true);
  _world_observer_sensor_actor_definition = std::make_shared<carla::ros2::types::SensorActorDefinition>(
      *world_observer_actor_definition,
      carla::ros2::types::PublisherSensorType::WorldObserver, 
      world_observer_stream_id);
  log_info("ROS2 enabled");
}

void ROS2::NotifyInitGame() {
  log_info("ROS2 NotifyInitGame");

  _carla_sensor_actor_list_publisher = std::make_shared<CarlaActorListPublisher>("sensor_list");
  _carla_sensor_actor_list_publisher->Init(_domain_participant_impl);

  // The world is crucial and has to be instanciated immediately
  if (AddSensorUe(_world_observer_sensor_actor_definition)) {
    ProcessDataFromUeSensorPreAction();
  }
  if (_world_publisher != nullptr) {
    _transform_publisher = _world_publisher->GetTransformPublisher();
  }
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

  auto load_map_service = std::make_shared<carla::ros2::SpawnObjectService>(
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
  _ue_sensors.clear();
  _name_registry->Clear();
}

void ROS2::NotifyEndGame() {
  log_info("ROS2 NotifyEndGame");
  NotifyEndEpisode();
  _world_publisher.reset();
  _transform_publisher.reset();
  _carla_sensor_actor_list_publisher.reset();
}

void ROS2::Disable() {
  NotifyEndEpisode();
  NotifyEndGame();
  _carla_sensor_actor_list_publisher.reset();
  _world_observer_sensor_actor_definition.reset();
  _dispatcher.reset();
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

ROS2::UeSensor* ROS2::AddSensorUeInternal(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition) {
  auto insert_result = _ue_sensors.insert({sensor_actor_definition->stream_id, UeSensor(sensor_actor_definition)});
  if (!insert_result.second) {
    log_warning("ROS2::AddSensorUe(", std::to_string(*sensor_actor_definition),
                "): Sensor already_registered. Ignoring");
    return nullptr;
  }
  _ue_sensors_changed = true;
  return &insert_result.first->second;
}    

bool ROS2::AddSensorUe(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                       carla::ros2::types::ActorSetTransformCallback actor_set_transform_callback) {
  auto ue_sensor = AddSensorUeInternal(sensor_actor_definition);
  if ( ue_sensor != nullptr ) {
    ue_sensor->actor_set_transform_callback = actor_set_transform_callback;
    return true;
  }
  return false;
}

bool ROS2::AddV2XCustomSensorUe(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition, 
    carla::ros2::types::V2XCustomSendCallback v2x_custom_send_callback) {
  auto ue_sensor = AddSensorUeInternal(sensor_actor_definition);
  if ( ue_sensor != nullptr ) {
    ue_sensor->v2x_custom_send_callback = v2x_custom_send_callback;
    return true;
  }
  return false;
}

void ROS2::AttachActors(ActorId const child, ActorId const parent) {
  log_debug("ROS2::AttachActors[", child, "]: parent=", parent);
  _name_registry->AttachActors(child, parent);
  for (auto iter = _ue_sensors.begin(); iter != _ue_sensors.end(); ++iter) {
    if (iter->second.sensor_actor_definition->id == child) {
      UeSensor &sensor = iter->second;
      if (sensor.publisher) {
        log_error("ROS2::AttachActors[", std::to_string(*sensor.sensor_actor_definition),
                  "]: Sensor attached to parent ", parent,
                  ". Sensor has already a running publisher with base topic name ", sensor.publisher->get_topic_name(),
                  " has to be destroyed due to re-attachment");
        sensor.publisher.reset();
      }
      _ue_sensors_changed = true;
      break;
    }
  }
}

void ROS2::CreateSensorUePublisher(UeSensor &sensor) {
  // Create the respective sensor publisher
  switch (sensor.sensor_actor_definition->sensor_type) {
    case types::PublisherSensorType::CollisionSensor:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeCollisionPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::DepthCamera:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeDepthCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::NormalsCamera:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeNormalsCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::DVSCamera:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeDVSCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::GnssSensor:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeGNSSPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::InertialMeasurementUnit:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeIMUPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::OpticalFlowCamera:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeOpticalFlowCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::Radar:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeRadarPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::RayCastSemanticLidar:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeSemanticLidarPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::RayCastLidar:
    case types::PublisherSensorType::HSSLidar: 
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeLidarPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::SceneCaptureCamera:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeRGBCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher, sensor.actor_set_transform_callback));
    } break;
    case types::PublisherSensorType::SemanticSegmentationCamera:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeSSCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::InstanceSegmentationCamera:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeISCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::WorldObserver:
    {
      _world_publisher =
          std::make_shared<UeWorldPublisher>(*_carla_server, _name_registry, sensor.sensor_actor_definition);
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(_world_publisher);
    } break;
    case types::PublisherSensorType::V2X:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeV2XPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::V2XCustom:
    {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeV2XCustomPublisher>(sensor.sensor_actor_definition, sensor.v2x_custom_send_callback, _transform_publisher));
    } break;
    case types::PublisherSensorType::RssSensor:
      // no server side interface to be implemented: maybe move client based implementation from client to the sensor
      // folder for those? in each case should be implemented in a form that the actual calcuations are only performed
      // if anyone listening to the topic
    case types::PublisherSensorType::CameraGBufferUint8:
    case types::PublisherSensorType::CameraGBufferFloat:
    case types::PublisherSensorType::LaneInvasionSensor:
    case types::PublisherSensorType::ObstacleDetectionSensor:
    default: {
      sensor.publisher_expected = false;
      log_error("ROS2::CreateSensorUePublisher[", std::to_string(*sensor.sensor_actor_definition),
                "]: Not a UE sensor or no publisher implemented yet");
    }
  }
  if (sensor.publisher != nullptr) {
    if (!sensor.publisher->Init(_domain_participant_impl)) {
      log_error("ROS2::CreateSensorUePublisher[", std::to_string(*sensor.sensor_actor_definition),
                "]: Failed to init publisher");
    } else {
      log_debug("ROS2::CreateSensorUePublisher[", std::to_string(*sensor.sensor_actor_definition),
                  "]: Publisher initialized");
    }
  }
}

void ROS2::RemoveActor(ActorId const actor) {
  for (auto iter = _ue_sensors.begin(); iter != _ue_sensors.end(); /*no update of iter*/) {
    if (iter->second.sensor_actor_definition->id == actor) {
      log_debug("ROS2::RemoveSensorUe(", std::to_string(*iter->second.sensor_actor_definition), ")");
      iter = _ue_sensors.erase(iter);
      _ue_sensors_changed = true;
    } else {
      ++iter;
    }
  }
  _world_publisher->RemoveActor(actor);
}

void ROS2::ProcessMessages() {
  for (auto service : _services) {
    service->CheckRequest();
  }
  _world_publisher->ProcessMessages();
}

void ROS2::ProcessDataFromUeSensorPreAction() {
  for (auto &ue_sensor : _ue_sensors) {
    if (ue_sensor.second.publisher_expected && (ue_sensor.second.publisher == nullptr)) {
      CreateSensorUePublisher(ue_sensor.second);
    }
    if (ue_sensor.second.publisher != nullptr) {
      if (ue_sensor.second.publisher->SubscribersConnected() && ue_sensor.second.session == nullptr) {
        ue_sensor.second.session = std::make_shared<ROS2Session>(ue_sensor.first);
        log_debug("ROS2::ProcessDataFromUeSensorPreAction[", std::to_string(*ue_sensor.second.sensor_actor_definition),
                    "]: Registering session");
        _dispatcher->RegisterSession(ue_sensor.second.session);
      } else if (!ue_sensor.second.publisher->SubscribersConnected() && ue_sensor.second.session != nullptr) {
        log_debug("ROS2::ProcessDataFromUeSensorPreAction[", std::to_string(*ue_sensor.second.sensor_actor_definition),
                    "]: Deregistering session");
        _dispatcher->DeregisterSession(ue_sensor.second.session);
        ue_sensor.second.session.reset();
      }
    }
  }

  for (auto &ue_sensor : _ue_sensors) {
    if ( (ue_sensor.second.publisher != nullptr) ) {
      ue_sensor.second.publisher->UpdateSensorDataPreAction();
    } 
  }


  if (_ue_sensors_changed) {
    _ue_sensors_changed = false;
    carla_msgs::msg::CarlaActorList actor_list;
    for (auto &ue_sensor : _ue_sensors) {
      actor_list.actors().push_back(ue_sensor.second.sensor_actor_definition->carla_actor_info(_name_registry));
    }
    _carla_sensor_actor_list_publisher->UpdateCarlaActorList(actor_list);
    _carla_sensor_actor_list_publisher->Publish();
  }


  _world_publisher->UpdateSensorDataPreAction();
}


void ROS2::ProcessDataFromUeSensor(carla::streaming::detail::stream_id_type const stream_id,
                                   std::shared_ptr<const carla::streaming::detail::Message> message) {
  auto ue_sensor = _ue_sensors.find(stream_id);
  if (ue_sensor != _ue_sensors.end()) {
    auto const &sensor_actor_definition = ue_sensor->second.sensor_actor_definition;

    auto buffer_list_view = message->GetBufferViewSequence();
    // currently we only support sensor header + data buffer
    DEBUG_ASSERT_EQ(buffer_list_view.size(), 2u);
    carla::SharedBufferView sensor_header_view = *buffer_list_view.begin();

    auto sensor_header = std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const>(
        sensor_header_view, reinterpret_cast<carla::sensor::s11n::SensorHeaderSerializer::Header const *>(
                                sensor_header_view.get()->data()));

    if (ue_sensor->second.publisher) {
      if ( ue_sensor->second.publisher->is_enabled_for_ros() ) {
        auto data_view_iter = buffer_list_view.begin();
        data_view_iter++;
        if (data_view_iter != buffer_list_view.end()) {
          ue_sensor->second.publisher->UpdateTransform(sensor_header);
          ue_sensor->second.publisher->UpdateSensorData(sensor_header, *data_view_iter);
          ue_sensor->second.publisher->Publish();
        }
        log_debug("Sensor Data to ROS data: frame.(", CurrentFrame(), ") stream.",
                  std::to_string(*sensor_actor_definition), " Processed.");

      } else {
        log_debug("Sensor Data to ROS data: frame.(", CurrentFrame(), ") stream.",
                  std::to_string(*sensor_actor_definition), std::to_string(*ue_sensor->second.publisher->_actor_name_definition), " not enabled for ROS. Dropping data.");
      }
    } else {
      log_error("Sensor Data to ROS data: frame.(", CurrentFrame(), ") stream.",
                std::to_string(*sensor_actor_definition), " not registered. Dropping data.");
    }

  } else {
    log_error("Sensor Data to ROS data: frame.(", CurrentFrame(), ") stream.", std::to_string(stream_id),
              " not registered. Dropping data.");
  }
}

void ROS2::ProcessDataFromUeSensorPostAction() {
  for (auto &ue_sensor : _ue_sensors) {
    if ( (ue_sensor.second.publisher != nullptr) ) {
      ue_sensor.second.publisher->UpdateSensorDataPostAction();
    } 
  }
  _world_publisher->UpdateSensorDataPostAction();
}


void ROS2::EnableForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  auto ue_sensor = _ue_sensors.find(stream_actor_id.stream_id);
  if (ue_sensor != _ue_sensors.end()) {
    if ( !ue_sensor->second.publisher->is_enabled_for_ros(stream_actor_id.actor_id) ) {
      log_debug("Enable Sensor for ROS: ",
                  std::to_string(*ue_sensor->second.publisher->_actor_name_definition));
      ue_sensor->second.publisher->enable_for_ros(stream_actor_id.actor_id);
    }
  }
}

void ROS2::DisableForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  auto ue_sensor = _ue_sensors.find(stream_actor_id.stream_id);
  if (ue_sensor != _ue_sensors.end()) {
    if ( ue_sensor->second.publisher->is_enabled_for_ros(stream_actor_id.actor_id) ) {
      log_debug("Disable Sensor for ROS: ",
                  std::to_string(*ue_sensor->second.publisher->_actor_name_definition));
      ue_sensor->second.publisher->disable_for_ros(stream_actor_id.actor_id);
    }
  }
}

bool ROS2::IsEnabledForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  auto ue_sensor = _ue_sensors.find(stream_actor_id.stream_id);
  if (ue_sensor != _ue_sensors.end()) {
    return ue_sensor->second.publisher->is_enabled_for_ros(stream_actor_id.actor_id);
  }
  return false;
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
