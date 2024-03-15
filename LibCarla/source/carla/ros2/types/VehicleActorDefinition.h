// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <functional>

#include "carla/ros2/ros_types/carla_msgs/msg/CarlaVehicleStatus.h"
#include "carla/ros2/types/ActorDefinition.h"
#include "carla/ros2/types/VehicleAckermannControl.h"
#include "carla/ros2/types/VehicleControl.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/sensor/data/ActorDynamicState.h"

namespace carla {
namespace ros2 {
namespace types {

using VehicleControlCallback = std::function<void(VehicleControl const &)>;

using VehicleAckermannControlCallback = std::function<void(VehicleAckermannControl const &)>;

inline uint8_t GetVehicleControlType(carla::sensor::data::ActorDynamicState const &actor_dynamic_state) {
  switch (actor_dynamic_state.state.vehicle_data.control_type) {
    case carla::rpc::VehicleControlType::AckermannControl:
      return carla_msgs::msg::CarlaVehicleStatus_Constants::ACKERMANN_CONTROL;
    case carla::rpc::VehicleControlType::VehicleControl:
    default:
      return carla_msgs::msg::CarlaVehicleStatus_Constants::VEHICLE_CONTROL;
  }
}

struct VehicleActorDefinition : public ActorDefinition {
  VehicleActorDefinition(ActorDefinition const &actor_definition, rpc::VehiclePhysicsControl vehicle_physics_control_in)
    : ActorDefinition(actor_definition), vehicle_physics_control(vehicle_physics_control_in) {}

  rpc::VehiclePhysicsControl vehicle_physics_control;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla

namespace std {

inline std::string to_string(carla::ros2::types::VehicleActorDefinition const &actor_definition) {
  return "VehicleActor(" + to_string(static_cast<carla::ros2::types::ActorDefinition>(actor_definition)) + ")";
}

}  // namespace std
