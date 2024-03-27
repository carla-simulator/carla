// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"
#include "geometry_msgs/msg/Point32.h"
#include "geometry_msgs/msg/Vector3.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert a carla (linear) CoordinateSystemTransform to a ROS accel (linear part)

  Considers the conversion from left-handed system (unreal) to right-handed
  system (ROS)
*/
class CoordinateSystemTransform {
public:
  /**
   * @param \in carla_linear_values: the carla linear values provided provided by UE4 coordinate system
   * @returns values in ROS coordinate system (x:forward = forward, y: right = -left, z; up = up)
   */
  static geometry_msgs::msg::Vector3 TransformLinearAxisMsg(carla::geom::Vector3D const &carla_linear_values) {
    geometry_msgs::msg::Vector3 result;
    result.x(carla_linear_values.x);
    result.y(-carla_linear_values.y);
    result.z(carla_linear_values.z);
    return result;
  }

  static geometry_msgs::msg::Point32 TransformLocationToPoint32Msg(carla::geom::Location const &carla_location) {
    geometry_msgs::msg::Point32 result;
    result.x(carla_location.x);
    result.y(-carla_location.y);
    result.z(carla_location.z);
    return result;
  }

  static geometry_msgs::msg::Vector3 TransformLocationToVector3Msg(carla::geom::Location const &carla_location) {
    geometry_msgs::msg::Vector3 result;
    result.x(carla_location.x);
    result.y(-carla_location.y);
    result.z(carla_location.z);
    return result;
  }

  static carla::geom::Vector3D TransformLinearAxixVector3D(carla::geom::Vector3D const &carla_linear_values) {
    carla::geom::Vector3D result(carla_linear_values);
    result.y = -result.y;
    return result;
  }
};
}  // namespace types
}  // namespace ros2
}  // namespace carla