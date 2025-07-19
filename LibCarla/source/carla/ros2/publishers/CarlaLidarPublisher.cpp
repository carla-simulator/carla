// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaLidarPublisher.h"

#include "carla/sensor/data/LidarData.h"

namespace carla {
namespace ros2 {

const size_t CarlaLidarPublisher::GetPointSize() {
  return sizeof(sensor::data::LidarDetection);
  // return 4 * sizeof(float);
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

std::vector<uint8_t> CarlaLidarPublisher::ComputePointCloud(uint32_t height, uint32_t width, float *data) {
  float* it = data;
  float* end = &data[height * width];
  for (++it; it < end; it += 4) {
      *it *= -1.0f;
  }
  std::vector<uint8_t> vector_data;
  const size_t size = height * width * sizeof(float);
  vector_data.resize(size);
  std::memcpy(&vector_data[0], &data[0], size);

  return std::move(vector_data);
}

}  // namespace ros2
}  // namespace carla
