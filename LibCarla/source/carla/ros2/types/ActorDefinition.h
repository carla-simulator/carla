// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/BoundingBox.h"
#include "carla/rpc/EnvironmentObject.h"
#include "carla/ros2/types/ActorNameDefinition.h"
#include "carla/ros2/types/Polygon.h"
#include "carla/ros2/types/Transform.h"

namespace carla {
namespace ros2 {
namespace types {

using ActorSetTransformCallback = std::function<void(carla::ros2::types::Transform &)>;

struct ActorDefinition : public ActorNameDefinition {
  ActorDefinition(ActorNameDefinition const &actor_name_definition, carla::geom::BoundingBox const &bounding_box_)
    : ActorNameDefinition(actor_name_definition), bounding_box(bounding_box_)
  {
    normalize_bounding_box();
  }

  ActorDefinition(const carla::rpc::EnvironmentObject &env_object, bool enabled_for_ros_) 
    : bounding_box(env_object.bounding_box) {

    id = env_object.id;
    type_id = env_object.name;
    object_type=std::to_string(env_object.type);
    base_type="environment_object";
    enabled_for_ros = enabled_for_ros_;
    city_object_label=env_object.type;

    normalize_bounding_box();
  }

  carla::geom::BoundingBox bounding_box;

private:
  void normalize_bounding_box() {
    // Unreal Bounding Boxes seem to be not always correct (some were NaN)
    if ( std::fpclassify(bounding_box.extent.x) != FP_NORMAL )
    {
      bounding_box.extent.x = 0.1f;
      bounding_box.extent.y = 0.1f;
      bounding_box.extent.z = 0.1f;
    } 
  }
};



}  // namespace types
}  // namespace ros2
}  // namespace carla

namespace std {

inline std::string to_string(carla::ros2::types::ActorDefinition const &actor_definition) {
  return "Actor(" + to_string(static_cast<carla::ros2::types::ActorNameDefinition>(actor_definition)) + ")";
}

}  // namespace std