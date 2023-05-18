// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/geom/Transform.h"
#include "carla/streaming/detail/Types.h"

#include <unordered_set>

// forward declarations
class AActor;
namespace carla {
  namespace geom {
    class GeoLocation;
    class Vector3D;
  }
  namespace sensor {
    namespace data {
      struct DVSEvent;
      class LidarData;
      class SemanticLidarData;
      class RadarData;
    }
  }
}


namespace carla {
namespace ros2 {

class CarlaRGBCameraPublisher;
class CarlaDepthCameraPublisher;
class CarlaSSCameraPublisher;
class CarlaDVSCameraPublisher;
class CarlaLidarPublisher;
class CarlaSemanticLidarPublisher;
class CarlaRadarPublisher;
class CarlaIMUPublisher;
class CarlaGNSSPublisher;
class CarlaMapSensorPublisher;
class CarlaSpeedometerSensor;

class ROS2
{
  public:

  // deleting copy constructor for singleton
  ROS2(const ROS2& obj) = delete;
  static std::shared_ptr<ROS2> GetInstance() {
    if (!_instance)
      _instance = std::shared_ptr<ROS2>(new ROS2);
    return _instance;
  }

  // general
  void Enable(bool enable);
  bool IsEnabled() { return _enabled; }
  void SetFrame(uint64_t frame);

  void InitPublishers();

  // enabling streams to publish
  void EnableStream(carla::streaming::detail::stream_id_type id) { _publish_stream.insert(id); }
  bool IsStreamEnabled(carla::streaming::detail::stream_id_type id) { return _publish_stream.count(id) > 0; }
  void ResetStreams() { _publish_stream.clear(); }

  // receiving data to publish
  void ProcessDataFromSensor(uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const carla::Buffer &buffer);
  void ProcessDataFromGNSS(uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const carla::geom::GeoLocation &data);
  void ProcessDataFromIMU(uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      carla::geom::Vector3D accelerometer,
      carla::geom::Vector3D gyroscope,
      float compass);
  void ProcessDataFromDVS(uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const std::vector<carla::sensor::data::DVSEvent> &events,
      const carla::Buffer &buffer);
  void ProcessDataFromLidar(uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const carla::sensor::data::LidarData &data);
  void ProcessDataFromSemanticLidar(uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const carla::sensor::data::SemanticLidarData &data);
  void ProcessDataFromRadar(uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      const carla::sensor::data::RadarData &data);
  void ProcessDataFromObstacleDetection(uint64_t sensor_type,
      carla::streaming::detail::stream_id_type stream_id,
      const carla::geom::Transform sensor_transform,
      AActor *Actor,
      AActor *OtherActor,
      float Distance);

  private:
  void UpdateRGBCamera(const carla::Buffer& buffer, const char* frame_id);
  void UpdateDepthCamera(const carla::Buffer& buffer, const char* frame_id);
  void UpdateSSCamera(const carla::Buffer& buffer, const char* frame_id);
  void UpdateDVSCamera(const carla::Buffer& buffer, const char* frame_id);
  void UpdateLidar(const carla::sensor::data::LidarData &data, const char* frame_id);
  void UpdateSemanticLidar(const carla::sensor::data::SemanticLidarData &data, const char* frame_id);
  void UpdateRadar(const carla::sensor::data::RadarData &data, const char* frame_id);
  void UpdateIMU(carla::geom::Vector3D accelerometer, carla::geom::Vector3D gyroscope, float compass, const char* frame_id);
  void UpdateGNSS(const carla::geom::GeoLocation &data, const char* frame_id);
  void UpdateMapSensor(const char* data);
  void UpdateSpeedometerSensor(float data);

  // sigleton
  ROS2() {};
  static std::shared_ptr<ROS2> _instance;

  bool _enabled { false };
  uint64_t _frame { 0 };
  std::unordered_set<carla::streaming::detail::stream_id_type> _publish_stream;
  CarlaRGBCameraPublisher* _rgb_camera_publisher { nullptr };
  CarlaDepthCameraPublisher* _depth_camera_publisher { nullptr };
  CarlaSSCameraPublisher* _ss_camera_publisher { nullptr };
  CarlaDVSCameraPublisher* _dvs_camera_publisher { nullptr };
  CarlaLidarPublisher* _lidar_publisher { nullptr };
  CarlaSemanticLidarPublisher* _semantic_lidar_publisher { nullptr };
  CarlaRadarPublisher* _radar_publisher { nullptr };
  CarlaIMUPublisher* _imu_publisher { nullptr };
  CarlaGNSSPublisher* _gnss_publisher { nullptr };
  CarlaMapSensorPublisher* _map_sensor_publisher { nullptr };
  CarlaSpeedometerSensor* _speedometer_sensor { nullptr };
};

} // namespace ros2
} // namespace carla
