// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/TransformQuaternion.h>

#include <cmath>

namespace {
constexpr float kEps = 1e-5f;
constexpr float kSqrtHalf = 0.70710678118654752440f;  // sqrt(2)/2

void ExpectNearArray3(
    const std::array<float, 3> &actual,
    const std::array<float, 3> &expected,
    float eps = kEps) {
  EXPECT_NEAR(actual[0], expected[0], eps);
  EXPECT_NEAR(actual[1], expected[1], eps);
  EXPECT_NEAR(actual[2], expected[2], eps);
}

void ExpectNearArray4(
    const std::array<float, 4> &actual,
    const std::array<float, 4> &expected,
    float eps = kEps) {
  EXPECT_NEAR(actual[0], expected[0], eps);
  EXPECT_NEAR(actual[1], expected[1], eps);
  EXPECT_NEAR(actual[2], expected[2], eps);
  EXPECT_NEAR(actual[3], expected[3], eps);
}

}  // namespace

TEST(TransformQuaternion, identity_rotation_is_w_one) {
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f);
  ExpectNearArray3(tf.translation, {0.0f, 0.0f, 0.0f});
  ExpectNearArray4(tf.rotation, {1.0f, 0.0f, 0.0f, 0.0f});
}

TEST(TransformQuaternion, translation_y_axis_is_mirrored) {
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      3.0f, 4.0f, 5.0f,
      0.0f, 0.0f, 0.0f);
  ExpectNearArray3(tf.translation, {3.0f, -4.0f, 5.0f});
}

TEST(TransformQuaternion, pure_pitch_ninety_lands_on_minus_y_half) {
  // CARLA pitch = +90 deg flips to rx = -pi/2; with all other components
  // identity the quaternion is (cos(-pi/4), 0, sin(-pi/4), 0).
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      0.0f, 0.0f, 0.0f,
      90.0f, 0.0f, 0.0f);
  ExpectNearArray4(tf.rotation, {kSqrtHalf, 0.0f, -kSqrtHalf, 0.0f});
}

TEST(TransformQuaternion, pure_yaw_ninety_lands_on_minus_z_half) {
  // CARLA yaw = +90 deg flips to ry = -pi/2; the quaternion picks up z =
  // sin(-pi/4).
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      0.0f, 0.0f, 0.0f,
      0.0f, 90.0f, 0.0f);
  ExpectNearArray4(tf.rotation, {kSqrtHalf, 0.0f, 0.0f, -kSqrtHalf});
}

TEST(TransformQuaternion, pure_roll_ninety_lands_on_plus_x_half) {
  // CARLA roll = +90 deg keeps the sign: rz = +pi/2, quaternion x = sin(pi/4).
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 90.0f);
  ExpectNearArray4(tf.rotation, {kSqrtHalf, kSqrtHalf, 0.0f, 0.0f});
}

TEST(TransformQuaternion, quaternion_stays_unit_norm) {
  for (float pitch : {-180.0f, -45.0f, 0.0f, 30.0f, 90.0f, 175.0f}) {
    for (float yaw : {-90.0f, 0.0f, 60.0f}) {
      for (float roll : {-30.0f, 0.0f, 45.0f}) {
        const auto tf = carla::ros2::TransformFromCarlaRotation(
            0.0f, 0.0f, 0.0f,
            pitch, yaw, roll);
        const float norm =
            tf.rotation[0] * tf.rotation[0]
            + tf.rotation[1] * tf.rotation[1]
            + tf.rotation[2] * tf.rotation[2]
            + tf.rotation[3] * tf.rotation[3];
        EXPECT_NEAR(norm, 1.0f, 1e-4f)
            << "pitch=" << pitch << " yaw=" << yaw << " roll=" << roll;
      }
    }
  }
}

TEST(TransformQuaternion, regression_guard_against_halved_deg_to_rad) {
  // The pre-d9cfc79fb spelling used M_PI_2 / 180 in place of M_PI / 180,
  // halving every rotation. A 180-degree pitch should land at quaternion
  // w=0 (pi/2 half-angle, cos(pi/2)=0); the buggy spelling would land at
  // w=cos(pi/4)=sqrt(2)/2. Guard against a regression to that spelling.
  const auto tf = carla::ros2::TransformFromCarlaRotation(
      0.0f, 0.0f, 0.0f,
      180.0f, 0.0f, 0.0f);
  EXPECT_NEAR(tf.rotation[0], 0.0f, 1e-3f);
  EXPECT_GT(std::abs(tf.rotation[2]), 0.9f);
}
