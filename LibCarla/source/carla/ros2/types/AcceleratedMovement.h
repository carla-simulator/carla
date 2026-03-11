// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/ros2/types/Acceleration.h"
#include "carla/ros2/types/AngularVelocity.h"
#include "carla/ros2/types/Timestamp.h"
#include "carla/geom/Velocity.h"
#include "carla/geom/AngularVelocity.h"
#include "carla/geom/Quaternion.h"
#include "carla/ros2/types/CoordinateSystemTransform.h"
#include "geometry_msgs/msg/AccelWithCovariance.h"
#include "geometry_msgs/msg/TwistWithCovariance.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Track the movement state including the angular acceleration based on speed upates
  Allow access for absolute and relative acceleration, velocity and twist in ROS coordinate system

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

  void Update(
    carla::geom::Velocity const& linear_velocity,
    carla::ros2::types::AngularVelocity const& angular_velocity,
    carla::geom::Acceleration const& linear_acceleration,
    carla::geom::Quaternion const& quaternion,
    Timestamp const& timestamp) {

    _acceleration._carla_linear_acceleration = linear_acceleration;
    float delta_seconds = static_cast<float>(timestamp.Stamp() - _timestamp.Stamp());
    if (delta_seconds > 1e-9) {
      _acceleration._carla_angular_acceleration_rad = (angular_velocity.GetAngularVelocityRad() - _angular_velocity.GetAngularVelocityRad()) / delta_seconds;
    }

    _linear_velocity = linear_velocity;
    _angular_velocity = angular_velocity;
    _quaternion = quaternion;
    _timestamp = timestamp;
  }

  Acceleration const & GetAcceleration() const {
    return _acceleration;
  }

  Acceleration GetRelativeAcceleration() const {
    return _acceleration.GetRelative(_quaternion);
  }

  geometry_msgs::msg::Accel absolute_accel() const {
    return GetAcceleration().accel();
  }

  geometry_msgs::msg::AccelWithCovariance absolute_accel_with_covariance() const {
    geometry_msgs::msg::AccelWithCovariance _ros_accel_with_covariance;
    _ros_accel_with_covariance.accel(absolute_accel());
    return _ros_accel_with_covariance;
  }

  geometry_msgs::msg::Accel relative_accel() const {
    return GetRelativeAcceleration().accel();
  }

  geometry_msgs::msg::AccelWithCovariance relative_accel_with_covariance() const {
    geometry_msgs::msg::AccelWithCovariance _ros_accel_with_covariance;
    _ros_accel_with_covariance.accel(relative_accel());
    return _ros_accel_with_covariance;
  }

  carla::geom::Velocity const& LinearVelocity() const {
    return _linear_velocity;
  }

  geometry_msgs::msg::Vector3 absolute_linear_velocity() const {
    return CoordinateSystemTransform::TransformLinearAxisMsg(_linear_velocity);;
  }

  carla::geom::Velocity RelativeLinearVelocity() const {
    return _quaternion.InverseRotatedVector(_linear_velocity);
  }

  geometry_msgs::msg::Vector3 relative_linear_velocity() const {
    return CoordinateSystemTransform::TransformLinearAxisMsg(RelativeLinearVelocity());;
  }

  carla::ros2::types::AngularVelocity AngularVelocity() const {
    return _angular_velocity;
  }

  geometry_msgs::msg::Vector3 absolute_angular_velocity() const {
    return AngularVelocity().angular_velocity();
  }

  carla::ros2::types::AngularVelocity RelativeAngularVelocity() const {
    return AngularVelocity().GetRelative(_quaternion);
  }

  geometry_msgs::msg::Vector3 relative_angular_velocity() const {
    return RelativeAngularVelocity().angular_velocity();
  }

  geometry_msgs::msg::Twist absolute_twist() const {
    geometry_msgs::msg::Twist ros_twist;
    ros_twist.linear() = absolute_linear_velocity();
    ros_twist.angular() = absolute_angular_velocity();
    return ros_twist;
  }

  geometry_msgs::msg::TwistWithCovariance absolute_twist_with_covariance() const {
    geometry_msgs::msg::TwistWithCovariance _ros_twist_with_covariance;
    _ros_twist_with_covariance.twist(absolute_twist());
    return _ros_twist_with_covariance;
  }

  geometry_msgs::msg::Twist relative_twist() const {
    geometry_msgs::msg::Twist ros_twist;
    ros_twist.linear() = relative_linear_velocity();
    ros_twist.angular() = relative_angular_velocity();
    return ros_twist;
  }

  geometry_msgs::msg::TwistWithCovariance relative_twist_with_covariance() const {
    geometry_msgs::msg::TwistWithCovariance _ros_twist_with_covariance;
    _ros_twist_with_covariance.twist(relative_twist());
    return _ros_twist_with_covariance;
  }

  carla::ros2::types::Timestamp const& Timestamp() const {
    return _timestamp;
  }

private:
  carla::geom::Velocity _linear_velocity;
  carla::ros2::types::AngularVelocity _angular_velocity;
  carla::geom::Quaternion _quaternion;
  carla::ros2::types::Timestamp _timestamp;
  carla::ros2::types::Acceleration _acceleration;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla