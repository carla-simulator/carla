// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

// Converts the five scalar fields of a ROS ackermann_msgs/AckermannDriveStamped
// message payload into the CARLA AckermannControl POD. The wire-side
// AckermannDriveStamped FastDDS type is decomposed in AckermannControlSubscriber.cpp;
// this seam exists so the numeric mapping can be exercised by a unit test that does
// not link FastDDS.
inline AckermannControl FromAckermannDrive(
    float steering_angle,
    float steering_angle_velocity,
    float speed,
    float acceleration,
    float jerk) {
  AckermannControl control{};
  control.steer = steering_angle;
  control.steer_speed = steering_angle_velocity;
  control.speed = speed;
  control.acceleration = acceleration;
  control.jerk = jerk;
  return control;
}

}  // namespace ros2
}  // namespace carla
