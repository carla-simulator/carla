// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/types/ActorNameDefinition.h"

#include "carla/ros2/ROS2NameRegistry.h"

namespace carla {
namespace ros2 {
namespace types {

carla_msgs::msg::CarlaActorInfo ActorNameDefinition::carla_actor_info(std::shared_ptr<ROS2NameRegistry> name_registry) const {
    carla_msgs::msg::CarlaActorInfo actor_info;
    actor_info.id(id);
    actor_info.parent_id(name_registry->ParentActorId(id));
    actor_info.type(type_id);
    actor_info.rosname(ros_name);
    actor_info.rolename(role_name);
    actor_info.object_type(object_type);
    actor_info.base_type(base_type);
    auto topic_prefix = name_registry->TopicPrefix(id);
    if ( topic_prefix.length() >= 3 )
    {
        // remove "rt" prefix
        actor_info.topic_prefix(topic_prefix.substr(3));
    }
    return actor_info;
}

}  // namespace types
}  // namespace ros2
}  // namespace carla
