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
}

void ROS2::SetFrame(uint64_t frame, double timestamp) {
  _frame = frame;
   //log_info("ROS2 new frame: ", _frame);
}

void ROS2::SetTimestamp(double timestamp) {
  _timestamp = timestamp;
   //log_info("ROS2 new timestamp: ", _timestamp);
}

void ROS2::InitPublishers() {
  if (_rgb_camera_publisher)
    return;

  _rgb_camera_publisher = new CarlaRGBCameraPublisher();
  _rgb_camera_publisher->Init();
  _depth_camera_publisher = new CarlaDepthCameraPublisher();
  _depth_camera_publisher->Init();
  _ss_camera_publisher = new CarlaSSCameraPublisher();
  _ss_camera_publisher->Init();
  _dvs_camera_publisher = new CarlaDVSCameraPublisher();
  _dvs_camera_publisher->Init();
  _lidar_publisher = new CarlaLidarPublisher();
  _lidar_publisher->Init();
  _semantic_lidar_publisher = new CarlaSemanticLidarPublisher();
  _semantic_lidar_publisher->Init();
  _radar_publisher = new CarlaRadarPublisher();
  _radar_publisher->Init();
  _imu_publisher = new CarlaIMUPublisher();
  _imu_publisher->Init();
  _gnss_publisher = new CarlaGNSSPublisher();
  _gnss_publisher->Init();
  _map_sensor_publisher = new CarlaMapSensorPublisher();
  _map_sensor_publisher->Init();
  _speedometer_sensor = new CarlaSpeedometerSensor();
  _speedometer_sensor->Init();
}

void ROS2::ProcessDataFromSensor(uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::Buffer &buffer) {

  switch (sensor_type) {
    case ESensors::CollisionSensor:
      log_info("Sensor Collision to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      break;
    case ESensors::DepthCamera:
      log_info("Sensor DepthCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      UpdateDepthCamera(buffer, std::to_string(_frame).c_str());
      _depth_camera_publisher->Publish();
      break;
    case ESensors::NormalsCamera:
      log_info("Sensor NormalsCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      break;
    case ESensors::LaneInvasionSensor:
      log_info("Sensor LaneInvasionSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      break;
    case ESensors::OpticalFlowCamera:
      log_info("Sensor OpticalFlowCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      break;
    case ESensors::RssSensor:
      log_info("Sensor RssSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      break;
    case ESensors::SceneCaptureCamera:
    {
      log_info("Sensor SceneCaptureCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      UpdateRGBCamera(buffer, std::to_string(_frame).c_str());
      _rgb_camera_publisher->Publish();
      break;
    }
    case ESensors::SemanticSegmentationCamera:
      log_info("Sensor SemanticSegmentationCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      UpdateSSCamera(buffer, std::to_string(_frame).c_str());
      _ss_camera_publisher->Publish();
      break;
    case ESensors::InstanceSegmentationCamera:
      log_info("Sensor InstanceSegmentationCamera to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      break;
    case ESensors::WorldObserver:
      log_info("Sensor WorldObserver to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      break;
    case ESensors::CameraGBufferUint8:
      log_info("Sensor CameraGBufferUint8 to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      break;
    case ESensors::CameraGBufferFloat:
      log_info("Sensor CameraGBufferFloat to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
      break;
    default:
      log_info("Sensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "buffer.", buffer.size());
  }
}

void ROS2::ProcessDataFromGNSS(uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::geom::GeoLocation &data) {
  log_info("Sensor GnssSensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "geo.", data.latitude, data.longitude, data.altitude);
  UpdateGNSS(data, std::to_string(_frame).c_str());
  _gnss_publisher->Publish();
}

void ROS2::ProcessDataFromIMU(uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    carla::geom::Vector3D accelerometer,
    carla::geom::Vector3D gyroscope,
    float compass) {
  log_info("Sensor InertialMeasurementUnit to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "imu.", accelerometer.x, gyroscope.x, compass);
  UpdateIMU(accelerometer, gyroscope, compass, std::to_string(_frame).c_str());
  _imu_publisher->Publish();
}

void ROS2::ProcessDataFromDVS(uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const std::vector<carla::sensor::data::DVSEvent> &events) {
  log_info("Sensor DVS to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "events.", events.size());
  UpdateDVSCamera(events, std::to_string(_frame).c_str());
  _dvs_camera_publisher->Publish();
}

void ROS2::ProcessDataFromLidar(uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::sensor::data::LidarData &data) {
  log_info("Sensor Lidar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._points.size());
  UpdateLidar(data, std::to_string(_frame).c_str());
  _lidar_publisher->Publish();
}

void ROS2::ProcessDataFromSemanticLidar(uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::sensor::data::SemanticLidarData &data) {
  log_info("Sensor SemanticLidar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._ser_points.size());
  UpdateSemanticLidar(data, std::to_string(_frame).c_str());
  _semantic_lidar_publisher->Publish();
}

void ROS2::ProcessDataFromRadar(uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    const carla::sensor::data::RadarData &data) {
  log_info("Sensor Radar to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "points.", data._detections.size());
  UpdateRadar(data, std::to_string(_frame).c_str());
  _radar_publisher->Publish();
}

void ROS2::ProcessDataFromObstacleDetection(uint64_t sensor_type,
    carla::streaming::detail::stream_id_type stream_id,
    const carla::geom::Transform sensor_transform,
    AActor *Actor,
    AActor *OtherActor,
    float Distance) {
  log_info("Sensor ObstacleDetector to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id, "distance.", Distance);
}


void ROS2::UpdateRGBCamera(const carla::Buffer& buffer, const char* frame_id) {
  if (!_rgb_camera_publisher)
    return;

  // header
  carla::sensor::s11n::ImageSerializer::ImageHeader *header =
    reinterpret_cast<carla::sensor::s11n::ImageSerializer::ImageHeader *>(buffer.data());
  if (!header)
    return;

  _rgb_camera_publisher->SetData(
      header->height,
      header->width,
      (const uint8_t*) (buffer.data() + carla::sensor::s11n::ImageSerializer::header_offset),
      frame_id);
}

void ROS2::UpdateDepthCamera(const carla::Buffer& buffer, const char* frame_id) {
  if (_depth_camera_publisher) {
    carla::Buffer tmp;
    tmp.copy_from(carla::sensor::s11n::SensorHeaderSerializer::header_offset, buffer);
    carla::sensor::data::Image img { std::move(tmp) };
    size_t width = img.GetWidth();
    size_t height = img.GetHeight();
    _depth_camera_publisher->SetData(height, width, (const uint8_t*)buffer.data(), frame_id);
  }
}

void ROS2::UpdateSSCamera(const carla::Buffer& buffer, const char* frame_id) {
  if (_ss_camera_publisher) {
    carla::Buffer tmp;
    tmp.copy_from(carla::sensor::s11n::SensorHeaderSerializer::header_offset, buffer);
    carla::sensor::data::Image img { std::move(tmp) };
    size_t width = img.GetWidth();
    size_t height = img.GetHeight();
    _ss_camera_publisher->SetData(height, width, (const uint8_t*)buffer.data(), frame_id);
  }
}

void ROS2::UpdateDVSCamera(const std::vector<carla::sensor::data::DVSEvent> &events, const char* frame_id) {
  if (_dvs_camera_publisher) {
    // carla::Buffer tmp;
    // tmp.copy_from(carla::sensor::s11n::SensorHeaderSerializer::header_offset, buffer);
    // carla::sensor::data::Image img { std::move(tmp) };
    // size_t width = img.GetWidth();
    // size_t height = img.GetHeight();
    // _dvs_camera_publisher->SetData(height, width, (const uint8_t*)buffer.data(), frame_id);
  }
}

void ROS2::UpdateLidar(const carla::sensor::data::LidarData &data, const char* frame_id) {
  if (_lidar_publisher) {
    size_t width = data._points.size();
    size_t height = 1;
    _lidar_publisher->SetData(height, width, (const uint8_t*)data._points.data(), frame_id);
  }
}

void ROS2::UpdateSemanticLidar(const carla::sensor::data::SemanticLidarData &data, const char* frame_id) {
  if (_semantic_lidar_publisher) {
    size_t width = data._ser_points.size();
    size_t height = 1;
    _semantic_lidar_publisher->SetData(height, width, (const uint8_t*)&data._ser_points[0], frame_id);
  }
}

void ROS2::UpdateRadar(const carla::sensor::data::RadarData &data, const char* frame_id) {
if (_radar_publisher) {
    size_t width = data.GetDetectionCount();
    size_t height = 1;
    _radar_publisher->SetData(height, width, (const uint8_t*)data._detections.data(), frame_id);
  }
}

void ROS2::UpdateIMU(carla::geom::Vector3D accelerometer, carla::geom::Vector3D gyroscope, float compass, const char* frame_id) {
  if (_imu_publisher) {
    _imu_publisher->SetData(reinterpret_cast<float*>(&accelerometer), reinterpret_cast<float*>(&gyroscope), compass, frame_id);
  }
}

void ROS2::UpdateGNSS(const carla::geom::GeoLocation &data, const char* frame_id) {
  if (_gnss_publisher) {
    _gnss_publisher->SetData(reinterpret_cast<const double*>(&data), frame_id);
  }
}

void ROS2::UpdateMapSensor(const char* data) {
  if (_map_sensor_publisher) {
    _map_sensor_publisher->SetData(data);
  }
}

void ROS2::UpdateSpeedometerSensor(float data) {
  if (_speedometer_sensor) {
    _speedometer_sensor->SetData(data);
  }
}

} // namespace ros2
} // namespace carla
