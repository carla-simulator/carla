// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/types/ActorNameDefinition.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "carla_msgs/msg/CarlaTrafficLightStatus.h"

namespace carla {
namespace ros2 {
namespace types {

inline uint8_t GetTrafficLightState(carla::sensor::data::ActorDynamicState const &actor_dynamic_state) {
  switch (actor_dynamic_state.state.traffic_light_data.state) {
    case carla::rpc::TrafficLightState::Red:
      return carla_msgs::msg::CarlaTrafficLightStatus_Constants::RED;
    case carla::rpc::TrafficLightState::Yellow:
      return carla_msgs::msg::CarlaTrafficLightStatus_Constants::YELLOW;
    case carla::rpc::TrafficLightState::Green:
      return carla_msgs::msg::CarlaTrafficLightStatus_Constants::GREEN;
    case carla::rpc::TrafficLightState::Off:
      return carla_msgs::msg::CarlaTrafficLightStatus_Constants::OFF;
    case carla::rpc::TrafficLightState::Unknown:
    default:
      return carla_msgs::msg::CarlaTrafficLightStatus_Constants::UNKNOWN;
  }
}

struct TrafficLightActorDefinition : public ActorNameDefinition {
  TrafficLightActorDefinition(ActorNameDefinition const &actor_definitions) : ActorNameDefinition(actor_definitions) {}
};
}  // namespace types
}  // namespace ros2
}  // namespace carla

namespace std {

inline std::string to_string(carla::ros2::types::TrafficLightActorDefinition const &actor_definition) {
  return "TrafficLightActor(" + to_string(static_cast<carla::ros2::types::ActorNameDefinition>(actor_definition)) + ")";
}

}  // namespace std