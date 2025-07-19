// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaDVSCameraPublisher.h"

#include "carla/sensor/data/DVSEvent.h"

namespace carla {
namespace ros2 {

const size_t CarlaDVSPointCloudPublisher::GetPointSize() {
  return sizeof(sensor::data::DVSEvent);
}

std::vector<sensor_msgs::msg::PointField> CarlaDVSPointCloudPublisher::GetFields() {

    sensor_msgs::msg::PointField descriptor1;
    descriptor1.name("x");
    descriptor1.offset(0);
    descriptor1.datatype(sensor_msgs::msg::PointField__UINT16);
    descriptor1.count(1);
    sensor_msgs::msg::PointField descriptor2;
    descriptor2.name("y");
    descriptor2.offset(2);
    descriptor2.datatype(sensor_msgs::msg::PointField__UINT16);
    descriptor2.count(1);
    sensor_msgs::msg::PointField descriptor3;
    descriptor3.name("t");
    descriptor3.offset(4);
    descriptor3.datatype(sensor_msgs::msg::PointField__FLOAT64);
    descriptor3.count(1);
    sensor_msgs::msg::PointField descriptor4;
    descriptor3.name("pol");
    descriptor3.offset(12);
    descriptor3.datatype(sensor_msgs::msg::PointField__INT8);
    descriptor3.count(1);

  return {descriptor1, descriptor2, descriptor3, descriptor4};
}

std::vector<uint8_t> CarlaDVSPointCloudPublisher::ComputePointCloud(uint32_t height, uint32_t width, float *data) {
  std::vector<uint8_t> vector_data;
  const size_t size = height * width;
  vector_data.resize(size);
  std::memcpy(&vector_data[0], &data[0], size);

  return vector_data;
}

}  // namespace ros2
}  // namespace carla
