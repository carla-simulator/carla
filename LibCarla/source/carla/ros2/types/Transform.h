// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h"
#include "carla/ros2/types/CoordinateSystemTransform.h"
#include "carla/ros2/types/Quaternion.h"
#include "geometry_msgs/msg/PoseWithCovariance.h"
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
    : _carla_location(carla_transform.location)
    , _carla_rotation(carla_transform.rotation)
    , _carla_rotation_initialized(true)
    , _carla_quaternion(carla_quaternion) {
    init_ros_transform();
  }

  explicit Transform(const carla::geom::Location& carla_location, const carla::geom::Quaternion& carla_quaternion)
    : _carla_location(carla_location)
    , _carla_rotation_initialized(false)
    , _carla_quaternion(carla_quaternion) {
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
    _carla_location = CoordinateSystemTransform::TransformLinearAxixVector3D(ros_location);
    auto const quaternion = carla::ros2::types::Quaternion(_ros_transform.rotation());
    _carla_quaternion = quaternion.GetQuaternion();
    _carla_rotation_initialized = false;
  }

#ifdef LIBCARLA_INCLUDED_FROM_UE4
  Transform(const FTransform& ue4_transform)
    : _carla_location(ue4_transform.GetLocation())
    , _carla_rotation(ue4_transform.Rotator())
    , _carla_rotation_initialized(true)
    , _carla_quaternion(ue4_transform.GetRotation()) {
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
   * Get the geometry_msgs::msg::PoseWithCovariance that is identical with the geometry_msgs::msg::Transform
   *
   * Uses ROS naming convention
   */
  const geometry_msgs::msg::PoseWithCovariance pose_with_covariance() const {
    geometry_msgs::msg::PoseWithCovariance ros_pose_with_covariance;
    ros_pose_with_covariance.pose(pose());
    return ros_pose_with_covariance;
  }

  /**
   * The carla Transform
   *
   * Uses CARLA naming convention
   */
  carla::geom::Transform GetTransform() {
    EnsureCarlaRotatorInitialized();
    return carla::geom::Transform(_carla_location, _carla_rotation);
  }

  /**
   * The carla Location
   *
   * Uses CARLA naming convention
   */
  const carla::geom::Location& GetLocation() const {
    return _carla_location;
  }

  /**
   * The carla Rotator
   *
   * Uses CARLA naming convention
   */
  const carla::geom::Rotation& GetRotator() {
    EnsureCarlaRotatorInitialized();
    return _carla_rotation;
  }

  /**
   * The carla Quaternion
   *
   * Uses CARLA naming convention
   */
  const carla::geom::Quaternion& GetQuaternion() const {
    return _carla_quaternion;
  }

  /**
   * Transform the in_point
   */
  void TransformPoint(carla::geom::Vector3D &in_point) const {
      carla::geom::Vector3D rotated_point = _carla_quaternion.RotatedPoint(in_point); // First rotate
      in_point = rotated_point + carla::geom::Vector3D(_carla_location);  // Then translate
  }

  /**
   * Get the transform of this relative to the provided base transform.
   */
  carla::ros2::types::Transform GetRelativeTransform(carla::ros2::types::Transform const &basis) const {
    auto const relative_quaternion_new_base = basis.GetQuaternion().Inverse() * GetQuaternion();
    auto const relative_location_current_base = GetLocation() - basis.GetLocation();
    carla::geom::Location const relative_location_new_base = carla::geom::Vector3D(basis.GetQuaternion().RotatedPoint(relative_location_current_base));
    carla::ros2::types::Transform relative_transform(relative_location_new_base, relative_quaternion_new_base);
    return relative_transform;
  }

private:
  void EnsureCarlaRotatorInitialized() {
    if ( !_carla_rotation_initialized ) {
      _carla_rotation_initialized = true;
      _carla_rotation = _carla_quaternion.Rotator();
    }
  }

  void init_ros_transform() {
    // switch y-axis from right to left -> negate y-axis
    _ros_transform.translation() = CoordinateSystemTransform::TransformLinearAxisMsg(_carla_location);
    _ros_transform.rotation(carla::ros2::types::Quaternion(_carla_quaternion).quaternion());
  }

  // keep the carla types, but with rotation optional (only to be calculated if required in case of ROS input)
  // be aware: rotation calculation requires some sin/cos calls and is rather expensive
  carla::geom::Rotation _carla_rotation;
  bool _carla_rotation_initialized = false;
  carla::geom::Location _carla_location;
  carla::geom::Quaternion _carla_quaternion;
  geometry_msgs::msg::Transform _ros_transform;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla


namespace std {

inline std::string to_string(geometry_msgs::msg::Transform const &transform) {
  return "ROSTransform(translation(x=" +
    std::to_string(transform.translation().x()) + ", y=" +
    std::to_string(transform.translation().y()) + ", z=" +
    std::to_string(transform.translation().z()) + "), " +
    "rotation(x=" +
    std::to_string(transform.rotation().x()) + ", y=" +
    std::to_string(transform.rotation().y()) + ", z=" +
    std::to_string(transform.rotation().z()) + ", w=" +
    std::to_string(transform.rotation().w()) + "))";
}

inline std::string to_string(carla::ros2::types::Transform const &transform) {
  return "Transform(" + std::to_string(transform.transform()) + " CARLA: " +
     std::to_string(transform.GetQuaternion()) + std::to_string(transform.GetLocation());
}

}  // namespace std