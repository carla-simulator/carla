// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "ackermann_msgs/msg/AckermannDriveStamped.h"
#include "carla/ros2/types/Timestamp.h"
#include "carla/rpc/VehicleAckermannControl.h"
#include "carla/sensor/data/ActorDynamicState.h"

namespace carla {
namespace ros2 {
namespace types {

/**
 * VehicleAckermannControl: convert ackermann_msgs::msg::AckermannDriveStamped into FVehicleAckermannControl without the
 * need of knowing FVehicleAckermannControl class within LibCarla
 */
class VehicleAckermannControl {
public:
  explicit VehicleAckermannControl(const ackermann_msgs::msg::AckermannDriveStamped& vehicle_ackermann_control)
    : _vehicle_ackermann_control(vehicle_ackermann_control) {}
  ~VehicleAckermannControl() = default;
  VehicleAckermannControl(const VehicleAckermannControl&) = default;
  VehicleAckermannControl& operator=(const VehicleAckermannControl&) = default;
  VehicleAckermannControl(VehicleAckermannControl&&) = default;
  VehicleAckermannControl& operator=(VehicleAckermannControl&&) = default;

  ackermann_msgs::msg::AckermannDriveStamped const& carla_vehicle_ackermann_control() const {
    return _vehicle_ackermann_control;
  }

  VehicleAckermannControl(const carla::rpc::VehicleAckermannControl& vehicle_ackermann_control) {
    _vehicle_ackermann_control.header().stamp(Timestamp(vehicle_ackermann_control.timestamp).time());
    _vehicle_ackermann_control.drive().steering_angle(vehicle_ackermann_control.steer);
    _vehicle_ackermann_control.drive().steering_angle_velocity(vehicle_ackermann_control.steer_speed);
    _vehicle_ackermann_control.drive().speed(vehicle_ackermann_control.speed);
    _vehicle_ackermann_control.drive().acceleration(vehicle_ackermann_control.acceleration);
    _vehicle_ackermann_control.drive().jerk(vehicle_ackermann_control.jerk);
  }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

  FVehicleAckermannControl GetVehicleAckermannControl() const {
    FVehicleAckermannControl vehicle_ackermann_control;
    vehicle_ackermann_control.Timestamp = Timestamp(_vehicle_ackermann_control.header().stamp()).Stamp();
    vehicle_ackermann_control.Steer = _vehicle_ackermann_control.drive().steering_angle();
    vehicle_ackermann_control.SteerSpeed = _vehicle_ackermann_control.drive().steering_angle_velocity();
    vehicle_ackermann_control.Speed = _vehicle_ackermann_control.drive().speed();
    vehicle_ackermann_control.Acceleration = _vehicle_ackermann_control.drive().acceleration();
    vehicle_ackermann_control.Jerk = _vehicle_ackermann_control.drive().jerk();
    return vehicle_ackermann_control;
  }
#endif  // LIBCARLA_INCLUDED_FROM_UE4
private:
  ackermann_msgs::msg::AckermannDriveStamped _vehicle_ackermann_control;
};
}  // namespace types
}  // namespace ros2
}  // namespace carla