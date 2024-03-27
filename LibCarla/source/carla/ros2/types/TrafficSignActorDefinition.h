// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/types/ActorNameDefinition.h"

namespace carla {
namespace ros2 {
namespace types {

struct TrafficSignActorDefinition : public ActorNameDefinition {
  TrafficSignActorDefinition(ActorNameDefinition const &actor_definitions) : ActorNameDefinition(actor_definitions) {}
  virtual ~TrafficSignActorDefinition() = default;
};

}  // namespace types
}  // namespace ros2
}  // namespace carla

namespace std {

inline std::string to_string(carla::ros2::types::TrafficSignActorDefinition const &actor_definition) {
  return "TrafficSignActor(" + to_string(static_cast<carla::ros2::types::ActorNameDefinition>(actor_definition)) + ")";
}

}  // namespace std