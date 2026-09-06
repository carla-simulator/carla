// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/RadarPolarToCartesian.h>

#include <cmath>

namespace {
constexpr float kEps = 1e-5f;
}  // namespace

TEST(RadarPolarToCartesian, depth_zero_collapses_to_origin) {
  const auto p = carla::ros2::RadarPolarToCartesian(0.0f, 0.0f, 0.0f);
  EXPECT_NEAR(p[0], 0.0f, kEps);
  EXPECT_NEAR(p[1], 0.0f, kEps);
  EXPECT_NEAR(p[2], 0.0f, kEps);
}

TEST(RadarPolarToCartesian, pure_depth_lands_on_x_axis) {
  // azimuth = 0, altitude = 0: point is straight in front of the sensor along
  // the positive x axis at distance depth.
  const auto p = carla::ros2::RadarPolarToCartesian(10.0f, 0.0f, 0.0f);
  EXPECT_NEAR(p[0], 10.0f, kEps);
  EXPECT_NEAR(p[1], 0.0f, kEps);
  EXPECT_NEAR(p[2], 0.0f, kEps);
}

TEST(RadarPolarToCartesian, positive_azimuth_flips_y_to_negative) {
  // azimuth = +pi/2 with the per-axis sign flip means y component picks up
  // sin(-pi/2) * cos(0) = -1. depth = 10 -> y = -10.
  constexpr float kHalfPi = static_cast<float>(M_PI) / 2.0f;
  const auto p = carla::ros2::RadarPolarToCartesian(10.0f, kHalfPi, 0.0f);
  EXPECT_NEAR(p[0], 0.0f, kEps);
  EXPECT_NEAR(p[1], -10.0f, kEps);
  EXPECT_NEAR(p[2], 0.0f, kEps);
}

TEST(RadarPolarToCartesian, positive_altitude_lifts_z) {
  // altitude = +pi/2, azimuth = 0: point straight up the +z axis.
  constexpr float kHalfPi = static_cast<float>(M_PI) / 2.0f;
  const auto p = carla::ros2::RadarPolarToCartesian(10.0f, 0.0f, kHalfPi);
  EXPECT_NEAR(p[0], 0.0f, kEps);
  EXPECT_NEAR(p[1], 0.0f, kEps);
  EXPECT_NEAR(p[2], 10.0f, kEps);
}

TEST(RadarPolarToCartesian, negative_azimuth_mirrors_y) {
  // azimuth = -pi/2 -> y component picks up sin(pi/2) = +1.
  constexpr float kHalfPi = static_cast<float>(M_PI) / 2.0f;
  const auto p = carla::ros2::RadarPolarToCartesian(10.0f, -kHalfPi, 0.0f);
  EXPECT_NEAR(p[0], 0.0f, kEps);
  EXPECT_NEAR(p[1], 10.0f, kEps);
  EXPECT_NEAR(p[2], 0.0f, kEps);
}

TEST(RadarPolarToCartesian, combined_polar_matches_hand_computed) {
  // depth = 2, azimuth = pi/4, altitude = pi/6.
  constexpr float depth = 2.0f;
  const float azimuth = static_cast<float>(M_PI) / 4.0f;
  const float altitude = static_cast<float>(M_PI) / 6.0f;
  const auto p = carla::ros2::RadarPolarToCartesian(depth, azimuth, altitude);

  const float expected_x = depth * std::cos(azimuth) * std::cos(-altitude);
  const float expected_y = depth * std::sin(-azimuth) * std::cos(altitude);
  const float expected_z = depth * std::sin(altitude);

  EXPECT_NEAR(p[0], expected_x, kEps);
  EXPECT_NEAR(p[1], expected_y, kEps);
  EXPECT_NEAR(p[2], expected_z, kEps);
}
