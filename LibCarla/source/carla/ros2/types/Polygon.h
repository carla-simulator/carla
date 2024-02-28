// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <array>
#include <memory>
#include <vector>

#include "carla/geom/Location.h"
#include "carla/ros2/types/CoordinateSystemTransform.h"
#include "geometry_msgs/msg/Point32.h"

namespace carla {
namespace ros2 {
namespace types {

/**
  Convert a carla (linear) acceleration to a ROS accel (linear part)

  Considers the conversion from left-handed system (unreal) to right-handed
  system (ROS)
*/
class Polygon {
public:
  /**
   * The representation of an (dynamic) Polygon in the sense of derived_Polygon_msgs::msg::Polygon.
   *
   * classification is one of the derived_Polygon_msgs::msg::Polygon_Constants::CLASSIFICATION_* constants
   */
  Polygon(std::array<carla::geom::Location, 8> const &vertices)
    : _ros_polygon(std::make_shared<std::vector<geometry_msgs::msg::Point32>>()) {
    _ros_polygon->reserve(vertices.size());
    for (auto const &vertex : vertices) {
      _ros_polygon->push_back(CoordinateSystemTransform::TransformLocationToPoint32Msg(vertex));
    }
  }
#ifdef LIBCARLA_INCLUDED_FROM_UE4
  Polygon() : _ros_polygon(std::make_shared<std::vector<geometry_msgs::msg::Point32>>()) {}
  void SetGlobalVertices(TArray<FVector> const &vertices) {
    _ros_polygon->reserve(vertices.Num());
    for (auto const &vertex : vertices) {
      _ros_polygon->push_back(CoordinateSystemTransform::TransformLocationToPoint32Msg(carla::geom::Location(vertex)));
    }
  }
#endif  // LIBCARLA_INCLUDED_FROM_UE4

  ~Polygon() = default;
  Polygon(const Polygon &) = default;
  Polygon &operator=(const Polygon &) = default;
  Polygon(Polygon &&) = default;
  Polygon &operator=(Polygon &&) = default;

  std::shared_ptr<std::vector<geometry_msgs::msg::Point32>> polygon() const {
    return _ros_polygon;
  }

private:
  // store a shared_ptr to prevent from vector copies
  std::shared_ptr<std::vector<geometry_msgs::msg::Point32>> _ros_polygon;
};

}  // namespace types
}  // namespace ros2
}  // namespace carla