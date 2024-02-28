// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/types/Timestamp.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/sensor/data/ActorDynamicState.h"
#include "carla_msgs/msg/CarlaVehicleControl.h"

namespace carla {
namespace ros2 {
namespace types {

/**
 * VehicleControl: convert carla_msgs::msg::CarlaVehicleControl into FVehicleControl without the need of
 * knowing FVehicleControl class within LibCarla
 */
class VehicleControl {
public:
  explicit VehicleControl(const carla_msgs::msg::CarlaVehicleControl& vehicle_control)
    : _vehicle_control(vehicle_control) {}
  ~VehicleControl() = default;
  VehicleControl(const VehicleControl&) = default;
  VehicleControl& operator=(const VehicleControl&) = default;
  VehicleControl(VehicleControl&&) = default;
  VehicleControl& operator=(VehicleControl&&) = default;

  carla_msgs::msg::CarlaVehicleControl const& carla_vehicle_control() const {
    return _vehicle_control;
  }

  VehicleControl(const carla::rpc::VehicleControl& vehicle_control, uint8_t control_priority = 0) {
    _vehicle_control.header().stamp(Timestamp(vehicle_control.timestamp).time());
    _vehicle_control.throttle(vehicle_control.throttle);
    _vehicle_control.steer(vehicle_control.steer);
    _vehicle_control.brake(vehicle_control.brake);
    _vehicle_control.hand_brake(vehicle_control.hand_brake);
    _vehicle_control.reverse(vehicle_control.reverse);
    _vehicle_control.gear(vehicle_control.gear);
    _vehicle_control.manual_gear_shift(vehicle_control.manual_gear_shift);
    _vehicle_control.control_priority(control_priority);
  }

#ifdef LIBCARLA_INCLUDED_FROM_UE4
  FVehicleControl GetVehicleControl() const {
    FVehicleControl vehicle_control;
    vehicle_control.Timestamp = Timestamp(_vehicle_control.header().stamp()).Stamp();
    vehicle_control.Throttle = _vehicle_control.throttle();
    vehicle_control.Steer = _vehicle_control.steer();
    vehicle_control.Brake = _vehicle_control.brake();
    vehicle_control.bHandBrake = _vehicle_control.hand_brake();
    vehicle_control.bReverse = _vehicle_control.reverse();
    vehicle_control.bManualGearShift = _vehicle_control.manual_gear_shift();
    vehicle_control.Gear = _vehicle_control.gear();
    return vehicle_control;
  }

  EVehicleInputPriority ControlPriority() const {
    return EVehicleInputPriority(_vehicle_control.control_priority());
  }

#endif  // LIBCARLA_INCLUDED_FROM_UE4
private:
  carla_msgs::msg::CarlaVehicleControl _vehicle_control;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla