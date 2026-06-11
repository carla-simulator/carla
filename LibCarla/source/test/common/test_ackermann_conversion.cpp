// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/subscribers/AckermannControlConversion.h>

#include <cmath>
#include <limits>
#include <variant>

TEST(AckermannConversion, identity_field_mapping) {
  const auto control = carla::ros2::FromAckermannDrive(0.1f, 0.5f, 12.0f, 1.0f, 0.25f);

  EXPECT_FLOAT_EQ(control.steer, 0.1f);
  EXPECT_FLOAT_EQ(control.steer_speed, 0.5f);
  EXPECT_FLOAT_EQ(control.speed, 12.0f);
  EXPECT_FLOAT_EQ(control.acceleration, 1.0f);
  EXPECT_FLOAT_EQ(control.jerk, 0.25f);
}

TEST(AckermannConversion, zero_inputs) {
  const auto control = carla::ros2::FromAckermannDrive(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  EXPECT_FLOAT_EQ(control.steer, 0.0f);
  EXPECT_FLOAT_EQ(control.steer_speed, 0.0f);
  EXPECT_FLOAT_EQ(control.speed, 0.0f);
  EXPECT_FLOAT_EQ(control.acceleration, 0.0f);
  EXPECT_FLOAT_EQ(control.jerk, 0.0f);
}

TEST(AckermannConversion, negative_speed_passthrough) {
  // ROS ackermann_msgs allows negative speed (reverse). Verify CARLA passes the
  // sign through without clamping or remapping to a reverse flag.
  const auto control = carla::ros2::FromAckermannDrive(0.0f, 0.0f, -3.0f, -0.5f, 0.0f);
  EXPECT_FLOAT_EQ(control.speed, -3.0f);
  EXPECT_FLOAT_EQ(control.acceleration, -0.5f);
}

TEST(AckermannConversion, large_steering_angle_passthrough) {
  // The conversion seam does not enforce vehicle-physical limits; clipping is the
  // simulator's job. Confirm large angles survive the field-mapping step.
  const auto control = carla::ros2::FromAckermannDrive(3.14159f, 100.0f, 0.0f, 0.0f, 0.0f);
  EXPECT_FLOAT_EQ(control.steer, 3.14159f);
  EXPECT_FLOAT_EQ(control.steer_speed, 100.0f);
}

TEST(AckermannConversion, nan_propagation) {
  // NaN inputs propagate to the output struct unchanged so downstream code can
  // detect and reject them at the simulator boundary.
  const float nan_val = std::numeric_limits<float>::quiet_NaN();
  const auto control = carla::ros2::FromAckermannDrive(nan_val, 0.0f, 0.0f, 0.0f, 0.0f);
  EXPECT_TRUE(std::isnan(control.steer));
  EXPECT_FALSE(std::isnan(control.steer_speed));
}

TEST(ROS2CallbackData, visit_routes_ackermann_to_correct_overload) {
  struct Visitor {
    int visited_vehicle{0};
    int visited_ackermann{0};
    void operator()(const carla::ros2::VehicleControl &) { ++visited_vehicle; }
    void operator()(const carla::ros2::AckermannControl &) { ++visited_ackermann; }
  } visitor;

  carla::ros2::ROS2CallbackData data =
      carla::ros2::FromAckermannDrive(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  std::visit(visitor, data);

  EXPECT_EQ(visitor.visited_ackermann, 1);
  EXPECT_EQ(visitor.visited_vehicle, 0);
}

TEST(ROS2CallbackData, visit_routes_vehicle_to_correct_overload) {
  struct Visitor {
    int visited_vehicle{0};
    int visited_ackermann{0};
    void operator()(const carla::ros2::VehicleControl &) { ++visited_vehicle; }
    void operator()(const carla::ros2::AckermannControl &) { ++visited_ackermann; }
  } visitor;

  carla::ros2::VehicleControl payload{};
  payload.throttle = 1.0f;
  carla::ros2::ROS2CallbackData data = payload;
  std::visit(visitor, data);

  EXPECT_EQ(visitor.visited_vehicle, 1);
  EXPECT_EQ(visitor.visited_ackermann, 0);
}
