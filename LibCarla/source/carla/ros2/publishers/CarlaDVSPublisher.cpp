// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaDVSPublisher.h"

#include "carla/sensor/data/DVSEvent.h"

namespace carla {
namespace ros2 {

size_t CarlaDVSPointCloudPublisher::GetPointSize() {
  return sizeof(sensor::data::DVSEvent);
}

std::vector<msg::PointField> CarlaDVSPointCloudPublisher::GetFields() {

    msg::PointField descriptor1;
    descriptor1.name = "x";
    descriptor1.offset = 0;
    descriptor1.datatype = msg::PointField::UINT16;
    descriptor1.count = 1;

    msg::PointField descriptor2;
    descriptor2.name = "y";
    descriptor2.offset = 2;
    descriptor2.datatype = msg::PointField::UINT16;
    descriptor2.count = 1;

    msg::PointField descriptor3;
    descriptor3.name = "t";
    descriptor3.offset = 4;
    descriptor3.datatype = msg::PointField::FLOAT64;
    descriptor3.count = 1;

    msg::PointField descriptor4;
    descriptor4.name = "pol";
    descriptor4.offset = 12;
    descriptor4.datatype = msg::PointField::INT8;
    descriptor4.count = 1;

  return {descriptor1, descriptor2, descriptor3, descriptor4};
}

std::vector<uint8_t> CarlaDVSPointCloudPublisher::ComputePointCloud(uint32_t height, uint32_t width, uint8_t *data) {

  sensor::data::DVSEvent* events = reinterpret_cast<sensor::data::DVSEvent*>(data);
  const size_t total_points = height * width;
  for (size_t i = 0; i < total_points; ++i) {
    events[i].y = static_cast<std::uint16_t>(static_cast<float>(events[i].y) * -1.0f);
  }

  const size_t total_bytes = total_points * sizeof(sensor::data::DVSEvent);
  std::vector<uint8_t> vector_data(reinterpret_cast<uint8_t*>(events),
                                   reinterpret_cast<uint8_t*>(events) + total_bytes);
  return vector_data;
}

}  // namespace ros2
}  // namespace carla
