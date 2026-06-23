// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <array>
#include <cmath>

namespace carla {
namespace ros2 {

// Converts radar polar coordinates (depth in metres, azimuth + altitude in
// radians) into a CARLA-handed Cartesian (x, y, z) triple. The convention
// matches sensor::data::RadarDetection's polar layout and the per-axis sign
// flips applied when packing the wire bytes for ROS 2 subscribers:
//   x = depth * cos(azimuth)  * cos(-altitude)
//   y = depth * sin(-azimuth) * cos( altitude)
//   z = depth * sin(altitude)
// The y axis carries the same left-handed-to-right-handed mirror used by the
// lidar / semantic-lidar publishers; this keeps every CARLA point-cloud
// publisher on the same convention.
[[nodiscard]] inline std::array<float, 3> RadarPolarToCartesian(
    float depth, float azimuth, float altitude) noexcept {
  const float cos_az  = std::cos(azimuth);
  const float sin_az  = std::sin(-azimuth);
  const float cos_alt = std::cos(altitude);
  const float sin_alt = std::sin(altitude);
  const float cos_neg_alt = std::cos(-altitude);
  return {
      depth * cos_az * cos_neg_alt,
      depth * sin_az * cos_alt,
      depth * sin_alt,
  };
}

}  // namespace ros2
}  // namespace carla
