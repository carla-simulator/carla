// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
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
  void AttachActors(carla::streaming::detail::actor_id_type const child, carla::streaming::detail::actor_id_type const parent);

  struct TopicAndFrame {
    TopicAndFrame(std::string topic_name = "", std::string frame_id = "")
      : _topic_name(topic_name), _frame_id(frame_id) {}
    std::string _topic_name = "";
    std::string _frame_id = "";
  };

  carla::streaming::detail::actor_id_type ParentActorId(carla::streaming::detail::actor_id_type const child_id) const {
    std::lock_guard<std::mutex> lock(access_mutex);
    carla::streaming::detail::actor_id_type parent_actor_id = 0;
    auto find_result = parent_map.find(child_id);
    if (find_result != parent_map.end()) {
      parent_actor_id = find_result->second;
    }
    return parent_actor_id;
  }

  /*!
    @brief returns the shortest common prefix of all registered topic names for this actor_id
  */
  std::string TopicPrefix(carla::streaming::detail::actor_id_type const actor_id);
  
  /*!
    @brief returns the FrameId for this actor_id
  */
  std::string FrameId(carla::streaming::detail::actor_id_type const actor_id);

  std::string FrameId(ROS2NameRecord const* record) {
    std::lock_guard<std::mutex> lock(access_mutex);
    std::string frame_id = GetTopicAndFrameLocked(record)._frame_id;
    if (frame_id.empty()) {
      frame_id = "map";
    }
    return frame_id;
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

  bool IsTopicNameAvailable(TopicAndFrame const& topic_and_frame, std::string const& individual_name);
  // per default frame and topic postfix are considered to be equal
  TopicAndFrame ExpandTopicName(TopicAndFrame const& topic_and_frame, std::string const& postfix_topic, std::string const& postfix_frame="");

  struct KeyType {
    explicit KeyType(ROS2NameRecord const* record) : 
    _actor_name_definition(record->_actor_name_definition) {}

    bool operator<(const KeyType& other) const {
      // the actor name definition shared pointer is the differentiating piece
      return _actor_name_definition < other._actor_name_definition;
    }
    carla::streaming::detail::actor_id_type actor_id()const { return _actor_name_definition->id; }

    std::shared_ptr<carla::ros2::types::ActorNameDefinition const> _actor_name_definition;
    mutable uint32_t _number_of_register_calls{0u};
  };

  // locked operations
  TopicAndFrame const& GetTopicAndFrameLocked(KeyType const& key);
  TopicAndFrame const& GetParentTopicAndFrameLocked(KeyType const& key);

  TopicAndFrame const& GetTopicAndFrameLocked(ROS2NameRecord const* record){
    return GetTopicAndFrameLocked(KeyType(record));
  }
  TopicAndFrame const& GetParentTopicAndFrameLocked(ROS2NameRecord const* record) {
    return GetParentTopicAndFrameLocked(KeyType(record));
  }

  void UpdateTopicAndFrameLocked(carla::streaming::detail::actor_id_type actor_id);
  std::map<KeyType, TopicAndFrame>::iterator CreateTopicAndFrameLocked(KeyType const& key);

  mutable std::mutex access_mutex;
  std::set<KeyType> record_set;
  std::map<carla::streaming::detail::actor_id_type, carla::streaming::detail::actor_id_type> parent_map;
  std::map<KeyType, TopicAndFrame> topic_and_frame_map;
  std::set<carla::streaming::detail::actor_id_type> missing_parents;
};

}  // namespace ros2
}  // namespace carla
