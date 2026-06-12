// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cmath>

#include "carla/geom/Math.h"
#include "carla/geom/Rotation.h"
#include "carla/geom/Vector3D.h"
#include "carla/ros2/types/msg/Quaternion.h"
#include "carla/ros2/types/msg/Vector3.h"

namespace carla {
namespace ros2 {

  /// Conversions from UE coordinates (left-handed, z-up, degrees) to ROS
  /// coordinates (right-handed, z-up, radians). Same convention as
  /// CarlaTransformPublisher::ComputeTransform: y negated; roll, -pitch,
  /// -yaw converted from degrees to radians.

  constexpr float UE_DEG_TO_RAD = geom::Math::Pi<float>() / 180.0f;

  /// UE rotation (degrees) to ROS orientation quaternion.
  inline msg::Quaternion ue_rotation_to_ros_quaternion(
      const geom::Rotation &rotation) {
    const float rx = (rotation.pitch * -1.0f) * UE_DEG_TO_RAD;
    const float ry = (rotation.yaw * -1.0f) * UE_DEG_TO_RAD;
    const float rz = rotation.roll * UE_DEG_TO_RAD;

    const float cr = cosf(rz * 0.5f);
    const float sr = sinf(rz * 0.5f);
    const float cp = cosf(rx * 0.5f);
    const float sp = sinf(rx * 0.5f);
    const float cy = cosf(ry * 0.5f);
    const float sy = sinf(ry * 0.5f);

    msg::Quaternion quaternion;
    quaternion.w = cr * cp * cy + sr * sp * sy;
    quaternion.x = sr * cp * cy - cr * sp * sy;
    quaternion.y = cr * sp * cy + sr * cp * sy;
    quaternion.z = cr * cp * sy - sr * sp * cy;
    return quaternion;
  }

  /// UE world-frame vector to ROS axes (y negated).
  inline msg::Vector3 ue_vector_to_ros_vector(const geom::Vector3D &vector) {
    msg::Vector3 ros_vector;
    ros_vector.x = vector.x;
    ros_vector.y = -vector.y;
    ros_vector.z = vector.z;
    return ros_vector;
  }

  /// UE world-frame velocity to a ROS body-frame velocity: project onto the
  /// vehicle axes given by rotation, then flip to right-handed (left = -right).
  inline msg::Vector3 ue_world_velocity_to_ros_body_velocity(
      const geom::Vector3D &velocity,
      const geom::Rotation &rotation) {
    const geom::Vector3D forward = rotation.GetForwardVector();
    const geom::Vector3D right = rotation.GetRightVector();
    const geom::Vector3D up = rotation.GetUpVector();

    msg::Vector3 body_velocity;
    body_velocity.x = geom::Math::Dot(velocity, forward);
    body_velocity.y = -geom::Math::Dot(velocity, right);
    body_velocity.z = geom::Math::Dot(velocity, up);
    return body_velocity;
  }

  /// UE angular velocity (deg/s) to ROS (rad/s), pitch and yaw rates negated.
  inline msg::Vector3 ue_angular_velocity_to_ros(
      const geom::Vector3D &angular_velocity) {
    msg::Vector3 ros_angular_velocity;
    ros_angular_velocity.x = angular_velocity.x * UE_DEG_TO_RAD;
    ros_angular_velocity.y = -angular_velocity.y * UE_DEG_TO_RAD;
    ros_angular_velocity.z = -angular_velocity.z * UE_DEG_TO_RAD;
    return ros_angular_velocity;
  }

}  // namespace ros2
}  // namespace carla
