// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/BoundingBox.h"
#include "carla/ros2/types/ActorNameDefinition.h"
#include "carla/ros2/types/Polygon.h"

namespace carla {
namespace ros2 {
namespace types {

struct ActorDefinition : public ActorNameDefinition {
  ActorDefinition(ActorNameDefinition const &actor_name_definition, carla::geom::BoundingBox bounding_box_,
                  carla::ros2::types::Polygon vertex_polygon_)
    : ActorNameDefinition(actor_name_definition), bounding_box(bounding_box_), vertex_polygon(vertex_polygon_) {}

  carla::geom::BoundingBox bounding_box;
  carla::ros2::types::Polygon vertex_polygon;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla

namespace std {

inline std::string to_string(carla::ros2::types::ActorDefinition const &actor_definition) {
  return "Actor(" + to_string(static_cast<carla::ros2::types::ActorNameDefinition>(actor_definition)) + ")";
}

}  // namespace std