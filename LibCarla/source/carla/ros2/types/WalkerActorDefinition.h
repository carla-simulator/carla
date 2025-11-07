// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <functional>
#include "carla/ros2/types/ActorDefinition.h"
#include "carla/ros2/types/WalkerControl.h"
#include "carla/rpc/WalkerControl.h"

namespace carla {
namespace ros2 {
namespace types {

using WalkerControlCallback = std::function<void(WalkerControl const &)>;

struct WalkerActorDefinition : public ActorDefinition {
  WalkerActorDefinition(ActorDefinition const &actor_definition) : ActorDefinition(actor_definition) {}
  virtual ~WalkerActorDefinition() = default;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla

namespace std {

inline std::string to_string(carla::ros2::types::WalkerActorDefinition const &actor_definition) {
  return "WalkerActor(" + to_string(static_cast<carla::ros2::types::ActorDefinition>(actor_definition)) + ")";
}

}  // namespace std
