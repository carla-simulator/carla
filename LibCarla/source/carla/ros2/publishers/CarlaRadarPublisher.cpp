// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRadarPublisher.h"

#include "carla/sensor/data/RadarData.h"

namespace carla {
namespace ros2 {

struct RadarDetectionWithPosition {
  float x;
  float y;
  float z;
  carla::sensor::data::RadarDetection detection;
};

const size_t CarlaRadarPublisher::GetPointSize() {
  return sizeof(RadarDetectionWithPosition);
}

std::vector<sensor_msgs::msg::PointField> CarlaRadarPublisher::GetFields() {

    sensor_msgs::msg::PointField descriptor1;
    descriptor1.name("x");
    descriptor1.offset(0);
    descriptor1.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor1.count(1);

    sensor_msgs::msg::PointField descriptor2;
    descriptor2.name("y");
    descriptor2.offset(4);
    descriptor2.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor2.count(1);

    sensor_msgs::msg::PointField descriptor3;
    descriptor3.name("z");
    descriptor3.offset(8);
    descriptor3.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor3.count(1);

    sensor_msgs::msg::PointField descriptor4;
    descriptor4.name("velocity");
    descriptor4.offset(12);
    descriptor4.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor4.count(1);

    sensor_msgs::msg::PointField descriptor5;
    descriptor5.name("azimuth");
    descriptor5.offset(16);
    descriptor5.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor5.count(1);

    sensor_msgs::msg::PointField descriptor6;
    descriptor6.name("altitude");
    descriptor6.offset(20);
    descriptor6.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor6.count(1);

    sensor_msgs::msg::PointField descriptor7;
    descriptor7.name("depth");
    descriptor7.offset(24);
    descriptor7.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor7.count(1);

  return {descriptor1, descriptor2, descriptor3, descriptor4, descriptor5, descriptor6, descriptor7};
}

std::vector<uint8_t> CarlaRadarPublisher::ComputePointCloud(uint32_t height, uint32_t width, uint8_t *data) {

  carla::sensor::data::RadarDetection* detections = reinterpret_cast<sensor::data::RadarDetection*>(data);
  const size_t total_bytes = height * width * sizeof(sensor::data::RadarDetection);
  const size_t total_points = total_bytes / sizeof(sensor::data::RadarDetection);

  std::vector<RadarDetectionWithPosition> radar_points(total_points);
  for (size_t i = 0; i < total_points; ++i) {
    const auto& det = detections[i];
    auto& point = radar_points[i];

    point.x = det.depth * std::cos(det.azimuth) * std::cos(-det.altitude);
    point.y = det.depth * std::sin(-det.azimuth) * std::cos(det.altitude);
    point.z = det.depth * std::sin(det.altitude);
    point.detection = det;
  }

  const uint8_t* byte_ptr = reinterpret_cast<const uint8_t*>(radar_points.data());
  return std::vector<uint8_t>(byte_ptr, byte_ptr + radar_points.size() * sizeof(RadarDetectionWithPosition));
}

}  // namespace ros2
}  // namespace carla
