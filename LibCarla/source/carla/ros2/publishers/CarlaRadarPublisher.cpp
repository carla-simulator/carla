// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaRadarPublisher.h"

#include "carla/ros2/publishers/RadarPolarToCartesian.h"
#include "carla/sensor/data/RadarData.h"

#include <cstring>

namespace carla {
namespace ros2 {

namespace {

// Cartesian projection prefixed to each RadarDetection so subscribers see
// (x, y, z, velocity, azimuth, altitude, depth) per point. Memory layout
// must match kRadarFields: 3 leading floats + the raw RadarDetection
// payload (velocity, azimuth, altitude, depth).
struct RadarDetectionWithPosition {
  float x;
  float y;
  float z;
  sensor::data::RadarDetection detection;
};

static_assert(
    sizeof(RadarDetectionWithPosition)
        == 3u * sizeof(float) + sizeof(sensor::data::RadarDetection),
    "RadarDetectionWithPosition must be tightly packed for the wire layout");

}  // namespace

std::size_t CarlaRadarPublisher::GetPointSize() const {
  return sizeof(RadarDetectionWithPosition);
}

const PointFieldDescriptor *CarlaRadarPublisher::GetFieldDescriptors() const {
  return kRadarFields.data();
}

std::size_t CarlaRadarPublisher::GetFieldDescriptorCount() const {
  return kRadarFields.size();
}

std::vector<std::uint8_t> CarlaRadarPublisher::ComputePointCloud(
    std::uint32_t height, std::uint32_t width, const std::uint8_t *data) const {
  const std::size_t total_points =
      static_cast<std::size_t>(height) * static_cast<std::size_t>(width);
  const auto *detections =
      reinterpret_cast<const sensor::data::RadarDetection *>(data);

  std::vector<RadarDetectionWithPosition> radar_points(total_points);
  for (std::size_t i = 0; i < total_points; ++i) {
    const auto &detection = detections[i];
    const auto cartesian = RadarPolarToCartesian(
        detection.depth, detection.azimuth, detection.altitude);
    radar_points[i].x = cartesian[0];
    radar_points[i].y = cartesian[1];
    radar_points[i].z = cartesian[2];
    radar_points[i].detection = detection;
  }

  const auto *byte_ptr = reinterpret_cast<const std::uint8_t *>(radar_points.data());
  return std::vector<std::uint8_t>(
      byte_ptr, byte_ptr + radar_points.size() * sizeof(RadarDetectionWithPosition));
}

}  // namespace ros2
}  // namespace carla
