// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h"
#include "carla/ros2/ROS2.h"
#include "carla/geom/GeoLocation.h"
#include "carla/geom/Vector3D.h"
#include "carla/sensor/data/DVSEvent.h"
#include "carla/sensor/data/LidarData.h"
#include "carla/sensor/data/SemanticLidarData.h"
#include "carla/sensor/data/RadarData.h"
#include "carla/sensor/data/Image.h"
#include "carla/sensor/s11n/ImageSerializer.h"
#include "carla/sensor/s11n/SensorHeaderSerializer.h"

#include "publishers/CarlaPublisher.h"
#include "publishers/CarlaClockPublisher.h"
#include "publishers/CarlaRGBCameraPublisher.h"
#include "publishers/CarlaDepthCameraPublisher.h"
#include "publishers/CarlaNormalsCameraPublisher.h"
#include "publishers/CarlaOpticalFlowCameraPublisher.h"
#include "publishers/CarlaSSCameraPublisher.h"
#include "publishers/CarlaISCameraPublisher.h"
#include "publishers/CarlaDVSCameraPublisher.h"
#include "publishers/CarlaLidarPublisher.h"
#include "publishers/CarlaSemanticLidarPublisher.h"
#include "publishers/CarlaRadarPublisher.h"
#include "publishers/CarlaIMUPublisher.h"
#include "publishers/CarlaGNSSPublisher.h"
#include "publishers/CarlaMapSensorPublisher.h"
#include "publishers/CarlaSpeedometerSensor.h"
#include "publishers/CarlaTransformPublisher.h"
#include "publishers/CarlaCollisionPublisher.h"
#include "publishers/CarlaLineInvasionPublisher.h"

#include "subscribers/CarlaSubscriber.h"
#include "subscribers/CarlaEgoVehicleControlSubscriber.h"

#include <vector>

namespace carla {
namespace ros2 {

// static fields
std::shared_ptr<ROS2> ROS2::_instance;

// list of sensors (should be equal to the list of SensorsRegistry
enum ESensors {
  CollisionSensor,
  DepthCamera,
  NormalsCamera,
  DVSCamera,
  GnssSensor,
  InertialMeasurementUnit,
  LaneInvasionSensor,
  ObstacleDetectionSensor,
  OpticalFlowCamera,
  Radar,
  RayCastSemanticLidar,
  RayCastLidar,
  RssSensor,
  SceneCaptureCamera,
  SemanticSegmentationCamera,
  InstanceSegmentationCamera,
  WorldObserver,
  CameraGBufferUint8,
  CameraGBufferFloat
};

void ROS2::Enable(bool enable) {
  _enabled = enable;
  log_info("ROS2 enabled: ", _enabled);
  _clock_publisher = std::make_shared<CarlaClockPublisher>("clock", "");
  _clock_publisher->Init();
}

void ROS2::SetFrame(uint64_t frame) {
  _frame = frame;
   //log_info("ROS2 new frame: ", _frame);
   if (_controller) {
    void* actor = _controller->GetVehicle();
    if (_controller->IsAlive()) {
      if (_controller->HasNewMessage()) {
        auto it = _actor_callbacks.find(actor);
        if (it != _actor_callbacks.end()) {
          VehicleControl control = _controller->GetMessage();
          it->second(actor, control);
        }
      }
    } else {
      RemoveActorCallback(actor);
    }
   }
}

void ROS2::SetTimestamp(double timestamp) {
  double integral;
  const double fractional = modf(timestamp, &integral);
  const double multiplier = 1000000000.0;
  _seconds = static_cast<int32_t>(integral);
  _nanoseconds = static_cast<uint32_t>(fractional * multiplier);
  _clock_publisher->SetData(_seconds, _nanoseconds);
  _clock_publisher->Publish();
   //log_info("ROS2 new timestamp: ", _timestamp);
}

void ROS2::AddActorRosName(void *actor, std::string ros_name) {
  _actor_ros_name.insert({actor, ros_name});
}

void ROS2::AddActorParentRosName(void *actor, void* parent) {
  auto it = _actor_parent_ros_name.find(actor);
  if (it != _actor_parent_ros_name.end()) {
    it->second.push_back(parent);
  } else {
    _actor_parent_ros_name.insert({actor, {parent}});
  }
}

void ROS2::RemoveActorRosName(void *actor) {
  _actor_ros_name.erase(actor);
  _actor_parent_ros_name.erase(actor);

  _publishers.erase(actor);
  _transforms.erase(actor);
}

void ROS2::UpdateActorRosName(void *actor, std::string ros_name) {
  auto it = _actor_ros_name.find(actor);
  if (it != _actor_ros_name.end()) {
    it->second = ros_name;
  }
}

std::string ROS2::GetActorRosName(void *actor) {
  auto it = _actor_ros_name.find(actor);
  if (it != _actor_ros_name.end()) {
    return it->second;
  } else {
    return std::string("");
  }
}

std::string ROS2::GetActorParentRosName(void *actor) {
  auto it = _actor_parent_ros_name.find(actor);
  if (it != _actor_parent_ros_name.end())
  {
    const std::string current_actor_name = GetActorRosName(actor);
    std::string parent_name;
    for (auto parent_it = it->second.cbegin(); parent_it != it->second.cend(); ++parent_it)
    {
      const std::string name = GetActorRosName(*parent_it);
      if (name == current_actor_name)
      {
        continue;
      }
      if (name.empty())
      {
        continue;
      }
      parent_name = name + '/' + parent_name;
    }
    if (parent_name.back() == '/')
      parent_name.pop_back();
    return parent_name;
  }
  else
    return std::string("");
}

void ROS2::AddActorCallback(void* actor, std::string ros_name, ActorCallback callback) {
  _actor_callbacks.insert({actor, std::move(callback)});

  _controller.reset();
  _controller = std::make_shared<CarlaEgoVehicleControlSubscriber>(actor, ros_name.c_str());
  _controller->Init();
}

void ROS2::RemoveActorCallback(void* actor) {
  _controller.reset();
  _actor_callbacks.erase(actor);
}

std::pair<std::shared_ptr<CarlaPublisher>, std::shared_ptr<CarlaTransformPublisher>> ROS2::GetOrCreateSensor(int type, carla::streaming::detail::stream_id_type id, void* actor) {
  auto it_publishers = _publishers.find(actor);
  auto it_transforms = _transforms.find(actor);
  std::shared_ptr<CarlaPublisher> publisher {};
  std::shared_ptr<CarlaTransformPublisher> transform {};
  if (it_publishers != _publishers.end()) {
    publisher = it_publishers->second;
    if (it_transforms != _transforms.end()) {
      transform = it_transforms->second;
    }
  } else {
    //Sensor not found, creating one of the given type
    const std::string string_id = std::to_string(id);
    std::string ros_name = GetActorRosName(actor);
    std::string parent_ros_name = GetActorParentRosName(actor);
    switch(type) {
      case ESensors::CollisionSensor: {
        if (ros_name == "collision__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaCollisionPublisher> new_publisher = std::make_shared<CarlaCollisionPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::DepthCamera: {
        if (ros_name == "depth__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaDepthCameraPublisher> new_publisher = std::make_shared<CarlaDepthCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::NormalsCamera: {
        if (ros_name == "normals__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaNormalsCameraPublisher> new_publisher = std::make_shared<CarlaNormalsCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::DVSCamera: {
        if (ros_name == "dvs__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaDVSCameraPublisher> new_publisher = std::make_shared<CarlaDVSCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::GnssSensor: {
        if (ros_name == "gnss__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaGNSSPublisher> new_publisher = std::make_shared<CarlaGNSSPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::InertialMeasurementUnit: {
        if (ros_name == "imu__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaIMUPublisher> new_publisher = std::make_shared<CarlaIMUPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::LaneInvasionSensor: {
        if (ros_name == "lane_invasion__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaLineInvasionPublisher> new_publisher = std::make_shared<CarlaLineInvasionPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::ObstacleDetectionSensor: {
        std::cerr << "Obstacle detection sensor does not have an available publisher" << std::endl;
      } break;
      case ESensors::OpticalFlowCamera: {
        if (ros_name == "optical_flow__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaOpticalFlowCameraPublisher> new_publisher = std::make_shared<CarlaOpticalFlowCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::Radar: {
        if (ros_name == "radar__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaRadarPublisher> new_publisher = std::make_shared<CarlaRadarPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::RayCastSemanticLidar: {
        if (ros_name == "ray_cast_semantic__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaSemanticLidarPublisher> new_publisher = std::make_shared<CarlaSemanticLidarPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::RayCastLidar: {
        if (ros_name == "ray_cast__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaLidarPublisher> new_publisher = std::make_shared<CarlaLidarPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::RssSensor: {
        std::cerr << "RSS sensor does not have an available publisher" << std::endl;
      } break;
      case ESensors::SceneCaptureCamera: {
        if (ros_name == "rgb__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaRGBCameraPublisher> new_publisher = std::make_shared<CarlaRGBCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::SemanticSegmentationCamera: {
        if (ros_name == "semantic_segmentation__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaSSCameraPublisher> new_publisher = std::make_shared<CarlaSSCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::InstanceSegmentationCamera: {
        if (ros_name == "instance_segmentation__") {
          ros_name.pop_back();
          ros_name.pop_back();
          ros_name += string_id;
          UpdateActorRosName(actor, ros_name);
        }
        std::shared_ptr<CarlaISCameraPublisher> new_publisher = std::make_shared<CarlaISCameraPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({actor, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(ros_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({actor, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::WorldObserver: {
        std::cerr << "World obserser does not have an available publisher" << std::endl;
      } break;
      case ESensors::CameraGBufferUint8: {
        std::cerr << "Camera GBuffer uint8 does not have an available publisher" << std::endl;
      } break;
      case ESensors::CameraGBufferFloat: {
        std::cerr << "Camera GBuffer float does not have an available publisher" << std::endl;
      } break;
      default: {
        std::cerr << "Unknown sensor type" << std::endl;
      }
    }
  }
  return { publisher, transform };
}

void ROS2::ProcessDataFromCamera(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    int W, int H, float Fov,
    const carla::SharedBufferView buffer,
    void *actor) {

  switch (sensor_type) {
    case ESensors::CollisionSensor:
      log_info("Sensor Collision to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      break;
    case ESensors::DepthCamera:
      {
        log_info("Sensor DepthCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
        auto sensors = GetOrCreateSensor(ESensors::DepthCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaDepthCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaDepthCameraPublisher>(sensors.first);
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::NormalsCamera:
      log_info("Sensor NormalsCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::NormalsCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaNormalsCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaNormalsCameraPublisher>(sensors.first);
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::LaneInvasionSensor:
      log_info("Sensor LaneInvasionSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::LaneInvasionSensor, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaLineInvasionPublisher> publisher = std::dynamic_pointer_cast<CarlaLineInvasionPublisher>(sensors.first);
          publisher->SetData(_seconds, _nanoseconds, (const int32_t*) buffer->data());
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::OpticalFlowCamera:
      log_info("Sensor OpticalFlowCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::OpticalFlowCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaOpticalFlowCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaOpticalFlowCameraPublisher>(sensors.first);
          const carla::sensor::s11n::OpticalFlowImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::OpticalFlowImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const float*) (buffer->data() + carla::sensor::s11n::OpticalFlowImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::RssSensor:
      log_info("Sensor RssSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      break;
    case ESensors::SceneCaptureCamera:
    {
      log_info("Sensor SceneCaptureCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::SceneCaptureCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaRGBCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaRGBCameraPublisher>(sensors.first);
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    }
    case ESensors::SemanticSegmentationCamera:
      log_info("Sensor SemanticSegmentationCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::SemanticSegmentationCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaSSCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaSSCameraPublisher>(sensors.first);
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::InstanceSegmentationCamera:
      log_info("Sensor InstanceSegmentationCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      {
        auto sensors = GetOrCreateSensor(ESensors::InstanceSegmentationCamera, stream_id, actor);
        if (sensors.first) {
          std::shared_ptr<CarlaISCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaISCameraPublisher>(sensors.first);
          const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
            reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
          if (!header)
            return;
          if (!publisher->HasBeenInitialized())
            publisher->InitInfoData(0, 0, H, W, Fov, true);
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
          publisher->SetCameraInfoData(_seconds, _nanoseconds);
          publisher->Publish();
        }
        if (sensors.second) {
          std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
          publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
          publisher->Publish();
        }
      }
      break;
    case ESensors::WorldObserver:
      log_info("Sensor WorldObserver to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      break;
    case ESensors::CameraGBufferUint8:
      log_info("Sensor CameraGBufferUint8 to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      break;
    case ESensors::CameraGBufferFloat:
      log_info("Sensor CameraGBufferFloat to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      break;
    default:
      log_info("Sensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
  }
}

void ROS2::ProcessDataFromGNSS(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::geom::GeoLocation &data,
    void *actor) {
  log_info("Sensor GnssSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "geo.", data.latitude, data.longitude, data.altitude);
  auto sensors = GetOrCreateSensor(ESensors::GnssSensor, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaGNSSPublisher> publisher = std::dynamic_pointer_cast<CarlaGNSSPublisher>(sensors.first);
    publisher->SetData(_seconds, _nanoseconds, reinterpret_cast<const double*>(&data));
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromIMU(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    carla::geom::Vector3D accelerometer,
    carla::geom::Vector3D gyroscope,
    float compass,
    void *actor) {
  log_info("Sensor InertialMeasurementUnit to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "imu.", accelerometer.x, gyroscope.x, compass);
  auto sensors = GetOrCreateSensor(ESensors::InertialMeasurementUnit, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaIMUPublisher> publisher = std::dynamic_pointer_cast<CarlaIMUPublisher>(sensors.first);
    publisher->SetData(_seconds, _nanoseconds, reinterpret_cast<float*>(&accelerometer), reinterpret_cast<float*>(&gyroscope), compass);
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromDVS(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::SharedBufferView buffer,
    int W, int H, float Fov,
    void *actor) {
  log_info("Sensor DVS to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id);
  auto sensors = GetOrCreateSensor(ESensors::DVSCamera, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaDVSCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaDVSCameraPublisher>(sensors.first);
    const carla::sensor::s11n::ImageSerializer::ImageHeader *header =
      reinterpret_cast<const carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer->data());
    if (!header)
      return;
    if (!publisher->HasBeenInitialized())
      publisher->InitInfoData(0, 0, H, W, Fov, true);
    size_t elements = (buffer->size() - carla::sensor::s11n::ImageSerializer::header_offset) / sizeof(carla::sensor::data::DVSEvent);
    publisher->SetImageData(_seconds, _nanoseconds, elements, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
    publisher->SetCameraInfoData(_seconds, _nanoseconds);
    publisher->SetPointCloudData(1, elements * sizeof(carla::sensor::data::DVSEvent), elements, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromLidar(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    carla::sensor::data::LidarData &data,
    void *actor) {
  log_info("Sensor Lidar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._points.size());
  auto sensors = GetOrCreateSensor(ESensors::RayCastLidar, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaLidarPublisher> publisher = std::dynamic_pointer_cast<CarlaLidarPublisher>(sensors.first);
    size_t width = data._points.size();
    size_t height = 1;
    publisher->SetData(_seconds, _nanoseconds, height, width, (float*)data._points.data());
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromSemanticLidar(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    carla::sensor::data::SemanticLidarData &data,
    void *actor) {
  static_assert(sizeof(float) == sizeof(uint32_t), "Invalid float size");
  log_info("Sensor SemanticLidar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._ser_points.size());
  auto sensors = GetOrCreateSensor(ESensors::RayCastSemanticLidar, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaSemanticLidarPublisher> publisher = std::dynamic_pointer_cast<CarlaSemanticLidarPublisher>(sensors.first);
    size_t width = data._ser_points.size();
    size_t height = 1;
    publisher->SetData(_seconds, _nanoseconds, 6, height, width, (float*)data._ser_points.data());
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromRadar(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::sensor::data::RadarData &data,
    void *actor) {
  log_info("Sensor Radar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._detections.size());
  auto sensors = GetOrCreateSensor(ESensors::Radar, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaRadarPublisher> publisher = std::dynamic_pointer_cast<CarlaRadarPublisher>(sensors.first);
    size_t elements = data.GetDetectionCount();
    size_t width = elements * sizeof(carla::sensor::data::RadarDetection);
    size_t height = 1;
    publisher->SetData(_seconds, _nanoseconds, height, width, elements, (const uint8_t*)data._detections.data());
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::ProcessDataFromObstacleDetection(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    AActor *first_ctor,
    AActor *second_actor,
    float distance,
    void *actor) {
  log_info("Sensor ObstacleDetector to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "distance.", distance);
}

void ROS2::ProcessDataFromCollisionSensor(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    uint32_t other_actor,
    carla::geom::Vector3D impulse,
    void* actor) {
  auto sensors = GetOrCreateSensor(ESensors::CollisionSensor, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaCollisionPublisher> publisher = std::dynamic_pointer_cast<CarlaCollisionPublisher>(sensors.first);
    publisher->SetData(_seconds, _nanoseconds, other_actor, impulse.x, impulse.y, impulse.z);
    publisher->Publish();
  }
  if (sensors.second) {
    std::shared_ptr<CarlaTransformPublisher> publisher = std::dynamic_pointer_cast<CarlaTransformPublisher>(sensors.second);
    publisher->SetData(_seconds, _nanoseconds, (const float*)&sensor_transform.location, (const float*)&sensor_transform.rotation);
    publisher->Publish();
  }
}

void ROS2::Shutdown() {
  for (auto& element : _publishers) {
    element.second.reset();
  }
  for (auto& element : _transforms) {
    element.second.reset();
  }
  _clock_publisher.reset();
  _controller.reset();
  _enabled = false;
}

} // namespace ros2
} // namespace carla
