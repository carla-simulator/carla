// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>

namespace carla {
namespace ros2 {

/// Azimuth of a point in the extended (Autoware PointXYZIRCAEDT) lidar layout,
/// from its coordinates in the ROS frame (x forward, y left).
///
/// Autoware reads `azimuth` as the sensor's rotation angle: in [0, 2*pi) and
/// increasing along a ring in scan order, as its lidar drivers (nebula) report
/// it. Its preprocessing relies on that; the ring outlier filter, for one,
/// groups neighbouring points of a ring by the increase in azimuth from one to
/// the next. CARLA's ray-cast lidar sweeps clockwise seen from above (Unreal's
/// yaw grows that way), so the angle is measured clockwise from +x -- the
/// Velodyne convention -- which makes it the ray's yaw in the sensor frame.
inline float LidarScanAzimuth(float x, float y) {
  constexpr float kTwoPi = 6.28318530717958647692f;
  float azimuth = std::atan2(-y, x);
  if (azimuth < 0.0f) {
    azimuth += kTwoPi;
  }
  // atan2 of a tiny negative y can round up to exactly 2*pi.
  return azimuth >= kTwoPi ? 0.0f : azimuth;
}

}  // namespace ros2
}  // namespace carla
