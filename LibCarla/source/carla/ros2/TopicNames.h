// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>
#include <unordered_map>

namespace carla {
namespace ros2 {
namespace detail {

inline std::string BuildActorBaseTopicName(
    void *actor,
    const std::unordered_map<void *, void *> &parents,
    const std::unordered_map<void *, std::string> &names) {
  const auto name_it = names.find(actor);
  const std::string name = name_it != names.end() ? name_it->second : "";
  const auto parent_it = parents.find(actor);
  if (parent_it != parents.end()) {
    const std::string parent_topic = BuildActorBaseTopicName(parent_it->second, parents, names);
    if (name.empty()) {
      return parent_topic;
    }
    return parent_topic + (parent_topic == "rt/carla/" ? "" : "/") + name;
  }
  return "rt/carla/" + name;
}

} // namespace detail
} // namespace ros2
} // namespace carla
