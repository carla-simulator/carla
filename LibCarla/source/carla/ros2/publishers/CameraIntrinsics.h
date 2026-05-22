// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>
#include <cstdint>

namespace carla {
namespace ros2 {

struct CameraIntrinsics {
  double fx;
  double fy;
  double cx;
  double cy;
};

// Builds a pinhole-camera intrinsic matrix from horizontal FOV (degrees) and
// image dimensions (pixels):
//   fx = width / (2 * tan(fov_deg * pi / 360))
//   fy = fx (square pixels)
//   cx = width  / 2
//   cy = height / 2
//
// This is the corrected form from upstream commit 86ebadcee. The pre-fix
// spelling read `width / (2 * tan(fov) * M_PI / 360)` which applied the
// degree-to-radian conversion to `tan(fov)` instead of to `fov`, producing a
// nonsensical focal length.
//
// M_PI is used here, not std::numbers::pi_v, because this header is consumed
// by the carla-ros2-native ExternalProject in Ros2Native/, which does not
// configure CMAKE_CXX_STANDARD and therefore defaults to C++17 where <numbers>
// is unavailable.
[[nodiscard]] inline CameraIntrinsics ComputeIntrinsics(
    uint32_t width, uint32_t height, float fov_degrees) noexcept {
  const double w = static_cast<double>(width);
  const double h = static_cast<double>(height);
  const double fov_rad = static_cast<double>(fov_degrees) * M_PI / 360.0;
  const double fx = w / (2.0 * std::tan(fov_rad));
  return {fx, fx, w / 2.0, h / 2.0};
}

}  // namespace ros2
}  // namespace carla
