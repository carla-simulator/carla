// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>
#include <sstream>
#include <map>

#include "carla/streaming/detail/Types.h"
#include "carla/rpc/ObjectLabel.h"
#include "carla_msgs/msg/CarlaActorInfo.h"
#include "carla/ros2/ROS2TopicVisibilityDefaultMode.h"

namespace carla {
namespace ros2 {

class ROS2NameRegistry;

namespace types {

struct ActorNameDefinition {

  ActorNameDefinition() {};

  ActorNameDefinition(ActorNameDefinition const &other, carla::rpc::CityObjectLabel city_object_label_)
    : id(other.id),
      type_id(other.type_id),
      ros_name(other.ros_name),
      role_name(other.role_name),
      object_type(other.object_type),
      base_type(other.base_type),
      enabled_for_ros(other.enabled_for_ros),
      publish_tf(other.publish_tf),
      frame_id(other.frame_id),
      city_object_label(city_object_label_),
      attributes(other.attributes) {
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4
  ActorNameDefinition(uint64_t id_, std::string type_id_, FActorDescription const &Description, carla::ros2::ROS2TopicVisibilityDefaultMode const topic_visibility_default_mode) 
    : id(id_),
      type_id(type_id_),
      ros_name(std::string(TCHAR_TO_UTF8(*Description.GetAttribute("ros_name").Value))),
      role_name(std::string(TCHAR_TO_UTF8(*Description.GetAttribute("role_name").Value))),
      object_type(std::string(TCHAR_TO_UTF8(*Description.GetAttribute("object_type").Value))),
      base_type(std::string(TCHAR_TO_UTF8(*Description.GetAttribute("base_type").Value))),
      enabled_for_ros(false),
      frame_id(TCHAR_TO_UTF8(*Description.GetAttribute("ros_frame_id").Value)),
      city_object_label(carla::rpc::CityObjectLabel::None) {

      std::string enabled_for_ros_string = TCHAR_TO_UTF8(*Description.GetAttribute("enabled_for_ros").Value);
      if ( (enabled_for_ros_string == "") && (topic_visibility_default_mode == carla::ros2::ROS2TopicVisibilityDefaultMode::eOn )) {
          enabled_for_ros = true;
      }
      else {
        enabled_for_ros = Description.GetAttribute("enabled_for_ros").Value.ToBool();
      }
      std::string ros_publish_tf_string = TCHAR_TO_UTF8(*Description.GetAttribute("ros_publish_tf").Value);
      if ( (ros_publish_tf_string == "") && (topic_visibility_default_mode == carla::ros2::ROS2TopicVisibilityDefaultMode::eOn )) {
          publish_tf = true;
      }
      else {
        publish_tf = Description.GetAttribute("ros_publish_tf").Value.ToBool();
      }

      for (auto const &ActorVariation: Description.Variations) {
        std::string key = TCHAR_TO_UTF8(*ActorVariation.Key);
        // filter out some values already stored explicitly
        if ((key == "ros_name")
          || (key == "role_name")
          || (key == "object_type")
          || (key == "base_type")) {
          continue;
         }
         std::string value = TCHAR_TO_UTF8(*ActorVariation.Value.Value);
         attributes[key] = value;
      }    }
#endif

  static std::shared_ptr<carla::ros2::types::ActorNameDefinition> CreateFromRoleName(std::string const &role_name_, 
     carla::ros2::ROS2TopicVisibilityDefaultMode const topic_visibility_default_mode = carla::ros2::ROS2TopicVisibilityDefaultMode::eOn ) {
    auto actor_name_definition = std::make_shared<carla::ros2::types::ActorNameDefinition>();
    actor_name_definition->role_name = role_name_;
    actor_name_definition->base_type = "world";
    actor_name_definition->enabled_for_ros = topic_visibility_default_mode == carla::ros2::ROS2TopicVisibilityDefaultMode::eOn;
    return actor_name_definition;
  }

  carla_msgs::msg::CarlaActorInfo carla_actor_info(std::shared_ptr<ROS2NameRegistry> name_registry) const;

  virtual ~ActorNameDefinition() = default;

  uint64_t id{0u};
  std::string type_id;
  std::string ros_name;
  std::string role_name;
  std::string object_type;
  std::string base_type;
  bool enabled_for_ros{false};
  bool publish_tf{true};
  std::string frame_id;
  carla::rpc::CityObjectLabel city_object_label{carla::rpc::CityObjectLabel::None};
  std::map<std::string, std::string> attributes;

};
}  // namespace types
}  // namespace ros2
}  // namespace carla

namespace std {

inline std::string to_string(carla::ros2::types::ActorNameDefinition const &actor_definition) {
  std::stringstream str;
  str << "ActorName(actor_id=" << std::to_string(actor_definition.id)
      << " type_id=" << actor_definition.type_id
      << " ros_name=" << actor_definition.ros_name
      << " role_name=" << actor_definition.role_name
      << " object_type=" << actor_definition.object_type
      << " base_type=" << actor_definition.base_type
      << " enabled_for_ros=" << std::to_string(actor_definition.enabled_for_ros)
      << " publish_tf=" << std::to_string(actor_definition.publish_tf)
      << " frame_id=" << actor_definition.frame_id;
  for (auto const &attribute: actor_definition.attributes) {
    str << " " << attribute.first << "=" << attribute.second;
  }
  str << ")";
  return str.str();
}

}  // namespace std