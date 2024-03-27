// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h"
#include "carla/ros2/types/CoordinateSystemTransform.h"
#include "carla/ros2/types/Quaternion.h"
#include "geometry_msgs/msg/Pose.h"
#include "geometry_msgs/msg/Transform.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert a carla transform to a ROS transform
*/
class Transform {
public:
  Transform() = default;

  /**
   * carla_transform: the carla Transform
   */
  explicit Transform(const carla::geom::Transform& carla_transform, const carla::geom::Quaternion& carla_quaternion)
    : _carla_transform(carla_transform), _carla_quaternion(carla_quaternion) {
    init_ros_transform();
  }

  explicit Transform(const geometry_msgs::msg::Pose& pose) {
    _ros_transform.translation().x(pose.position().x());
    _ros_transform.translation().y(pose.position().y());
    _ros_transform.translation().z(pose.position().z());
    _ros_transform.rotation(pose.orientation());
    // switch y-axis from right to left -> negate y-axis
    carla::geom::Vector3D ros_location;
    ros_location.x = float(pose.position().x());
    ros_location.y = float(pose.position().y());
    ros_location.z = float(pose.position().z());
    _carla_transform.location = CoordinateSystemTransform::TransformLinearAxixVector3D(ros_location);
    _carla_quaternion = carla::ros2::types::Quaternion(_ros_transform.rotation()).GetQuaternion();
  }

#ifdef LIBCARLA_INCLUDED_FROM_UE4
  Transform(const FTransform& ue4_transform)
    : _carla_transform(ue4_transform), _carla_quaternion(ue4_transform.GetRotation()) {
    init_ros_transform();
  }
#endif  // LIBCARLA_INCLUDED_FROM_UE4

  ~Transform() = default;
  Transform(const Transform&) = default;
  Transform& operator=(const Transform&) = default;
  Transform(Transform&&) = default;
  Transform& operator=(Transform&&) = default;

  /**
   * The resulting ROS geometry_msgs::msg::Transform
   *
   * Uses ROS naming convention
   */
  const geometry_msgs::msg::Transform& transform() const {
    return _ros_transform;
  }

  /**
   * Get the geometry_msgs::msg::Pose that is identical with the geometry_msgs::msg::Transform
   *
   * Uses ROS naming convention
   */
  const geometry_msgs::msg::Pose pose() const {
    geometry_msgs::msg::Pose ros_pose;
    ros_pose.position().x(_ros_transform.translation().x());
    ros_pose.position().y(_ros_transform.translation().y());
    ros_pose.position().z(_ros_transform.translation().z());
    ros_pose.orientation(_ros_transform.rotation());
    return ros_pose;
  }

  /**
   * The carla Transform
   *
   * Uses CARLA naming convention
   */
  const carla::geom::Transform& GetTransform() const {
    return _carla_transform;
  }

  /**
   * The carla Location
   *
   * Uses CARLA naming convention
   */
  const carla::geom::Location& GetLocation() const {
    return _carla_transform.location;
  }

  /**
   * The carla Rotator
   *
   * Uses CARLA naming convention
   */
  const carla::geom::Rotation& GetRotator() const {
    return _carla_transform.rotation;
  }

  /**
   * The carla Quaternion
   *
   * Uses CARLA naming convention
   */
  const carla::geom::Quaternion& GetQuaternion() const {
    return _carla_quaternion;
  }

private:
  void init_ros_transform() {
    // switch y-axis from right to left -> negate y-axis
    _ros_transform.translation() = CoordinateSystemTransform::TransformLinearAxisMsg(_carla_transform.location);
    _ros_transform.rotation(carla::ros2::types::Quaternion(_carla_quaternion).quaternion());
  }

  // keep the carla types for local
  carla::geom::Transform _carla_transform;
  carla::geom::Quaternion _carla_quaternion;
  geometry_msgs::msg::Transform _ros_transform;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla