// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeWorldPublisher.h"

#include "carla/sensor/data/RawEpisodeState.h"
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
#include "carla/ros2/publishers/UeV2XPublisher.h"
#include "carla/ros2/publishers/UeV2XCustomPublisher.h"
#include "carla/ros2/publishers/VehiclePublisher.h"
#include "carla/ros2/subscribers/AckermannControlSubscriber.h"
#include "carla/ros2/subscribers/VehicleControlSubscriber.h"
#include "carla/ros2/types/EpisodeSettings.h"


#include "carla/ros2/types/Acceleration.h"
#include "carla/ros2/types/AngularVelocity.h"
#include "carla/ros2/types/Quaternion.h"
#include "carla/ros2/types/Speed.h"
#include "carla/ros2/types/VehicleAckermannControl.h"
#include "carla/ros2/types/VehicleControl.h"

namespace carla {
namespace ros2 {

    
UeWorldPublisher::UeWorldPublisher(carla::rpc::RpcServerInterface& carla_server,
                                   std::shared_ptr<ROS2NameRegistry> name_registry,
                                   std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition)
  : UePublisherBaseSensor(sensor_actor_definition, std::make_shared<TransformPublisher>()),
    _carla_server(carla_server),
    _name_registry(name_registry),
    _clock_publisher(std::make_shared<ClockPublisher>()),
    _world_info_publisher(std::make_shared<WorldInfoPublisher>(_carla_server)),
    _status_publisher(std::make_shared<CarlaStatusPublisher>()),
    _weather_publisher(std::make_shared<WeatherPublisher>(_carla_server)),
    _sensor_actor_list_publisher(std::make_shared<CarlaActorListPublisher>("sensor_list")),
    _actor_list_publisher(std::make_shared<CarlaActorListPublisher>("actor_list")),
    _objects_publisher(std::make_shared<ObjectsPublisher>(ObjectsPublisher::ObjectMode::DYNAMIC_PUBLISH_ALWAYS, "objects")),
    _objects_with_covariance_publisher(std::make_shared<ObjectsWithCovariancePublisher>()),
    _traffic_lights_publisher(std::make_shared<TrafficLightsPublisher>()),
    _traffic_light_actor_list_publisher(std::make_shared<CarlaActorListPublisher>("traffic_lights/actor_list")),
    _traffic_light_objects_publisher(std::make_shared<ObjectsPublisher>(ObjectsPublisher::ObjectMode::DYNAMIC_PUBLISH_ON_CHANGE, "traffic_lights/objects")),
    _traffic_sign_actor_list_publisher(std::make_shared<CarlaActorListPublisher>("traffic_signs/actor_list")),
    _traffic_sign_objects_publisher(std::make_shared<ObjectsPublisher>(ObjectsPublisher::ObjectMode::DYNAMIC_PUBLISH_ON_CHANGE, "traffic_signs/objects")),
    _environment_actor_list_publisher(std::make_shared<CarlaActorListPublisher>("environment/actor_list")),
    _environment_objects_publisher(std::make_shared<ObjectsPublisher>(ObjectsPublisher::ObjectMode::STATIC_PUBLISH_ONCE, "environment/objects")),
    _carla_control_subscriber(std::make_shared<CarlaControlSubscriber>(*this, _carla_server)),
    _sync_subscriber(std::make_shared<CarlaSynchronizationWindowSubscriber>(*this, _carla_server)),
    _weather_control_subscriber(std::make_shared<WeatherControlSubscriber>(*this, _carla_server)) {

  _dispatcher = _carla_server.GetDispatcher();
}

bool UeWorldPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  // add this to the list of sensors first
  auto sensor_ue = AddSensorUeInternal(GetSensorActorDefinition());
  sensor_ue->publisher=std::static_pointer_cast<UePublisherBaseSensor>(shared_from_this());

  _domain_participant_impl = domain_participant;
  _initialized =
                 _transform_publisher->Init(domain_participant) &&
                 _clock_publisher->Init(domain_participant) &&
                 _world_info_publisher->Init(domain_participant) && 
                 _status_publisher->Init(domain_participant) &&
                 _weather_publisher->Init(domain_participant) &&
                 _sensor_actor_list_publisher->Init(_domain_participant_impl) &&
                 _actor_list_publisher->Init(domain_participant) && 
                 _objects_publisher->Init(domain_participant) &&
                 _objects_with_covariance_publisher->Init(domain_participant) && 
                 _traffic_lights_publisher->Init(domain_participant) &&
                 _traffic_light_actor_list_publisher->Init(_domain_participant_impl) &&
                 _traffic_light_objects_publisher->Init(domain_participant) && 
                 _traffic_sign_actor_list_publisher->Init(_domain_participant_impl) &&
                 _traffic_sign_objects_publisher->Init(domain_participant) &&
                 _environment_actor_list_publisher->Init(domain_participant) &&
                 _environment_objects_publisher->Init(domain_participant) && 
                 _carla_control_subscriber->Init(domain_participant) &&
                 _weather_control_subscriber->Init(domain_participant) &&
                 _sync_subscriber->Init(domain_participant);
  return _initialized;
}

void UeWorldPublisher::Cleanup() {
  _objects.clear();
  _vehicles.clear();
  _walkers.clear();
  _traffic_lights.clear();
  _traffic_signs.clear();
  _ue_sensors.clear();
}

bool UeWorldPublisher::Publish() {
  if (!_initialized) {
    return false;
  }
  return _clock_publisher->Publish() && _world_info_publisher->Publish() && _weather_publisher->Publish();
}

void UeWorldPublisher::ProcessMessages() {
  if (!_initialized) {
    return;
  }

  _carla_control_subscriber->ProcessMessages();
  _sync_subscriber->ProcessMessages();
  _weather_publisher->ProcessMessages();
  _world_info_publisher->ProcessMessages();
  _weather_control_subscriber->ProcessMessages();
  for (auto& vehicle : _vehicles) {
    vehicle.second._vehicle_controller->ProcessMessages();
    vehicle.second._vehicle_ackermann_controller->ProcessMessages();
    vehicle.second._actor_set_transform_subscriber->ProcessMessages();
    vehicle.second._vehicle_publisher->ProcessMessages();
  }
  for (auto& walker : _walkers) {
    walker.second._walker_controller->ProcessMessages();
  }

  UpdateAndPublishEnvironmentObjects();
  UpdateAndPublishStatus();
}

void UeWorldPublisher::UpdateSensorDataPreAction() {
  for (auto &ue_sensor : _ue_sensors) {
    if (ue_sensor.second.publisher_expected && (ue_sensor.second.publisher == nullptr)) {
      CreateSensorUePublisher(ue_sensor.second);
    }
    if (ue_sensor.second.publisher != nullptr) {
      if (ue_sensor.second.publisher->SubscribersConnected() && ue_sensor.second.session == nullptr) {
        ue_sensor.second.session = std::make_shared<ROS2Session>(ue_sensor.first);
        log_debug("UeWorldPublisher::UpdateSensorDataPreAction[", std::to_string(*ue_sensor.second.sensor_actor_definition),
                    "]: Registering session");
        _dispatcher->RegisterSession(ue_sensor.second.session);
      } else if (!ue_sensor.second.publisher->SubscribersConnected() && ue_sensor.second.session != nullptr) {
        log_debug("UeWorldPublisher::UpdateSensorDataPreAction[", std::to_string(*ue_sensor.second.sensor_actor_definition),
                    "]: Deregistering session");
        _dispatcher->DeregisterSession(ue_sensor.second.session);
        ue_sensor.second.session.reset();
      }
    }
  }

  for (auto &ue_sensor : _ue_sensors) {
    if ( (ue_sensor.second.publisher != nullptr) && (ue_sensor.first != GetSensorActorDefinition()->stream_id) ) {
      ue_sensor.second.publisher->UpdateSensorDataPreAction();
    } 
  }

  if (_sensors_changed) {
    _sensors_changed = false;
    carla_msgs::msg::CarlaActorList actor_list;
    for (auto &ue_sensor : _ue_sensors) {
      if (ue_sensor.second.sensor_actor_definition->id != 0) {
        actor_list.actors().push_back(ue_sensor.second.sensor_actor_definition->carla_actor_info(_name_registry));
      }
    }
    _sensor_actor_list_publisher->UpdateCarlaActorList(actor_list);
    _sensor_actor_list_publisher->Publish();
  }
}

void UeWorldPublisher::ProcessDataFromUeSensor(carla::streaming::detail::stream_id_type const stream_id,
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
          if (ue_sensor->second.publisher->do_publish_tf() ) {
            ue_sensor->second.publisher->UpdateTransform(sensor_header);
          }
          ue_sensor->second.publisher->UpdateSensorData(sensor_header, *data_view_iter);
          ue_sensor->second.publisher->Publish();
        }
        log_verbose("Sensor Data to ROS data: frame.(", CurrentFrame(), ") stream.",
                  std::to_string(*sensor_actor_definition), " Processed.");

      } else {
        log_verbose("Sensor Data to ROS data: frame.(", CurrentFrame(), ") stream.",
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

void UeWorldPublisher::UpdateSensorDataPostAction() {
  if (!_initialized) {
    return;
  }

  for (auto &ue_sensor : _ue_sensors) {
    if ( (ue_sensor.second.publisher != nullptr)  && (ue_sensor.first != GetSensorActorDefinition()->stream_id) ) {
      ue_sensor.second.publisher->UpdateSensorDataPostAction();
    } 
  }

  UpdateAndPublishStatus();

  _transform_publisher->Publish();
  _actor_list_publisher->Publish();
  _objects_publisher->Publish();
  _objects_with_covariance_publisher->Publish();
  _traffic_lights_publisher->Publish();
  _traffic_light_actor_list_publisher->Publish();
  _traffic_light_objects_publisher->Publish();
  _traffic_sign_actor_list_publisher->Publish();
  _traffic_sign_objects_publisher->Publish();
}

void UeWorldPublisher::CreateSensorUePublisher(UeSensor &sensor) {
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
    case types::PublisherSensorType::WorldObserver:
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
      log_error("UeWorldPublisher::CreateSensorUePublisher[", std::to_string(*sensor.sensor_actor_definition),
                "]: Not a UE sensor or no publisher implemented yet");
    }
  }
  if (sensor.publisher != nullptr) {
    if (!sensor.publisher->Init(_domain_participant_impl)) {
      log_error("UeWorldPublisher::CreateSensorUePublisher[", std::to_string(*sensor.sensor_actor_definition),
                "]: Failed to init publisher");
    } else {
      log_debug("UeWorldPublisher::CreateSensorUePublisher[", std::to_string(*sensor.sensor_actor_definition),
                  "]: Publisher initialized");
    }
  }
}

UeWorldPublisher::UeSensor* UeWorldPublisher::AddSensorUeInternal(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition) {
  auto insert_result = _ue_sensors.insert({sensor_actor_definition->stream_id, UeSensor(sensor_actor_definition)});
  if (!insert_result.second) {
    log_warning("UeWorldPublisher::AddSensorUe(", std::to_string(*sensor_actor_definition),
                "): Sensor already_registered. Ignoring");
    return nullptr;
  }
  _sensors_changed = true;
  return &insert_result.first->second;
}    

void UeWorldPublisher::AddSensorUe(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                       carla::ros2::types::ActorSetTransformCallback actor_set_transform_callback) {
  auto ue_sensor = AddSensorUeInternal(sensor_actor_definition);
  if ( ue_sensor != nullptr ) {
    ue_sensor->actor_set_transform_callback = actor_set_transform_callback;
  }
}

void UeWorldPublisher::AddV2XCustomSensorUe(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition, 
    carla::ros2::types::V2XCustomSendCallback v2x_custom_send_callback) {
  auto ue_sensor = AddSensorUeInternal(sensor_actor_definition);
  if ( ue_sensor != nullptr ) {
    ue_sensor->v2x_custom_send_callback = v2x_custom_send_callback;
  }
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
  _traffic_light_objects_publisher->AddObject(*object);
  _traffic_lights_changed = true;

  auto traffic_light_publisher = std::make_shared<TrafficLightPublisher>(traffic_light_actor_definition,
                                                                         _traffic_light_objects_publisher, _traffic_lights_publisher);
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
  _traffic_sign_objects_publisher->AddObject(*object);
  _traffic_signs_changed = true;

  auto traffic_sign_publisher =
      std::make_shared<TrafficSignPublisher>(traffic_sign_actor_definition, _traffic_sign_objects_publisher);
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
  auto vehicle_iter = _vehicles.find(actor);
  if ( vehicle_iter != _vehicles.end() ) {
    log_debug("ROS2::RemoveVehicleUe(", std::to_string(
      *std::static_pointer_cast<carla::ros2::types::VehicleActorDefinition>(vehicle_iter->second._vehicle_publisher->_actor_name_definition)), ")");
    _vehicles.erase(vehicle_iter);
    _objects_changed = true;
  }
  auto walker_iter = _walkers.find(actor);
  if ( walker_iter != _walkers.end() ) {
    log_debug("ROS2::RemoveWalkerUe(", std::to_string(
      *std::static_pointer_cast<carla::ros2::types::WalkerActorDefinition>(walker_iter->second._walker_publisher->_actor_name_definition)), ")");
    _walkers.erase(walker_iter);
    _objects_changed = true;
  }
  auto traffic_light_iter = _traffic_lights.find(actor);
  if ( traffic_light_iter != _traffic_lights.end() ) {
    log_debug("ROS2::RemoveTrafficLightUe(", std::to_string(
      *std::static_pointer_cast<carla::ros2::types::TrafficLightActorDefinition>(traffic_light_iter->second._traffic_light_publisher->_actor_name_definition)), ")");
    _traffic_lights.erase(traffic_light_iter);
    _traffic_lights_changed = true;
  }
  _traffic_lights_publisher->RemoveTrafficLight(actor);
  _traffic_light_objects_publisher->RemoveObject(actor);

  auto traffic_sign_iter = _traffic_signs.find(actor);
  if ( traffic_sign_iter != _traffic_signs.end() ) {
    log_debug("ROS2::RemoveTrafficSignUe(", std::to_string(
      *std::static_pointer_cast<carla::ros2::types::TrafficSignActorDefinition>(traffic_sign_iter->second._traffic_sign_publisher->_actor_name_definition)), ")");
    _traffic_signs.erase(traffic_sign_iter);
    _traffic_signs_changed = true;
  }
  _traffic_sign_objects_publisher->RemoveObject(actor);

  auto sensor_iter = find_ue_sensor(actor);
  if (sensor_iter!=_ue_sensors.end()) {
    log_debug("ROS2::RemoveSensorUe(", std::to_string(*sensor_iter->second.sensor_actor_definition), ")");
    _ue_sensors.erase(sensor_iter);
    _sensors_changed = true;
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
    _status_publisher->UpdateCarlaStatus(status);

    _status_publisher->Publish();
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
    _world_info_publisher->SetMapUpdated();
  }

  for (auto const& actor_dynamic_state : buffer_data_2_vector(buffer_view)) {
    bool actor_processed = false;
    carla::ros2::types::Transform transform(actor_dynamic_state.transform, actor_dynamic_state.quaternion);
  
   auto object_it = _objects.find(actor_dynamic_state.id);
   if (object_it != _objects.end()) {
      object_it->second->UpdateObject(_timestamp, actor_dynamic_state);

      std::shared_ptr<const carla::ros2::types::Object> object = object_it->second;

      auto vehicle_it = _vehicles.find(actor_dynamic_state.id);
      if (vehicle_it != _vehicles.end()) {
        actor_processed=true;
        UeVehicle& ue_vehicle = vehicle_it->second;
        auto publisher = ue_vehicle._vehicle_publisher;
        if ( publisher->is_enabled_for_ros() ) {
          if ( publisher->do_publish_tf() ) {
            publisher->UpdateTransform(_timestamp, transform);
          }
          publisher->UpdateVehicle(object, actor_dynamic_state);
          publisher->Publish();
        }
      }

      if ( !actor_processed ) {
        auto walker_it = _walkers.find(actor_dynamic_state.id);
        if (walker_it != _walkers.end()) {
          actor_processed=true;
          UeWalker& ue_walker = walker_it->second;
          auto publisher = ue_walker._walker_publisher;
          if ( publisher->is_enabled_for_ros() ) {
            if ( publisher->do_publish_tf() ) {
              publisher->UpdateTransform(_timestamp, transform);
            }
            publisher->UpdateWalker(object, actor_dynamic_state);
            publisher->Publish();
          }
        }
      }

      if ( !actor_processed ) {
        auto traffic_sign_it = _traffic_signs.find(actor_dynamic_state.id);
        if (traffic_sign_it != _traffic_signs.end()) {
          actor_processed=true;
          UeTrafficSign& ue_traffic_sign = traffic_sign_it->second;
          auto publisher = ue_traffic_sign._traffic_sign_publisher;
          if ( publisher->is_enabled_for_ros() ) {
            publisher->UpdateTrafficSign(object, actor_dynamic_state);
            publisher->Publish();
          }
        }
      }

      if ( !actor_processed ) {
        auto traffic_light_it = _traffic_lights.find(actor_dynamic_state.id);
        if (traffic_light_it != _traffic_lights.end()) {
          actor_processed=true;
          UeTrafficLight& ue_traffic_light = traffic_light_it->second;
          auto publisher = ue_traffic_light._traffic_light_publisher;
          if ( publisher->is_enabled_for_ros() ) {
            publisher->UpdateTrafficLight(object, actor_dynamic_state);
            publisher->Publish();
          }
        }
      }
    }

    if ( !actor_processed ) {
      auto sensor_it = find_ue_sensor(actor_dynamic_state.id);
      if ( sensor_it != _ue_sensors.end() ) {
        actor_processed=true;
        // store the transform to be able to calculate relative transform in case of nested sensors
        sensor_it->second.transform = transform;
      }
    }
  }

  for (auto &ue_sensor : _ue_sensors) {
    if ( (ue_sensor.second.publisher != nullptr) && 
         (ue_sensor.first != GetSensorActorDefinition()->stream_id) &&
         (ue_sensor.second.publisher->is_enabled_for_ros()) &&
         (ue_sensor.second.publisher->do_publish_tf()) &&
         (!ue_sensor.second.publisher->SubscribersConnected())) {
      // update sensor transform of sensors not subscribed, as their data stream is not deployed
      auto const parent_actor_id = ue_sensor.second.publisher->get_parent_actor_id();
      auto const parent_transform = get_transform(parent_actor_id);
      auto const relative_transform = ue_sensor.second.transform.GetRelativeTransform(parent_transform);
      ue_sensor.second.publisher->UpdateTransform(_timestamp, relative_transform);
    }
  }

  if (_objects_changed) {
    _objects_changed = false;
    carla_msgs::msg::CarlaActorList actor_list;
    for (auto const& vehicle : _vehicles) {
      actor_list.actors().push_back(vehicle.second._vehicle_publisher->carla_actor_info(_name_registry));
    }
    for (auto const& walker : _walkers) {
      actor_list.actors().push_back(walker.second._walker_publisher->carla_actor_info(_name_registry));
    }
    _actor_list_publisher->UpdateCarlaActorList(actor_list);
  }
  if (_traffic_lights_changed) {
    _traffic_lights_changed = false;
    carla_msgs::msg::CarlaActorList actor_list;
    for (auto const& traffic_light : _traffic_lights) {
      actor_list.actors().push_back(traffic_light.second._traffic_light_publisher->carla_actor_info(_name_registry));
    }
    _traffic_light_actor_list_publisher->UpdateCarlaActorList(actor_list);
  }
  _traffic_light_objects_publisher->UpdateHeader(_timestamp.time());

  if (_traffic_signs_changed) {
    _traffic_signs_changed = false;
    carla_msgs::msg::CarlaActorList actor_list;
    for (auto const& traffic_sign : _traffic_signs) {
      actor_list.actors().push_back(traffic_sign.second._traffic_sign_publisher->carla_actor_info(_name_registry));
    }
    _traffic_sign_actor_list_publisher->UpdateCarlaActorList(actor_list);
  }
  _traffic_sign_objects_publisher->UpdateHeader(_timestamp.time());
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
  auto sensor_it = find_ue_sensor(actor_id);
  if (sensor_it != _ue_sensors.end()) {
    if ( !sensor_it->second.publisher->is_enabled_for_ros() ) {
      log_debug("Enable Sensor for ROS: ",
                  std::to_string(*sensor_it->second.publisher->_actor_name_definition));
      sensor_it->second.publisher->enable_for_ros();
    }
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
  auto sensor_it = find_ue_sensor(actor_id);
  if (sensor_it != _ue_sensors.end()) {
    if ( sensor_it->second.publisher->is_enabled_for_ros() ) {
      log_debug("Disable Sensor for ROS: ",
                  std::to_string(*sensor_it->second.publisher->_actor_name_definition));
      sensor_it->second.publisher->disable_for_ros();
    }
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
  auto sensor_it = find_ue_sensor(actor_id);
  if (sensor_it != _ue_sensors.end()) {
    return sensor_it->second.publisher->is_enabled_for_ros();
  }
  return false;
}

void UeWorldPublisher::UpdateAndPublishEnvironmentObjects() 
{
  if (!_initialized) 
  {
    return;
  }

  // The world observer enabled_for_ros flag is matching the ROS2TopicVisibility configuration value which is used to decide
  // on the publication of the environment objects at this point
  // Otherwhise one might have to define some enable_for_ros() calls for e.g. different carla::rpc::CityObjectLabel parameters
  // if a fine granular selection will be required.
  bool const ros2_topic_visibility = PublisherBase::is_enabled_for_ros(0);
  if ( !ros2_topic_visibility)
  {
    return;
  }

  if ( !_environment_objects_initialized ) 
  {
    carla_msgs::msg::CarlaActorList actor_list;
    for (auto label: {carla::rpc::CityObjectLabel::Any})
    {
      auto response = _carla_server.call_get_environment_objects(static_cast<uint8_t>(label));
      if ( !response.HasError() )
      {
        auto const &environment_objects = response.Get();
        for (auto const& env_object : environment_objects)  
        {
          auto const object = carla::ros2::types::Object(env_object, ros2_topic_visibility);
          _environment_objects_publisher->AddObject(object);
          actor_list.actors().push_back(object.carla_actor_info());
        }
      }
    }
    if (!actor_list.actors().empty()) {
      _environment_actor_list_publisher->UpdateCarlaActorList(actor_list);
      _environment_objects_publisher->UpdateHeader(_timestamp.time());
      
      _environment_objects_initialized = _environment_actor_list_publisher->Publish();
      _environment_objects_initialized &= _environment_objects_publisher->Publish();

      log_debug("ROS2::UpdateAndPublishEnvironmentObjects() = ", actor_list.actors().size(), _environment_objects_initialized);
    }
  }
}

void UeWorldPublisher::AttachActors(ActorId const child, ActorId const parent) {
  log_debug("UeWorldPublisher::AttachActors[", child, "]: parent=", parent);
  _name_registry->AttachActors(child, parent);
  auto find_result = find_ue_sensor(child);
  if ( find_result != _ue_sensors.end()) {
    UeSensor &sensor = find_result->second;
    if (sensor.publisher) {
      log_error("UeWorldPublisher::AttachActors[", std::to_string(*sensor.sensor_actor_definition),
                "]: Sensor attached to parent ", parent,
                ". Sensor has already a running publisher with base topic name ", sensor.publisher->get_topic_name(),
                " has to be destroyed due to re-attachment");
      sensor.publisher.reset();
    }
    _sensors_changed = true;
  }
}

std::unordered_map<carla::streaming::detail::stream_id_type, UeWorldPublisher::UeSensor>::iterator 
UeWorldPublisher::find_ue_sensor(ActorId actor_id)
{
  auto find_result = std::find_if(_ue_sensors.begin(), _ue_sensors.end(), 
    [actor_id](std::pair<carla::streaming::detail::stream_id_type, UeWorldPublisher::UeSensor> element) {
      return actor_id == element.second.sensor_actor_definition->id;
    });
  return find_result;
}

std::unordered_map<carla::streaming::detail::stream_id_type, UeWorldPublisher::UeSensor>::const_iterator 
UeWorldPublisher::find_ue_sensor(ActorId actor_id)const
{
  auto find_result = std::find_if(_ue_sensors.begin(), _ue_sensors.end(), 
    [actor_id](std::pair<carla::streaming::detail::stream_id_type, UeWorldPublisher::UeSensor> element) {
      return actor_id == element.second.sensor_actor_definition->id;
    });
  return find_result;
}

carla::ros2::types::Transform UeWorldPublisher::get_transform(ActorId actor_id) {
  auto object_it = _objects.find(actor_id);
  if (object_it != _objects.end()) {
    return object_it->second->Transform();
  }
  auto sensor_it = find_ue_sensor(actor_id);
  if ( sensor_it != _ue_sensors.end() ) {
    return sensor_it->second.transform;
  }
  return carla::ros2::types::Transform();
}


}  // namespace ros2
}  // namespace carla
