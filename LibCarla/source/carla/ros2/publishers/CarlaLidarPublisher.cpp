// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaLidarPublisher.h"

#include "carla/sensor/data/LidarData.h"

namespace carla {
namespace ros2 {

const size_t CarlaLidarPublisher::GetPointSize() {
  return sizeof(sensor::data::LidarDetection);
}

std::vector<sensor_msgs::msg::PointField> CarlaLidarPublisher::GetFields() {

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
    descriptor4.name("intensity");
    descriptor4.offset(12);
    descriptor4.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor4.count(1);

    return {descriptor1, descriptor2, descriptor3, descriptor4};
}

std::vector<uint8_t> CarlaLidarPublisher::ComputePointCloud(uint32_t height, uint32_t width, uint8_t *data) {

  sensor::data::LidarDetection* detections = reinterpret_cast<sensor::data::LidarDetection*>(data);

  const size_t total_points = height * width;
  for (size_t i = 0; i < total_points; ++i) {
    detections[i].point.y *= -1.0f;
  }

  const size_t total_bytes = total_points * sizeof(sensor::data::LidarDetection);
  std::vector<uint8_t> vector_data(reinterpret_cast<uint8_t*>(detections),
                                   reinterpret_cast<uint8_t*>(detections) + total_bytes);
  return vector_data;
}

}  // namespace ros2
}  // namespace carla
