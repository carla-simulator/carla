// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/LidarAzimuth.h>

#include <cmath>

using carla::ros2::LidarScanAzimuth;

namespace {

constexpr float kPi = 3.14159265358979323846f;

}  // namespace

// Measured clockwise from +x in the ROS frame (x forward, y left).
TEST(LidarAzimuth, quarter_turns_run_clockwise_from_forward) {
  EXPECT_NEAR(LidarScanAzimuth(1.0f, 0.0f), 0.0f, 1e-6f);          // forward
  EXPECT_NEAR(LidarScanAzimuth(0.0f, -1.0f), 0.5f * kPi, 1e-6f);   // right
  EXPECT_NEAR(LidarScanAzimuth(-1.0f, 0.0f), kPi, 1e-6f);          // behind
  EXPECT_NEAR(LidarScanAzimuth(0.0f, 1.0f), 1.5f * kPi, 1e-6f);    // left
}

TEST(LidarAzimuth, stays_in_zero_to_two_pi) {
  for (int i = 0; i < 3600; ++i) {
    const float angle = static_cast<float>(i) * 0.1f * kPi / 180.0f;
    const float azimuth = LidarScanAzimuth(std::cos(angle), std::sin(angle));
    EXPECT_GE(azimuth, 0.0f);
    EXPECT_LT(azimuth, 2.0f * kPi);
  }
  // atan2(+0, x) of a point just left of forward must not land on 2*pi.
  EXPECT_LT(LidarScanAzimuth(1.0f, 1e-30f), 2.0f * kPi);
  EXPECT_GE(LidarScanAzimuth(1.0f, -0.0f), 0.0f);
}

// CARLA's lidar sweeps clockwise seen from above, i.e. towards -y in the ROS
// frame. Along a ring the azimuth must grow from one point to the next, which
// is what Autoware's ring outlier filter groups neighbouring points by.
TEST(LidarAzimuth, increases_along_a_clockwise_sweep) {
  const float step = 0.2f * kPi / 180.0f;
  for (int i = 1; i < 1800; ++i) {
    const float previous = -static_cast<float>(i - 1) * step;
    const float current = -static_cast<float>(i) * step;
    const float a = LidarScanAzimuth(std::cos(previous), std::sin(previous));
    const float b = LidarScanAzimuth(std::cos(current), std::sin(current));
    EXPECT_GT(b, a) << "at step " << i;
    EXPECT_NEAR(b - a, step, 1e-4f) << "at step " << i;
  }
}
