// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <algorithm>

#include "carla/ros2/ROS2NameRecord.h"
#include "carla/ros2/ROS2NameRegistry.h"
#include "carla/ros2/types/SensorActorDefinition.h"
#include "carla/ros2/types/TrafficLightActorDefinition.h"
#include "carla/ros2/types/TrafficSignActorDefinition.h"
#include "carla/ros2/types/VehicleActorDefinition.h"
#include "carla/ros2/types/WalkerActorDefinition.h"

namespace carla {
namespace ros2 {

const ROS2NameRegistry::TopicAndFrame g_empty_topic_and_frame;

void ROS2NameRegistry::Clear() {
  std::lock_guard<std::mutex> lock(access_mutex);
  record_set.clear();
  parent_map.clear();
  topic_and_frame_map.clear();
}

void ROS2NameRegistry::RegisterRecord(ROS2NameRecord const* record) {
  std::lock_guard<std::mutex> lock(access_mutex);
  record_set.insert(record);
}

void ROS2NameRegistry::UnregisterRecord(ROS2NameRecord const* record) {
  std::lock_guard<std::mutex> lock(access_mutex);
  auto const actor_id = record->_actor_name_definition->id;
  record_set.erase(record);

  for (auto iter = parent_map.begin(); iter != parent_map.end(); /*no update of iter*/) {
    if (iter->first == actor_id) {
      // erase this actor from the map
      iter = parent_map.erase(iter);
    } else if (iter->second == actor_id) {
      // if this actor was the parent of another one, erase this dependency
      auto const child = iter->first;
      iter = parent_map.erase(iter);
      // and update child data
      UpdateTopicAndFrameLocked(child);
    } else {
      ++iter;
    }
  }

  for (auto iter = topic_and_frame_map.begin(); iter != topic_and_frame_map.end(); /*no update of iter*/) {
    if (iter->first._record == record) {
      iter = topic_and_frame_map.erase(iter);
    } else {
      ++iter;
    }
  }
}

void ROS2NameRegistry::AttachActors(ActorId const child_id, ActorId const parent_id) {
  std::lock_guard<std::mutex> lock(access_mutex);
  log_debug("ROS2NameRegistry::AttachActors[", child_id, "]: parent=", parent_id);
  auto insert_result = parent_map.insert({child_id, parent_id});
  if (!insert_result.second) {
    // update parent entry
    insert_result.first->second = parent_id;
  }
  // enforce an update the topic and frames of the child
  UpdateTopicAndFrameLocked(child_id);
}

std::string ROS2NameRegistry::TopicPrefix(ActorId const actor_id) {
  std::lock_guard<std::mutex> lock(access_mutex);
  std::string result_topic_name = "";
  for (auto& record : record_set) {
    auto const actor_definition = record->_actor_name_definition;
    if (actor_definition->id == actor_id) {
      auto const topic_name = GetTopicAndFrameLocked(KeyType(record))._topic_name;
      if (result_topic_name.empty()) {
        result_topic_name = topic_name;
      } else {
        auto iter_a = result_topic_name.begin();
        auto iter_b = topic_name.begin();
        while (iter_a != result_topic_name.end() && iter_b != topic_name.end() && (*iter_a == *iter_b)) {
        }
        if (iter_a == result_topic_name.end()) {
          // result_topic_name is already shortest common prefix
        } else if (iter_b == topic_name.end()) {
          // topic_name is new shortest common prefix
          result_topic_name = topic_name;
        } else {
          result_topic_name = {result_topic_name.begin(), iter_a};
        }
      }
    }
  }
  return result_topic_name;
}

ROS2NameRegistry::TopicAndFrame const& ROS2NameRegistry::GetTopicAndFrameLocked(ROS2NameRecord const* record) {
  return GetTopicAndFrameLocked(KeyType(record));
}

ROS2NameRegistry::TopicAndFrame const& ROS2NameRegistry::GetParentTopicAndFrameLocked(
    ROS2NameRecord const* child_record) {
  ActorId const child_id = child_record->_actor_name_definition->id;
  // multiple parent entries are not allowed
  auto find_result = parent_map.find(child_id);
  if (find_result != parent_map.end()) {
    auto const parent_actor_id = find_result->second;
    std::map<KeyType, TopicAndFrame>::iterator parent_iter = topic_and_frame_map.end();
    for (auto iter = topic_and_frame_map.begin(); iter != topic_and_frame_map.end(); ++iter) {
      if (iter->first._actor_id == parent_actor_id) {
        if (parent_iter != topic_and_frame_map.end()) {
          log_error("ROS2NameRegistry::GetParentTopicAndFrameLocked: multiple parent candidates for child ",
                    std::to_string(*child_record->_actor_name_definition), " found. ", " Potential Parents ",
                    std::to_string(*iter->first._record->_actor_name_definition),
                    std::to_string(*parent_iter->first._record->_actor_name_definition),
                    " This is not an expected configuration. Cannot decide. Ignore parent");
          return g_empty_topic_and_frame;
        } else {
          parent_iter = iter;
        }
      }
    }
    if (parent_iter != topic_and_frame_map.end()) {
      return parent_iter->second;
    } else {
      // create the parent topic and frame
      ROS2NameRecord const* parent_record = nullptr;
      for (auto& record : record_set) {
        if (record->_actor_name_definition->id == parent_actor_id) {
          if (parent_record != nullptr) {
            log_error("ROS2NameRegistry::GetParentTopicAndFrameLocked: multiple parent candidates for child ",
                      std::to_string(*child_record->_actor_name_definition), " found. ", " Potential Parents ",
                      std::to_string(*record->_actor_name_definition),
                      std::to_string(*parent_record->_actor_name_definition),
                      " This is not an expected configuration. Cannot decide. Ignore parent");
            return g_empty_topic_and_frame;
          } else {
            parent_record = record;
          }
        }
      }
      if (parent_record != nullptr) {
        KeyType const key(parent_record);
        return CreateTopicAndFrameLocked(key)->second;
      } else {
        log_error("ROS2NameRegistry::GetParentTopicAndFrameLocked: no parent candidate found for child ",
                  std::to_string(*child_record->_actor_name_definition), " found. ",
                  " This is not an expected configuration. Cannot decide. Ignore parent_id=", parent_actor_id);
        return g_empty_topic_and_frame;
      }
    }
  }
  return g_empty_topic_and_frame;
}

ROS2NameRegistry::TopicAndFrame const& ROS2NameRegistry::GetTopicAndFrameLocked(ROS2NameRegistry::KeyType const& key) {
  auto find_result = topic_and_frame_map.find(key);
  if (find_result != topic_and_frame_map.end()) {
    return find_result->second;
  } else {
    return CreateTopicAndFrameLocked(key)->second;
  }
}

void ROS2NameRegistry::UpdateTopicAndFrameLocked(carla::rpc::ActorId actor_id) {
  // update all of this
  for (auto& record : record_set) {
    auto const actor_definition = record->_actor_name_definition;
    if (actor_definition->id == actor_id) {
      KeyType const key(record);
      (void)CreateTopicAndFrameLocked(key);
    }
  }
}

std::string number_to_three_letter_string(uint32_t number) {
  auto number_string = std::to_string(number);
  if (number_string.length() < 3u) {
    number_string.insert(number_string.begin(), 3u - number_string.length(), '0');
  }
  return number_string;
}

std::map<ROS2NameRegistry::KeyType, ROS2NameRegistry::TopicAndFrame>::iterator
ROS2NameRegistry::CreateTopicAndFrameLocked(ROS2NameRegistry::KeyType const& key) {
  auto const actor_definition = key._record->_actor_name_definition;

  TopicAndFrame parent_topic_and_frame;
  auto parent_iter = parent_map.find(key._actor_id);
  if (parent_iter != parent_map.end()) {
    // get the data, if not availble, update also the parent
    parent_topic_and_frame = GetParentTopicAndFrameLocked(key._record);
  }

  ROS2NameRegistry::TopicAndFrame topic_and_frame("rt/carla");
  // first bring in the parent hierarchy if present
  if (!parent_topic_and_frame._topic_name.empty()) {
    if (parent_topic_and_frame._topic_name.find("rt/carla") == 0) {
      topic_and_frame._topic_name = parent_topic_and_frame._topic_name;
    } else {
      topic_and_frame._topic_name += "/" + parent_topic_and_frame._topic_name;
    }
  }
  if (!parent_topic_and_frame._frame_id.empty()) {
    topic_and_frame._frame_id = parent_topic_and_frame._frame_id;
  }

  // let us query the type of actor we have
  auto vehicle_actor_definition =
      std::dynamic_pointer_cast<carla::ros2::types::VehicleActorDefinition>(actor_definition);
  auto walker_actor_definition = std::dynamic_pointer_cast<carla::ros2::types::WalkerActorDefinition>(actor_definition);
  auto sensor_actor_definition = std::dynamic_pointer_cast<carla::ros2::types::SensorActorDefinition>(actor_definition);
  auto traffic_light_actor_definition =
      std::dynamic_pointer_cast<carla::ros2::types::TrafficLightActorDefinition>(actor_definition);
  auto traffic_sign_actor_definition =
      std::dynamic_pointer_cast<carla::ros2::types::TrafficSignActorDefinition>(actor_definition);

  // prefix with generic type prefix
  std::string type;
  if (vehicle_actor_definition != nullptr) {
    type = "vehicles";
  } else if (walker_actor_definition != nullptr) {
    type = "walkers";
  } else if (traffic_light_actor_definition != nullptr) {
    type = "traffic_lights";
  } else if (traffic_sign_actor_definition != nullptr) {
    type = "traffic_signs";
  } else if (sensor_actor_definition != nullptr) {
    type = "sensors";
  } else {
    type = "world";
  }
  // add type
  topic_and_frame._topic_name += "/" + type;
  topic_and_frame._frame_id += "/" + type;

  std::string individual_name;
  if (sensor_actor_definition != nullptr) {
    // on sensors we use the sensor name as additions type prefix
    auto pos = actor_definition->ros_name.find_last_of('.');
    if (pos != std::string::npos) {
      topic_and_frame._topic_name += "/" + actor_definition->ros_name.substr(pos + 1u);
      topic_and_frame._frame_id += "/" + actor_definition->ros_name.substr(pos + 1u);
    } else {
      topic_and_frame._topic_name += "/" + actor_definition->ros_name;
      topic_and_frame._frame_id += "/" + actor_definition->ros_name;
    }
    // and use stream id as individualization
    auto const stream_id_string = "/stream_" + number_to_three_letter_string(sensor_actor_definition->stream_id);
    if (IsTopicNameAvailable(topic_and_frame, stream_id_string)) {
      individual_name = stream_id_string;
    }
  }

  // the role name overrules other individualization
  if (!actor_definition->role_name.empty()) {
    if (IsTopicNameAvailable(topic_and_frame, actor_definition->role_name)) {
      individual_name = actor_definition->role_name;
    }
  }
  // no valid individualization yet, use actor id
  if (individual_name.empty()) {
    auto const actor_id_string = "actor_" + number_to_three_letter_string(actor_definition->id);
    if (IsTopicNameAvailable(topic_and_frame, actor_id_string)) {
      individual_name = actor_id_string;
    }
  }
  // if also this doesn't help, we try with a random number using the actor_id as initialization
  if (individual_name.empty()) {
    std::srand(actor_definition->id);
    individual_name = "randomid_" + number_to_three_letter_string(uint32_t(std::rand()));
  }

  topic_and_frame._topic_name += "/" + individual_name;
  topic_and_frame._frame_id += "/" + individual_name;

  auto insert_result = topic_and_frame_map.insert({key, topic_and_frame});
  if (!insert_result.second) {
    // enforce update if already there
    insert_result.first->second = topic_and_frame;
  }

  return insert_result.first;
}

bool ROS2NameRegistry::IsTopicNameAvailable(TopicAndFrame const& topic_and_frame, std::string const& individual_name) {
  auto topic_name_check = topic_and_frame._topic_name + "/" + individual_name;
  auto iter =
      std::find_if(topic_and_frame_map.begin(), topic_and_frame_map.end(),
                   [topic_name_check](auto const& element) { return element.second._topic_name == topic_name_check; });
  return iter == topic_and_frame_map.end();
}

}  // namespace ros2
}  // namespace carla
