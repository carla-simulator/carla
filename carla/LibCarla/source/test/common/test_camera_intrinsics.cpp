// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/CameraIntrinsics.h>

#include <cmath>

namespace {
constexpr double kEps = 1e-6;
}  // namespace

TEST(CameraIntrinsics, hd_90deg_fov_focal_length) {
  // 1920x1080 with 90deg horizontal FOV: fx = 1920 / (2 * tan(45deg)) = 960.
  const auto k = carla::ros2::ComputeIntrinsics(1920u, 1080u, 90.0f);
  EXPECT_NEAR(k.fx, 960.0, kEps);
  EXPECT_NEAR(k.fy, 960.0, kEps);
  EXPECT_NEAR(k.cx, 960.0, kEps);
  EXPECT_NEAR(k.cy, 540.0, kEps);
}

TEST(CameraIntrinsics, square_60deg_fov) {
  // 800x800 with 60deg FOV: fx = 800 / (2 * tan(30deg)) ~ 692.82.
  const auto k = carla::ros2::ComputeIntrinsics(800u, 800u, 60.0f);
  const double expected_fx = 800.0 / (2.0 * std::tan(M_PI / 6.0));
  EXPECT_NEAR(k.fx, expected_fx, kEps);
  EXPECT_NEAR(k.fy, k.fx, kEps);
  EXPECT_NEAR(k.cx, 400.0, kEps);
  EXPECT_NEAR(k.cy, 400.0, kEps);
}

TEST(CameraIntrinsics, regression_guard_against_buggy_precedence) {
  // The pre-86ebadcee spelling computed fx = width / (2 * tan(fov) * M_PI / 360),
  // applying the degree-to-radian conversion to tan(fov) instead of to fov.
  // For fov=90 that returned ~0.06; the corrected form returns 960. Guard against
  // a regression to the buggy spelling.
  const auto k = carla::ros2::ComputeIntrinsics(1920u, 1080u, 90.0f);
  ASSERT_GT(k.fx, 100.0);
}

TEST(CameraIntrinsics, narrow_fov) {
  // 1024x768 with 30deg horizontal FOV.
  const auto k = carla::ros2::ComputeIntrinsics(1024u, 768u, 30.0f);
  const double expected_fx = 1024.0 / (2.0 * std::tan(M_PI / 12.0));
  EXPECT_NEAR(k.fx, expected_fx, kEps);
  EXPECT_NEAR(k.cy, 384.0, kEps);
}

TEST(CameraIntrinsics, principal_point_always_at_image_centre) {
  for (float fov : {30.0f, 60.0f, 90.0f, 120.0f, 150.0f}) {
    const auto k = carla::ros2::ComputeIntrinsics(640u, 480u, fov);
    EXPECT_NEAR(k.cx, 320.0, kEps);
    EXPECT_NEAR(k.cy, 240.0, kEps);
  }
}
