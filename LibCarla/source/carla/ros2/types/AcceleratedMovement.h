// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/geom/Acceleration.h"
#include "carla/ros2/types/AngularVelocity.h"
#include "carla/ros2/types/Speed.h"
#include "carla/ros2/types/Timestamp.h"
#include "carla/ros2/types/Twist.h"
#include "geometry_msgs/msg/Accel.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Track accelerations based on Speed upates

  Considers the conversion from left-handed system (unreal) to right-handed
  system (ROS)
*/
class AcceleratedMovement {
public:
  AcceleratedMovement() = default;
  ~AcceleratedMovement() = default;
  AcceleratedMovement(const AcceleratedMovement&) = default;
  AcceleratedMovement& operator=(const AcceleratedMovement&) = default;
  AcceleratedMovement(AcceleratedMovement&&) = default;
  AcceleratedMovement& operator=(AcceleratedMovement&&) = default;

  void UpdateSpeed(Speed const& speed, AngularVelocity const& angular_velocity, Timestamp const& timestamp) {
    auto delta_seconds = timestamp.Stamp() - _last_timestamp.Stamp();
    if (delta_seconds > 1e-9) {
      auto last_linear_velocity_ros = _last_speed.LinearVelocityROS();
      auto current_linear_velocity_ros = speed.LinearVelocityROS();
      auto current_linear_acceleration_ros = (current_linear_velocity_ros - last_linear_velocity_ros) / delta_seconds;
      _ros_accel.linear().x(current_linear_acceleration_ros.x);
      _ros_accel.linear().y(current_linear_acceleration_ros.y);
      _ros_accel.linear().z(current_linear_acceleration_ros.z);

      auto last_angular_velocity_ros = _last_angular_velocity.AngularVelocityROS();
      auto current_angular_velocity_ros = angular_velocity.AngularVelocityROS();
      auto current_angular_acceleration_ros =
          (current_angular_velocity_ros - last_angular_velocity_ros) / delta_seconds;
      _ros_accel.angular().x(current_angular_acceleration_ros.x);
      _ros_accel.angular().y(current_angular_acceleration_ros.y);
      _ros_accel.angular().z(current_angular_acceleration_ros.z);
    }
    _last_speed = speed;
    _last_angular_velocity = angular_velocity;
    _last_timestamp = timestamp;
  }

  /**
   * The resulting ROS geometry_msgs::msg::Accel
   */
  geometry_msgs::msg::Accel accel() const {
    return _ros_accel;
  }

  /**
   * The resulting ROS geometry_msgs::msg::twist
   */
  geometry_msgs::msg::Twist twist() const {
    carla::ros2::types::Twist ros_twist(_last_speed, _last_angular_velocity);
    return ros_twist.twist();
  }

  carla::ros2::types::Speed const& Speed() const {
    return _last_speed;
  }

  carla::ros2::types::AngularVelocity const& AngularVelocity() const {
    return _last_angular_velocity;
  }

  carla::ros2::types::Timestamp const& Timestamp() const {
    return _last_timestamp;
  }

private:
  carla::ros2::types::Speed _last_speed{carla::geom::Vector3D(), carla::geom::Quaternion()};
  carla::ros2::types::AngularVelocity _last_angular_velocity{carla::geom::AngularVelocity()};
  carla::ros2::types::Timestamp _last_timestamp;
  geometry_msgs::msg::Accel _ros_accel;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla