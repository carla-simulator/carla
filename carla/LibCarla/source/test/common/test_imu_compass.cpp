// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/ImuMath.h>

#include <cmath>

namespace {

constexpr float kEps = 1e-5f;

// Recover yaw (radians, REP-103) from a (w, x, y, z) quaternion that is known
// to be yaw-only (x == y == 0).
float YawFrom(const std::array<float, 4> &q) {
  return 2.0f * std::atan2(q[3], q[0]);
}

}  // namespace

TEST(ImuMath, north_is_pi_over_two) {
  // Compass = 0 means the vehicle faces magnetic North. REP-103 yaw measured
  // counter-clockwise from East should be +pi/2.
  const auto q = carla::ros2::OrientationFromCompass(0.0f);

  ASSERT_NEAR(q[1], 0.0f, kEps);
  ASSERT_NEAR(q[2], 0.0f, kEps);
  ASSERT_NEAR(q[0], std::cos(static_cast<float>(M_PI) / 4.0f), kEps);
  ASSERT_NEAR(q[3], std::sin(static_cast<float>(M_PI) / 4.0f), kEps);
  ASSERT_NEAR(YawFrom(q), static_cast<float>(M_PI) / 2.0f, kEps);
}

TEST(ImuMath, east_is_zero_yaw) {
  // Compass = pi/2 means the vehicle faces magnetic East. REP-103 yaw = 0.
  const auto q = carla::ros2::OrientationFromCompass(static_cast<float>(M_PI) / 2.0f);

  ASSERT_NEAR(q[1], 0.0f, kEps);
  ASSERT_NEAR(q[2], 0.0f, kEps);
  ASSERT_NEAR(q[0], 1.0f, kEps);
  ASSERT_NEAR(q[3], 0.0f, kEps);
  ASSERT_NEAR(YawFrom(q), 0.0f, kEps);
}

TEST(ImuMath, south_is_negative_pi_over_two) {
  // Compass = pi means the vehicle faces magnetic South. REP-103 yaw = -pi/2.
  const auto q = carla::ros2::OrientationFromCompass(static_cast<float>(M_PI));

  ASSERT_NEAR(q[1], 0.0f, kEps);
  ASSERT_NEAR(q[2], 0.0f, kEps);
  ASSERT_NEAR(YawFrom(q), -static_cast<float>(M_PI) / 2.0f, kEps);
}

TEST(ImuMath, west_is_pi) {
  // Compass = 3*pi/2 means the vehicle faces magnetic West. REP-103 yaw is
  // pi/2 - 3*pi/2 = -pi (equivalently +pi); compare on |yaw| to avoid the
  // atan2 wrap-around at the +/-pi seam.
  const auto q = carla::ros2::OrientationFromCompass(
      3.0f * static_cast<float>(M_PI) / 2.0f);

  ASSERT_NEAR(q[1], 0.0f, kEps);
  ASSERT_NEAR(q[2], 0.0f, kEps);
  ASSERT_NEAR(std::abs(YawFrom(q)), static_cast<float>(M_PI), kEps);
}

TEST(ImuMath, is_not_the_regressed_pi_over_four) {
  // Regression guard: the spelling shipped on ue5-dev before this fix
  // returned pi/4 - compass instead of pi/2 - compass. With compass = 0 the
  // buggy yaw is pi/4. Verify the helper does not produce that anymore.
  const auto q = carla::ros2::OrientationFromCompass(0.0f);
  ASSERT_GT(std::abs(YawFrom(q) - static_cast<float>(M_PI) / 4.0f), 1e-3f);
}
