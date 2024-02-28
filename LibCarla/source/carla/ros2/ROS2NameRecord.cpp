// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/ROS2NameRecord.h"

#include "carla/ros2/ROS2.h"
#include "carla/ros2/types/SensorActorDefinition.h"
#include "carla/ros2/types/TrafficLightActorDefinition.h"
#include "carla/ros2/types/TrafficSignActorDefinition.h"
#include "carla/ros2/types/VehicleActorDefinition.h"
#include "carla/ros2/types/WalkerActorDefinition.h"

namespace carla {
namespace ros2 {

ROS2NameRecord::ROS2NameRecord(std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition)
  : _actor_name_definition(actor_name_definition) {
  ROS2::GetInstance()->GetNameRegistry()->RegisterRecord(this);
}

ROS2NameRecord::~ROS2NameRecord() {
  ROS2::GetInstance()->GetNameRegistry()->UnregisterRecord(this);
}

std::string ROS2NameRecord::get_topic_name(std::string postfix) const {
  auto topic_name = ROS2::GetInstance()->GetNameRegistry()->TopicName(this);
  if (!postfix.empty()) {
    topic_name += "/" + postfix;
  }
  return topic_name;
}

std::string ROS2NameRecord::frame_id() const {
  return ROS2::GetInstance()->GetNameRegistry()->FrameId(this);
}

std::string ROS2NameRecord::parent_frame_id() const {
  return ROS2::GetInstance()->GetNameRegistry()->ParentFrameId(this);
}

}  // namespace ros2
}  // namespace carla
