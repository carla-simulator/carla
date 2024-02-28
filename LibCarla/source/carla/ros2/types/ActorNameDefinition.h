// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

#include "carla/rpc/ActorId.h"

namespace carla {
namespace ros2 {
namespace types {

struct ActorNameDefinition {
  ActorNameDefinition(carla::rpc::ActorId id_ = 0u, std::string type_id_ = "", std::string ros_name_ = "",
                      std::string role_name_ = "", std::string object_type_ = "", std::string base_type_ = "")
    : id(id_),
      type_id(type_id_),
      ros_name(ros_name_),
      role_name(role_name_),
      object_type(object_type_),
      base_type(base_type_) {}

  static std::shared_ptr<carla::ros2::types::ActorNameDefinition> CreateFromRoleName(std::string const &role_name_) {
    auto actor_name_definition = std::make_shared<carla::ros2::types::ActorNameDefinition>();
    actor_name_definition->role_name = role_name_;
    return actor_name_definition;
  }

  virtual ~ActorNameDefinition() = default;

  carla::rpc::ActorId id;
  std::string type_id;
  std::string ros_name;
  std::string role_name;
  std::string object_type;
  std::string base_type;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla

namespace std {

inline std::string to_string(carla::ros2::types::ActorNameDefinition const &actor_definition) {
  return "ActorName(actor_id=" + std::to_string(actor_definition.id) + " type_id=" + actor_definition.type_id;
  +" ros_name=" + actor_definition.ros_name;
  +" role_name=" + actor_definition.role_name;
  +" object_type=" + actor_definition.object_type;
  +" base_type=" + actor_definition.base_type + ")";
}

}  // namespace std