// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/AngularVelocity.h"
#include "carla/geom/Math.h"
#include "geometry_msgs/msg/Accel.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert a carla AngularVelocity to a ROS accel

  Considers the conversion from left-handed system (unreal) to right-handed
  system (ROS)

*/
class AngularVelocity {
public:
  AngularVelocity() = default;
  ~AngularVelocity() = default;
  AngularVelocity(const AngularVelocity&) = default;
  AngularVelocity& operator=(const AngularVelocity&) = default;
  AngularVelocity(AngularVelocity&&) = default;
  AngularVelocity& operator=(AngularVelocity&&) = default;

  /**
   * carla_AngularVelocity: the carla linear AngularVelocity
   */
  explicit AngularVelocity(const carla::geom::AngularVelocity& carla_angular_velocity) {
    _angular_velocity_ros.x = -carla::geom::Math::ToRadians(carla_angular_velocity.x);  // -(forward =  forward)
    _angular_velocity.x(_angular_velocity_ros.x);
    _angular_velocity_ros.y = carla::geom::Math::ToRadians(carla_angular_velocity.y);  // -(  right = -left  )
    _angular_velocity.y(_angular_velocity_ros.y);
    _angular_velocity_ros.z = -carla::geom::Math::ToRadians(carla_angular_velocity.z);  // -(     up =  up     )
    _angular_velocity.z(_angular_velocity_ros.z);
  }
#ifdef LIBCARLA_INCLUDED_FROM_UE4
  AngularVelocity(const FVector& carla_angular_velocity)
    : AngularVelocity(
          carla::geom::Vector3D(carla_angular_velocity.X, carla_angular_velocity.Y, carla_angular_velocity.Z)) {}
#endif  // LIBCARLA_INCLUDED_FROM_UE4

  /**
   * The resulting ROS geometry_msgs::msg::Vector3
   */
  geometry_msgs::msg::Vector3 angular_velocity() const {
    return _angular_velocity;
  }

  /**
   * The angular velocity as carla::geom::Vector3D but in ROS coordinates
   */
  carla::geom::AngularVelocity AngularVelocityROS() const {
    return _angular_velocity_ros;
  }

private:
  carla::geom::AngularVelocity _angular_velocity_ros;
  geometry_msgs::msg::Vector3 _angular_velocity;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla