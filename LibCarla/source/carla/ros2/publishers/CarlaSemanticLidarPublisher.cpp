// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaSemanticLidarPublisher.h"

#include <cstring>

namespace carla {
namespace ros2 {

namespace {

// Wire-compatible POD mirror of sensor::data::SemanticLidarDetection:
// {x, y, z, cos_inc_angle, object_idx, object_tag}. The real struct lives
// behind carla/sensor/data/SemanticLidarData.h, which transitively pulls
// Boost; that header chain is unavailable in the carla-ros2-native
// ExternalProject include path. Layout pinned by kSemanticLidarFields in
// PointCloudFieldsLayout.h and by test_point_cloud_fields.cpp.
struct SemanticLidarPoint {
  float x;
  float y;
  float z;
  float cos_inc_angle;
  std::uint32_t object_idx;
  std::uint32_t object_tag;
};

static_assert(
    sizeof(SemanticLidarPoint) == 24u,
    "SemanticLidarPoint must be tightly packed (4 float32s + 2 uint32s) for the wire layout");

}  // namespace

std::size_t CarlaSemanticLidarPublisher::GetPointSize() const {
  return sizeof(SemanticLidarPoint);
}

const PointFieldDescriptor *CarlaSemanticLidarPublisher::GetFieldDescriptors() const {
  return kSemanticLidarFields.data();
}

std::size_t CarlaSemanticLidarPublisher::GetFieldDescriptorCount() const {
  return kSemanticLidarFields.size();
}

std::vector<std::uint8_t> CarlaSemanticLidarPublisher::ComputePointCloud(
    std::uint32_t height, std::uint32_t width, const std::uint8_t *data) const {
  const std::size_t total_points =
      static_cast<std::size_t>(height) * static_cast<std::size_t>(width);
  const std::size_t total_bytes = total_points * sizeof(SemanticLidarPoint);

  std::vector<std::uint8_t> bytes(total_bytes);
  std::memcpy(bytes.data(), data, total_bytes);

  // Mirror the Y axis for the ROS right-handed frame. The buffer is a
  // contiguous array of SemanticLidarPoint PODs by contract.
  auto *points = reinterpret_cast<SemanticLidarPoint *>(bytes.data());
  for (std::size_t i = 0; i < total_points; ++i) {
    points[i].y *= -1.0f;
  }
  return bytes;
}

}  // namespace ros2
}  // namespace carla
