// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
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

#include "carla/ros2/publishers/ObjectPublisher.h"
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
#include "carla/ros2/publishers/VehiclePublisher.h"

#include "carla/ros2/services/DestroyObjectService.h"
#include "carla/ros2/services/GetBlueprintsService.h"
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
                  carla::streaming::detail::stream_id_type const world_observer_stream_id) {
  _enabled = true;
  _carla_server = carla_server;
  _name_registry = std::make_shared<ROS2NameRegistry>();
  _dispatcher = _carla_server->GetDispatcher();
  _domain_participant_impl = std::make_shared<DdsDomainParticipantImpl>();
  _world_observer_sensor_actor_definition = std::make_shared<carla::ros2::types::SensorActorDefinition>(
      carla::ros2::types::ActorNameDefinition(0, "world_observer", "world_observer"),
      carla::ros2::types::PublisherSensorType::WorldObserver, world_observer_stream_id);
  log_warning("ROS2 enabled");
}

void ROS2::NotifyInitGame() {
  log_warning("ROS2 NotifyInitGame start");

  // The world is crucial and has to be instanciated immediately
  if (AddSensorUe(_world_observer_sensor_actor_definition)) {
    PreTickAction();
  }
  if (_world_publisher != nullptr) {
    _transform_publisher = _world_publisher->GetTransformPublisher();
  }
  log_warning("ROS2 NotifyInitGame end");
}

void ROS2::NotifyBeginEpisode() {
  log_warning("ROS2 NotifyBeginEpisode start");

  auto spwan_object_service = std::make_shared<carla::ros2::SpawnObjectService>(
      *_carla_server, carla::ros2::types::ActorNameDefinition::CreateFromRoleName("world/spawn_object"));
  spwan_object_service->Init(_domain_participant_impl);
  _services.push_back(spwan_object_service);

  auto destroy_object_service = std::make_shared<carla::ros2::DestroyObjectService>(
      *_carla_server, carla::ros2::types::ActorNameDefinition::CreateFromRoleName("world/destroy_object"));
  destroy_object_service->Init(_domain_participant_impl);
  _services.push_back(destroy_object_service);

  auto get_blueprints_service = std::make_shared<carla::ros2::GetBlueprintsService>(
      *_carla_server, carla::ros2::types::ActorNameDefinition::CreateFromRoleName("world/get_blueprints"));
  get_blueprints_service->Init(_domain_participant_impl);
  _services.push_back(get_blueprints_service);

  log_warning("ROS2 NotifyBeginEpisode end");
}

void ROS2::NotifyEndEpisode() {
  log_warning("ROS2 NotifyEndEpisode start");
  _services.clear();
  _ue_sensors.clear();
  _name_registry->Clear();
  log_warning("ROS2 NotifyEndEpisode done");
}

void ROS2::NotifyEndGame() {
  log_warning("ROS2 NotifyEndGame start");
  NotifyEndEpisode();
  _world_publisher.reset();
  _transform_publisher.reset();
  log_warning("ROS2 NotifyEndGame end");
}

void ROS2::Disable() {
  NotifyEndEpisode();
  NotifyEndGame();
  _world_observer_sensor_actor_definition.reset();
  _dispatcher.reset();
  _domain_participant_impl.reset();
  _name_registry.reset();
  _enabled = false;
  log_warning("ROS2 disabled");
}

void ROS2::AddVehicleUe(std::shared_ptr<carla::ros2::types::VehicleActorDefinition> vehicle_actor_definition,
                        carla::ros2::types::VehicleControlCallback vehicle_control_callback,
                        carla::ros2::types::VehicleAckermannControlCallback vehicle_ackermann_control_callback) {
  log_warning("ROS2::AddVehicleUe(", std::to_string(*vehicle_actor_definition), ")");
  _world_publisher->AddVehicleUe(vehicle_actor_definition, vehicle_control_callback,
                                 vehicle_ackermann_control_callback);
}

void ROS2::AddWalkerUe(std::shared_ptr<carla::ros2::types::WalkerActorDefinition> walker_actor_definition,
                       carla::ros2::types::WalkerControlCallback walker_control_callback) {
  log_warning("ROS2::AddWalkerUe(", std::to_string(*walker_actor_definition), ")");
  _world_publisher->AddWalkerUe(walker_actor_definition, walker_control_callback);
}

void ROS2::AddTrafficLightUe(
    std::shared_ptr<carla::ros2::types::TrafficLightActorDefinition> traffic_light_actor_definition) {
  log_warning("ROS2::AddTrafficLightUe(", std::to_string(*traffic_light_actor_definition), ")");
  _world_publisher->AddTrafficLightUe(traffic_light_actor_definition);
}

void ROS2::AddTrafficSignUe(
    std::shared_ptr<carla::ros2::types::TrafficSignActorDefinition> traffic_sign_actor_definition) {
  log_warning("ROS2::AddTrafficSignUe(", std::to_string(*traffic_sign_actor_definition), ")");
  _world_publisher->AddTrafficSignUe(traffic_sign_actor_definition);
}

bool ROS2::AddSensorUe(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition) {
  log_warning("ROS2::AddSensorUe(", std::to_string(*sensor_actor_definition), ")");

  auto insert_result = _ue_sensors.insert({sensor_actor_definition->stream_id, UeSensor(sensor_actor_definition)});
  if (!insert_result.second) {
    log_warning("ROS2::AddSensorUe(", std::to_string(*sensor_actor_definition),
                "): Sensor already_registered. Ignoring");
    return false;
  }

  return true;
}

void ROS2::AttachActors(ActorId const child, ActorId const parent) {
  log_warning("ROS2::AttachActors[", child, "]: parent=", parent);
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
      break;
    }
  }
}

void ROS2::CreateSensorUePublisher(UeSensor &sensor) {
  // Create the respective sensor publisher
  switch (sensor.sensor_actor_definition->sensor_type) {
    case types::PublisherSensorType::CollisionSensor: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeCollisionPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::DepthCamera: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeDepthCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::NormalsCamera: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeNormalsCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::DVSCamera: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeDVSCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::GnssSensor: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeGNSSPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::InertialMeasurementUnit: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeIMUPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::OpticalFlowCamera: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeOpticalFlowCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::Radar: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeRadarPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::RayCastSemanticLidar: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeSemanticLidarPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::RayCastLidar: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeLidarPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::SceneCaptureCamera: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeRGBCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::SemanticSegmentationCamera: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeSSCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::InstanceSegmentationCamera: {
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(
          std::make_shared<UeISCameraPublisher>(sensor.sensor_actor_definition, _transform_publisher));
    } break;
    case types::PublisherSensorType::WorldObserver: {
      _world_publisher =
          std::make_shared<UeWorldPublisher>(*_carla_server, _name_registry, sensor.sensor_actor_definition);
      sensor.publisher = std::static_pointer_cast<UePublisherBaseSensor>(_world_publisher);
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
      log_warning("ROS2::CreateSensorUePublisher[", std::to_string(*sensor.sensor_actor_definition),
                  "]: Publisher initialized");
    }
  }
}

void ROS2::RemoveActor(ActorId const actor) {
  for (auto iter = _ue_sensors.begin(); iter != _ue_sensors.end(); /*no update of iter*/) {
    if (iter->second.sensor_actor_definition->id == actor) {
      iter = _ue_sensors.erase(iter);
    } else {
      ++iter;
    }
  }
  _world_publisher->RemoveActor(actor);
}

void ROS2::PreTickAction() {
  for (auto &ue_sensor : _ue_sensors) {
    if (ue_sensor.second.publisher_expected && (ue_sensor.second.publisher == nullptr)) {
      CreateSensorUePublisher(ue_sensor.second);
    }
    if (ue_sensor.second.publisher != nullptr) {
      if (ue_sensor.second.publisher->SubscribersConnected() && ue_sensor.second.session == nullptr) {
        ue_sensor.second.session = std::make_shared<ROS2Session>(ue_sensor.first);
        log_warning("ROS2::PreTickAction[", std::to_string(*ue_sensor.second.sensor_actor_definition),
                    "]: Registering session");
        _dispatcher->RegisterSession(ue_sensor.second.session);
      } else if (!ue_sensor.second.publisher->SubscribersConnected() && ue_sensor.second.session != nullptr) {
        log_warning("ROS2::PreTickAction[", std::to_string(*ue_sensor.second.sensor_actor_definition),
                    "]: Deregistering session");
        _dispatcher->DeregisterSession(ue_sensor.second.session);
        ue_sensor.second.session.reset();
      }
    }
  }

  _world_publisher->PreTickAction();
}

void ROS2::ProcessMessages() {
  for (auto service : _services) {
    service->CheckRequest();
  }
  _world_publisher->ProcessMessages();
}

void ROS2::PostTickAction() {
  _world_publisher->PostTickAction();
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
      auto data_view_iter = buffer_list_view.begin();
      data_view_iter++;
      if (data_view_iter != buffer_list_view.end()) {
        ue_sensor->second.publisher->UpdateTransform(sensor_header);
        ue_sensor->second.publisher->UpdateSensorData(sensor_header, *data_view_iter);
        ue_sensor->second.publisher->Publish();
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

uint64_t ROS2::CurrentFrame() const {
  return (_world_publisher != nullptr) ? _world_publisher->CurrentFrame() : 0u;
}

carla::ros2::types::Timestamp const &ROS2::CurrentTimestamp() const {
  static carla::ros2::types::Timestamp const dummy;
  return (_world_publisher != nullptr) ? _world_publisher->CurrentTimestamp() : dummy;
}

}  // namespace ros2
}  // namespace carla
