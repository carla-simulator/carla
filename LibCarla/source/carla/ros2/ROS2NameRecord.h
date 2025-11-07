// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/types/ActorNameDefinition.h"

namespace carla {
namespace ros2 {

class DdsDomainParticipantImpl;

/**
 * @brief class to manage the most topic/frame handling in the sense of parent/child role_name, duplicates, etc.
 */
class ROS2NameRecord {
public:
  ROS2NameRecord(std::shared_ptr<carla::ros2::types::ActorNameDefinition> actor_name_definition);
  ~ROS2NameRecord();

  ROS2NameRecord(const ROS2NameRecord&) = delete;
  ROS2NameRecord& operator=(const ROS2NameRecord&) = delete;
  ROS2NameRecord(ROS2NameRecord&&) = default;
  ROS2NameRecord& operator=(ROS2NameRecord&&) = default;

  std::string frame_id() const;

  std::string parent_frame_id() const;

  std::string get_topic_name(std::string postfix = "") const;

  std::shared_ptr<carla::ros2::types::ActorNameDefinition> _actor_name_definition;
};

}  // namespace ros2
}  // namespace carla
