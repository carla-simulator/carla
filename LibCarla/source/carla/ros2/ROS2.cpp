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
#include "publishers/CarlaRGBCameraPublisher.h"
#include "publishers/CarlaDepthCameraPublisher.h"
#include "publishers/CarlaSSCameraPublisher.h"
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
  _controller = std::make_shared<CarlaEgoVehicleControlSubscriber>("ego", "map");
  _controller->Init();
}

void ROS2::SetFrame(uint64_t frame) {
  _frame = frame;
   //log_info("ROS2 new frame: ", _frame);
}

void ROS2::SetTimestamp(double timestamp) {
  double integral;
  const double fractional = modf(timestamp, &integral);
  const double multiplier = 1000000000.0;
  _seconds = static_cast<int32_t>(integral);
  _nanoseconds = static_cast<uint32_t>(fractional * multiplier);
   //log_info("ROS2 new timestamp: ", _timestamp);
}

void ROS2::AddActorRosName(void *actor, std::string ros_name) {
  _actor_ros_name.insert({actor, ros_name});
}

void ROS2::AddActorParentRosName(void *actor, std::string ros_name) {
  _actor_parent_ros_name.insert({actor, ros_name});
}

void ROS2::RemoveActorRosName(void *actor) {
  _actor_ros_name.erase(actor);
  _actor_parent_ros_name.erase(actor);
}

std::string ROS2::GetActorRosName(void *actor) {
  auto it = _actor_ros_name.find(actor);
  if (it != _actor_ros_name.end())
    return it->second;
  else
    return std::string("");
}

std::string ROS2::GetActorParentRosName(void *actor) {
  auto it = _actor_parent_ros_name.find(actor);
  if (it != _actor_parent_ros_name.end())
    return it->second;
  else
    return std::string("");
}

std::pair<std::shared_ptr<CarlaPublisher>, std::shared_ptr<CarlaTransformPublisher>> ROS2::GetOrCreateSensor(int type, carla::streaming::detail::stream_id_type id, void* actor) {
  auto it_publishers = _publishers.find(id);
  auto it_transforms = _transforms.find(id);
  std::shared_ptr<CarlaPublisher> publisher {};
  std::shared_ptr<CarlaTransformPublisher> transform {};
  std::string string_id = std::to_string(id);
  std::string ros_name = GetActorRosName(actor);
  std::string parent_ros_name = GetActorParentRosName(actor);
  if (it_publishers != _publishers.end()) {
    publisher = it_publishers->second;
    if (it_transforms != _transforms.end()) {
      transform = it_transforms->second;
    }
  } else {
    //Sensor not found, creating one of the given type
    switch(type) {
      case ESensors::CollisionSensor: {
        std::string pub_name = ros_name;
        if (pub_name.empty()) {
          pub_name = "collision_";
          pub_name += string_id;
        }
        std::shared_ptr<CarlaCollisionPublisher> new_publisher = std::make_shared<CarlaCollisionPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({id, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({id, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::DepthCamera: {
        std::string pub_name = ros_name;
        if (pub_name.empty()) {
          pub_name = "depth_camera_";
          pub_name += string_id;
        }
        std::shared_ptr<CarlaDepthCameraPublisher> new_publisher = std::make_shared<CarlaDepthCameraPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({id, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({id, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::NormalsCamera: {
        std::cout << "Normals camera does not have an available publisher" << std::endl;
      } break;
      case ESensors::DVSCamera: {
        std::string pub_name = ros_name;
        if (pub_name.empty()) {
          pub_name = "dvs_camera_";
          pub_name += string_id;
        }
        std::shared_ptr<CarlaDVSCameraPublisher> new_publisher = std::make_shared<CarlaDVSCameraPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({id, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({id, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::GnssSensor: {
        std::string pub_name = ros_name;
        if (pub_name.empty()) {
          pub_name = "gnss_";
          pub_name += string_id;
        }
        std::shared_ptr<CarlaGNSSPublisher> new_publisher = std::make_shared<CarlaGNSSPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({id, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({id, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::InertialMeasurementUnit: {
        std::string pub_name = ros_name;
        if (pub_name.empty()) {
          pub_name = "imu_";
          pub_name += string_id;
        }
        std::shared_ptr<CarlaIMUPublisher> new_publisher = std::make_shared<CarlaIMUPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({id, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({id, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::LaneInvasionSensor: {
        std::string pub_name = ros_name;
        if (pub_name.empty()) {
          pub_name = "lane_invasion_";
          pub_name += string_id;
        }
        std::shared_ptr<CarlaLineInvasionPublisher> new_publisher = std::make_shared<CarlaLineInvasionPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({id, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({id, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::ObstacleDetectionSensor: {
        std::cout << "Obstacle detection sensor does not have an available publisher" << std::endl;
      } break;
      case ESensors::OpticalFlowCamera: {
        std::cout << "Optical flow camera does not have an available publisher" << std::endl;
      } break;
      case ESensors::Radar: {
        std::string pub_name = ros_name;
        if (pub_name.empty()) {
          pub_name = "radar_";
          pub_name += string_id;
        }
        std::shared_ptr<CarlaRadarPublisher> new_publisher = std::make_shared<CarlaRadarPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({id, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({id, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::RayCastSemanticLidar: {
        std::string pub_name = ros_name;
        if (pub_name.empty()) {
          pub_name = "semantic_lidar_";
          pub_name += string_id;
        }
        std::shared_ptr<CarlaSemanticLidarPublisher> new_publisher = std::make_shared<CarlaSemanticLidarPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({id, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({id, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::RayCastLidar: {
        std::string pub_name = ros_name;
        if (pub_name.empty()) {
          pub_name = "lidar_";
          pub_name += string_id;
        }
        std::shared_ptr<CarlaLidarPublisher> new_publisher = std::make_shared<CarlaLidarPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({id, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({id, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::RssSensor: {
        std::cout << "RSS sensor does not have an available publisher" << std::endl;
      } break;
      case ESensors::SceneCaptureCamera: {
        std::string pub_name = ros_name;
        if (pub_name.empty()) {
          pub_name = "rgb_camera_";
          pub_name += string_id;
        }
        std::shared_ptr<CarlaRGBCameraPublisher> new_publisher = std::make_shared<CarlaRGBCameraPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({id, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({id, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::SemanticSegmentationCamera: {
        std::string pub_name = ros_name;
        if (pub_name.empty()) {
          pub_name = "semantic_segmentation_camera_";
          pub_name += string_id;
        }
        std::shared_ptr<CarlaSSCameraPublisher> new_publisher = std::make_shared<CarlaSSCameraPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_publisher->Init()) {
          _publishers.insert({id, new_publisher});
          publisher = new_publisher;
        }
        std::shared_ptr<CarlaTransformPublisher> new_transform = std::make_shared<CarlaTransformPublisher>(pub_name.c_str(), parent_ros_name.c_str());
        if (new_transform->Init()) {
          _transforms.insert({id, new_transform});
          transform = new_transform;
        }
      } break;
      case ESensors::InstanceSegmentationCamera: {
        std::cout << "Instance segmentation camera does not have an available publisher" << std::endl;
      } break;
      case ESensors::WorldObserver: {
        std::cout << "World obserser does not have an available publisher" << std::endl;
      } break;
      case ESensors::CameraGBufferUint8: {
        std::cout << "Camera GBuffer uint8 does not have an available publisher" << std::endl;
      } break;
      case ESensors::CameraGBufferFloat: {
        std::cout << "Camera GBuffer float does not have an available publisher" << std::endl;
      } break;
      default: {
        std::cout << "Unknown sensor type" << std::endl;
      }
    }
  }
  return { publisher, transform };
}

void ROS2::ProcessDataFromSensor(
    uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
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
          publisher->SetData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
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
      break;
    case ESensors::LaneInvasionSensor:
      log_info("Sensor LaneInvasionSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
      break;
    case ESensors::OpticalFlowCamera:
      log_info("Sensor OpticalFlowCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
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
          publisher->SetImageData(_seconds, _nanoseconds, header->height, header->width, (const uint8_t*) (buffer->data() + carla::sensor::s11n::ImageSerializer::header_offset));
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
    case ESensors::InstanceSegmentationCamera:
      log_info("Sensor InstanceSegmentationCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer->size());
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
    const std::vector<carla::sensor::data::DVSEvent> &events,
    void *actor) {
  log_info("Sensor DVS to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "events.", events.size());
  auto sensors = GetOrCreateSensor(ESensors::DVSCamera, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaDVSCameraPublisher> publisher = std::dynamic_pointer_cast<CarlaDVSCameraPublisher>(sensors.first);
    //publisher->SetData(_seconds, _nanoseconds, );
    //set camera data
    //set camera info data
    //set pointcloud data
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
  log_info("Sensor SemanticLidar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._ser_points.size());
  auto sensors = GetOrCreateSensor(ESensors::RayCastSemanticLidar, stream_id, actor);
  if (sensors.first) {
    std::shared_ptr<CarlaSemanticLidarPublisher> publisher = std::dynamic_pointer_cast<CarlaSemanticLidarPublisher>(sensors.first);
    size_t width = data._ser_points.size();
    size_t height = 1;
    publisher->SetData(_seconds, _nanoseconds, height, width, (float*)data._ser_points.data());
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
    size_t width = data.GetDetectionCount();
    size_t height = 1;
    publisher->SetData(_seconds, _nanoseconds, height, width, (const uint8_t*)data._detections.data());
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

void ROS2::Shutdown() {
  for (auto& element : _publishers) {
    element.second.reset();
  }
  for (auto& element : _transforms) {
    element.second.reset();
  }
  _controller.reset();
  _enabled = false;
}

 } // namespace ros2
} // namespace carla
