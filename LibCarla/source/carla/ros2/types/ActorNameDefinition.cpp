// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/types/ActorNameDefinition.h"
#include "carla/ros2/types/SensorActorDefinition.h"

#include "carla/ros2/ROS2NameRegistry.h"

namespace carla {
namespace ros2 {
namespace types {

carla_msgs::msg::CarlaActorInfo ActorNameDefinition::carla_actor_info(std::shared_ptr<ROS2NameRegistry> name_registry) const {
    carla_msgs::msg::CarlaActorInfo actor_info;
    actor_info.id(id);
    actor_info.type(type_id);
    actor_info.rosname(ros_name);
    actor_info.rolename(role_name);
    actor_info.object_type(object_type);
    actor_info.base_type(base_type);
    if ( name_registry != nullptr ) {
        actor_info.parent_id(name_registry->ParentActorId(id));
        auto topic_prefix = name_registry->TopicPrefix(id);
        if ( topic_prefix.length() >= 3 ) {
            // remove "rt/" prefix
            topic_prefix = topic_prefix.substr(3);
        }
        if ( topic_prefix.front() == '/') {
            // remove any leading "/"
            topic_prefix.erase(topic_prefix.begin());
        }
        actor_info.topic_prefix(topic_prefix);
        auto sensor_actor_definition = dynamic_cast<carla::ros2::types::SensorActorDefinition const*>(this);
        if ( sensor_actor_definition != nullptr ) {
            if ( id == 0 ) {
                // the world and multiple world sensors share the same id
                actor_info.frame_id("map");
            } else {
                actor_info.frame_id(name_registry->FrameId(id));
            }
        }
    } else {
        // environment objects
        actor_info.parent_id(0);
        actor_info.topic_prefix("environment/");
    }
    for (auto const &attribute: attributes) {
        diagnostic_msgs::msg::KeyValue key_value;
        key_value.key(attribute.first);
        key_value.value(attribute.second);
        actor_info.attributes().push_back(key_value);
    }
    actor_info.city_object_label(static_cast<uint8_t>(city_object_label));

    return actor_info;
}

}  // namespace types
}  // namespace ros2
}  // namespace carla
