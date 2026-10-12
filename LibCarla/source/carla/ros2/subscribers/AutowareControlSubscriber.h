// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <string>

#include "carla/ros2/ROS2CallbackData.h"
#include "carla/ros2/subscribers/BaseSubscriber.h"

namespace carla {
namespace ros2 {

// Port of tier4's AutowareController: the six Autoware command subscribers,
// re-expressed through SubscriberImpl / ISubscriberMiddleware instead of
// tier4's AutowareSubscriber / SubscriberListenerBase Fast-DDS scaffolding
// (superseded).
//
// Topics (fixed absolute names, DDS "rt/" prefix, QoS RELIABLE /
// TRANSIENT_LOCAL / KEEP_LAST depth 1 per tier4):
//   /control/command/control_cmd         autoware_control_msgs/Control
//   /control/command/gear_cmd            autoware_vehicle_msgs/GearCommand
//   /control/command/turn_indicators_cmd autoware_vehicle_msgs/TurnIndicatorsCommand
//   /control/command/hazard_lights_cmd   autoware_vehicle_msgs/HazardLightsCommand
//   /control/command/emergency_cmd       tier4_vehicle_msgs/VehicleEmergencyStamped
//   /vehicle/engage                      autoware_vehicle_msgs/Engage
//
// Control application (tier4 AutowareController::GetControl): only
// longitudinal.acceleration drives the vehicle (velocity/jerk intentionally
// ignored — pure acceleration control); steer =
// GetSteeringInput(-steering_tire_angle) via the compensation LUT; steer_speed
// = -steering_tire_rotation_rate when defined, else 0. The remaining commands
// are drained each frame but not yet applied (tier4 TODO preserved for the
// Unreal phase).
class AutowareControlSubscriber : public BaseSubscriber {
public:
  AutowareControlSubscriber(void *vehicle, std::string frame_id);
  ~AutowareControlSubscriber() override;

  /// Drains the six command topics once per frame (called from
  /// ROS2::SetFrame like every BaseSubscriber). Fires the actor callback with
  /// a VehicleAccelerationControl when any command topic has news.
  void ProcessMessages(ActorCallback callback) override;

protected:
  ROS2CallbackData GetMessage() override;

private:
  class Impl;
  std::shared_ptr<Impl> _impl;
};

}  // namespace ros2
}  // namespace carla
