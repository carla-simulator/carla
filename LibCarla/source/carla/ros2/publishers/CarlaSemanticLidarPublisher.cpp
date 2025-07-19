// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaSemanticLidarPublisher.h"

#include "carla/sensor/data/SemanticLidarData.h"

namespace carla {
namespace ros2 {

const size_t CarlaSemanticLidarPublisher::GetPointSize() {
  return sizeof(sensor::data::SemanticLidarDetection);
}

std::vector<sensor_msgs::msg::PointField> CarlaSemanticLidarPublisher::GetFields() {

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
  descriptor4.name("cos_inc_angle");
  descriptor4.offset(12);
  descriptor4.datatype(sensor_msgs::msg::PointField__FLOAT32);
  descriptor4.count(1);

  sensor_msgs::msg::PointField descriptor5;
  descriptor5.name("object_idx");
  descriptor5.offset(16);
  descriptor5.datatype(sensor_msgs::msg::PointField__UINT32);
  descriptor5.count(1);

  sensor_msgs::msg::PointField descriptor6;
  descriptor6.name("object_tag");
  descriptor6.offset(20);
  descriptor6.datatype(sensor_msgs::msg::PointField__UINT32);
  descriptor6.count(1);

  return {descriptor1, descriptor2, descriptor3, descriptor4, descriptor5, descriptor6};
}

std::vector<uint8_t> CarlaSemanticLidarPublisher::ComputePointCloud(uint32_t height, uint32_t width, float *data) {
  // TODO: Do not harcode number of elements (6)

  float* it = data;
  float* end = &data[height * width * 6];
  for (++it; it < end; it += 6) {
    *it *= -1.0f;
  }

  std::vector<uint8_t> vector_data;
  const size_t size = height * width * sizeof(float) * 6;
  vector_data.resize(size);
  std::memcpy(&vector_data[0], &data[0], size);

  return vector_data;
}

}  // namespace ros2
}  // namespace carla
