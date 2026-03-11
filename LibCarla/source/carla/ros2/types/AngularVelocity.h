// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/AngularVelocity.h"
#include "carla/geom/Math.h"
#include "geometry_msgs/msg/Vector3.h"

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

  enum class AngularVelocityMode {
    DEGREE,
    RADIAN
  };

  /**
   * carla_AngularVelocity: the carla linear AngularVelocity
   */
  AngularVelocity(const carla::geom::AngularVelocity& carla_angular_velocity, AngularVelocityMode mode) {
      if ( mode == AngularVelocityMode::RADIAN) {
        _carla_angular_velocity_rad.x = carla_angular_velocity.x;
        _carla_angular_velocity_rad.y = carla_angular_velocity.y;
        _carla_angular_velocity_rad.z = carla_angular_velocity.z;
      } else {
        _carla_angular_velocity_rad.x = carla::geom::Math::ToRadians(carla_angular_velocity.x);
        _carla_angular_velocity_rad.y = carla::geom::Math::ToRadians(carla_angular_velocity.y);
        _carla_angular_velocity_rad.z = carla::geom::Math::ToRadians(carla_angular_velocity.z);
      }
  }
#ifdef LIBCARLA_INCLUDED_FROM_UE4
  AngularVelocity(const FVector& carla_angular_velocity)
    : AngularVelocity(
          carla::geom::Vector3D(carla_angular_velocity.X, carla_angular_velocity.Y, carla_angular_velocity.Z)) {}
#endif  // LIBCARLA_INCLUDED_FROM_UE4

  /**
   * The resulting ROS geometry_msgs::msg::Vector3 in ROS coordinates
   */
  geometry_msgs::msg::Vector3 angular_velocity() const {
    geometry_msgs::msg::Vector3 angular_velocity_ros;
    angular_velocity_ros.x() = -_carla_angular_velocity_rad.x;  // -(forward =  forward)
    angular_velocity_ros.y() = _carla_angular_velocity_rad.y;  // -(  right = -left  )
    angular_velocity_ros.z() = -_carla_angular_velocity_rad.z;  // -(     up =  up     )
    return angular_velocity_ros;
  }

  /**
   * The angular velocity in the carla coordinate system in radians per second
   */
  carla::geom::AngularVelocity const & GetAngularVelocityRad() const {
    return _carla_angular_velocity_rad;
  }

  /**
   * Get the relative angular velocity in the reference frame of the provided transform
   */
  AngularVelocity GetRelative(carla::geom::Quaternion const& quat) const {
    AngularVelocity relative_angular_velocity;
    relative_angular_velocity._carla_angular_velocity_rad = quat.InverseRotatedVector(_carla_angular_velocity_rad);
    return relative_angular_velocity;
  }

private:
  carla::geom::AngularVelocity _carla_angular_velocity_rad;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla