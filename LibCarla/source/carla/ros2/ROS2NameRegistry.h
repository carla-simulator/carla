// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <list>
#include <map>
#include <mutex>
#include <set>

#include "carla/ros2/ROS2NameRecord.h"

namespace carla {
namespace ros2 {

/**
 * @brief Registry to manage topic/frame handling in the sense of parent/child role_name, duplicates, etc.
 *  Calls to this object are thread-safe
 */
class ROS2NameRegistry {
public:
  ROS2NameRegistry() = default;
  ~ROS2NameRegistry() = default;

  void Clear();

  // registering and unregistering records
  void RegisterRecord(ROS2NameRecord const* record);
  void UnregisterRecord(ROS2NameRecord const* record);

  // attaching actors to each other
  void AttachActors(ActorId const child, ActorId const parent);

  struct TopicAndFrame {
    TopicAndFrame(std::string topic_name = "", std::string frame_id = "")
      : _topic_name(topic_name), _frame_id(frame_id) {}
    std::string _topic_name = "";
    std::string _frame_id = "";
  };

  carla::rpc::ActorId ParentActorId(ActorId const child_id) const {
    std::lock_guard<std::mutex> lock(access_mutex);
    carla::rpc::ActorId parent_actor_id = 0;
    auto find_result = parent_map.find(child_id);
    if (find_result != parent_map.end()) {
      parent_actor_id = find_result->second;
    }
    return parent_actor_id;
  }

  /*!
    @brief returns the shortest common prefix of all registered topic names for this actor_id
  */
  std::string TopicPrefix(ActorId const actor_id);

  std::string FrameId(ROS2NameRecord const* record) {
    std::lock_guard<std::mutex> lock(access_mutex);
    return GetTopicAndFrameLocked(record)._frame_id;
  }
  std::string TopicName(ROS2NameRecord const* record) {
    std::lock_guard<std::mutex> lock(access_mutex);
    return GetTopicAndFrameLocked(record)._topic_name;
  }

  std::string ParentFrameId(ROS2NameRecord const* record) {
    std::lock_guard<std::mutex> lock(access_mutex);
    auto parent_frame_id = GetParentTopicAndFrameLocked(record)._frame_id;
    if (parent_frame_id.empty()) {
      parent_frame_id = "map";
    } else if (parent_frame_id.find("rt/carla") == 0) {
      // fully qualified parent
      return parent_frame_id.substr(8);
    }
    return parent_frame_id;
  }
  std::string ParentTopicName(ROS2NameRecord const* record) {
    std::lock_guard<std::mutex> lock(access_mutex);
    return GetParentTopicAndFrameLocked(record)._topic_name;
  }

private:
  ROS2NameRegistry(const ROS2NameRegistry&) = delete;
  ROS2NameRegistry& operator=(const ROS2NameRegistry&) = delete;
  ROS2NameRegistry(ROS2NameRegistry&&) = delete;
  ROS2NameRegistry& operator=(ROS2NameRegistry&&) = delete;

  bool IsTopicNameAvailable(TopicAndFrame const& topic_and_frame, std::string const & individual_name);
  TopicAndFrame ExpandTopicName(TopicAndFrame const& topic_and_frame, std::string const & postfix);

  struct KeyType {
    explicit KeyType(ROS2NameRecord const* record) : _record(record), _actor_id(record->_actor_name_definition->id) {}

    bool operator<(const KeyType& other) const {
      if (_actor_id == other._actor_id) {
        return _record < other._record;
      } else {
        return _actor_id < other._actor_id;
      }
    }

    ROS2NameRecord const* const _record;
    carla::rpc::ActorId _actor_id;
  };

  // locked operations
  TopicAndFrame const& GetTopicAndFrameLocked(ROS2NameRecord const* record);
  TopicAndFrame const& GetParentTopicAndFrameLocked(ROS2NameRecord const* record);

  TopicAndFrame const& GetTopicAndFrameLocked(KeyType const& key);
  void UpdateTopicAndFrameLocked(carla::rpc::ActorId actor_id);
  std::map<KeyType, TopicAndFrame>::iterator CreateTopicAndFrameLocked(KeyType const& key);

  mutable std::mutex access_mutex;
  std::set<ROS2NameRecord const*> record_set;
  std::map<carla::rpc::ActorId, carla::rpc::ActorId> parent_map;
  std::map<KeyType, TopicAndFrame> topic_and_frame_map;
};

}  // namespace ros2
}  // namespace carla
