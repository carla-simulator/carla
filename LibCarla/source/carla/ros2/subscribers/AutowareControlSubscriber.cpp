// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/subscribers/AutowareControlSubscriber.h"

#include "carla/Logging.h"
#include "carla/ros2/AutowareSteeringCompensation.h"
#include "carla/ros2/middleware/QosProfile.h"
#include "carla/ros2/subscribers/SubscriberImpl.h"
#include "carla/ros2/types/msg/Control.h"
#include "carla/ros2/types/msg/Engage.h"
#include "carla/ros2/types/msg/GearCommand.h"
#include "carla/ros2/types/msg/HazardLightsCommand.h"
#include "carla/ros2/types/msg/TurnIndicatorsCommand.h"
#include "carla/ros2/types/msg/VehicleEmergencyStamped.h"

namespace carla {
namespace ros2 {

struct AutowareControlCmdTraits { using msg_type = msg::Control; };
struct AutowareGearCmdTraits { using msg_type = msg::GearCommand; };
struct AutowareTurnIndicatorsCmdTraits { using msg_type = msg::TurnIndicatorsCommand; };
struct AutowareHazardLightsCmdTraits { using msg_type = msg::HazardLightsCommand; };
struct AutowareEmergencyCmdTraits { using msg_type = msg::VehicleEmergencyStamped; };
struct AutowareEngageTraits { using msg_type = msg::Engage; };

class AutowareControlSubscriber::Impl {
public:
  SubscriberImpl<AutowareControlCmdTraits> control;
  SubscriberImpl<AutowareGearCmdTraits> gear;
  SubscriberImpl<AutowareTurnIndicatorsCmdTraits> turn_indicators;
  SubscriberImpl<AutowareHazardLightsCmdTraits> hazard_lights;
  SubscriberImpl<AutowareEmergencyCmdTraits> emergency;
  SubscriberImpl<AutowareEngageTraits> engage;

  bool HasNewMessage() {
    return control.HasNewMessage() ||
           gear.HasNewMessage() ||
           turn_indicators.HasNewMessage() ||
           hazard_lights.HasNewMessage() ||
           emergency.HasNewMessage() ||
           engage.HasNewMessage();
  }
};

AutowareControlSubscriber::AutowareControlSubscriber(
    void *vehicle, std::string frame_id)
  : BaseSubscriber(vehicle, "rt/control/command", std::move(frame_id)),
    _impl(std::make_shared<Impl>()) {
  // QoS per tier4's AutowareController: RELIABLE / TRANSIENT_LOCAL /
  // KEEP_LAST 1 so the latest latched command survives a late join.
  const QosProfile qos = QosProfile::ReliableTransientLocal();
  bool ok = true;
  ok &= _impl->control.Init("rt/control/command/control_cmd", qos);
  ok &= _impl->gear.Init("rt/control/command/gear_cmd", qos);
  ok &= _impl->turn_indicators.Init("rt/control/command/turn_indicators_cmd", qos);
  ok &= _impl->hazard_lights.Init("rt/control/command/hazard_lights_cmd", qos);
  ok &= _impl->emergency.Init("rt/control/command/emergency_cmd", qos);
  ok &= _impl->engage.Init("rt/vehicle/engage", qos);
  if (!ok) {
    log_error("AutowareControlSubscriber: failed to initialize one or more command readers");
  }
}

AutowareControlSubscriber::~AutowareControlSubscriber() = default;

ROS2CallbackData AutowareControlSubscriber::GetMessage() {
  // Drain every subscriber (clearing its new-message flag) even though only
  // control_cmd feeds the vehicle today — tier4 AutowareController::GetControl
  // does the same; gear/turn/hazard/emergency/engage application is a tier4
  // TODO deferred to the Unreal phase.
  const auto control_in = _impl->control.GetMessage();
  (void)_impl->gear.GetMessage();
  (void)_impl->turn_indicators.GetMessage();
  (void)_impl->hazard_lights.GetMessage();
  (void)_impl->emergency.GetMessage();
  (void)_impl->engage.GetMessage();

  VehicleAccelerationControl control_out{};

  // Longitudinal: use ONLY acceleration from /control/command/control_cmd to
  // drive the vehicle. Velocity and jerk from the message are intentionally
  // ignored to keep pure acceleration control.
  control_out.acceleration = control_in.longitudinal.acceleration;

  // Set lateral negative, because CARLA treats positive as right and Autoware
  // expects positive to represent left (all when moving forward).
  const float raw_steering = -control_in.lateral.steering_tire_angle;

  // Apply steering compensation lookup table (measured on Lincoln MKZ). This
  // reduces understeer / outward drift by mapping Autoware's target tire angle
  // to a vehicle input that better matches the requested actual tire angle.
  control_out.steer = autoware_steering_compensation::GetSteeringInput(raw_steering);
  control_out.steer_speed = 0.0f;
  if (control_in.lateral.is_defined_steering_tire_rotation_rate) {
    control_out.steer_speed = -control_in.lateral.steering_tire_rotation_rate;
  }

  return ROS2CallbackData(control_out);
}

void AutowareControlSubscriber::ProcessMessages(ActorCallback callback) {
  if (_impl->HasNewMessage()) {
    auto control = this->GetMessage();
    callback(this->GetActor(), control);
  }
}

}  // namespace ros2
}  // namespace carla
