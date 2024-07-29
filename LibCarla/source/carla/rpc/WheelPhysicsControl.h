// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"
#include "carla/MsgPack.h"

namespace carla {
namespace rpc {

  class WheelPhysicsControl {
  public:

    WheelPhysicsControl() = default;

    WheelPhysicsControl(
        float in_tire_friction,
        float in_max_steer_angle,
        float in_radius,
        float in_cornering_stiffness,
		    bool in_abs,
		    bool in_traction_control,
        float in_max_brake_torque,
        float in_max_handbrake_torque,
        geom::Vector3D in_position)
      : tire_friction(in_tire_friction),
        max_steer_angle(in_max_steer_angle),
        radius(in_radius),
        cornering_stiffness(in_cornering_stiffness),
        abs(in_abs),
        traction_control(in_traction_control),
        max_brake_torque(in_max_brake_torque),
        max_handbrake_torque(in_max_handbrake_torque),
        position(in_position) {}

    float tire_friction = 3.0f;
    float max_steer_angle = 70.0f;
    float radius = 30.0f;
    float cornering_stiffness = 1000.0f;
    bool abs = false;
    bool traction_control = false;
    float max_brake_torque = 1500.0f;
    float max_handbrake_torque = 3000.0f;
    geom::Vector3D position = {0.0f, 0.0f, 0.0f};

    bool operator!=(const WheelPhysicsControl &rhs) const {
      return
        tire_friction != rhs.tire_friction ||
        max_steer_angle != rhs.max_steer_angle ||
        radius != rhs.radius ||
        cornering_stiffness != rhs.cornering_stiffness ||
        abs != rhs.abs ||
        traction_control != rhs.traction_control ||
        max_brake_torque != rhs.max_brake_torque ||
        max_handbrake_torque != rhs.max_handbrake_torque ||
        position != rhs.position;
    }

    bool operator==(const WheelPhysicsControl &rhs) const {
      return !(*this != rhs);
    }
#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WheelPhysicsControl(const FWheelPhysicsControl &Wheel)
      : tire_friction(Wheel.FrictionForceMultiplier),
        max_steer_angle(Wheel.MaxSteerAngle),
        radius(Wheel.Radius),
        cornering_stiffness(Wheel.CorneringStiffness),
        abs(Wheel.bABSEnabled),
        traction_control(Wheel.bTractionControlEnabled),
        max_brake_torque(Wheel.MaxBrakeTorque),
        max_handbrake_torque(Wheel.MaxHandBrakeTorque),
        position(Wheel.Position.X, Wheel.Position.Y, Wheel.Position.Z) {}

    operator FWheelPhysicsControl() const {
      FWheelPhysicsControl Wheel;
      Wheel.FrictionForceMultiplier = tire_friction;
      Wheel.MaxSteerAngle = max_steer_angle;
      Wheel.Radius = radius;
      Wheel.CorneringStiffness = cornering_stiffness;
      Wheel.bABSEnabled = abs;
      Wheel.bTractionControlEnabled = traction_control;
      Wheel.MaxBrakeTorque = max_brake_torque;
      Wheel.MaxHandBrakeTorque = max_handbrake_torque;
      Wheel.Position = {position.x, position.y, position.z};
      return Wheel;
    }
#endif

    MSGPACK_DEFINE_ARRAY(tire_friction,
        max_steer_angle,
        radius,
        cornering_stiffness,
        abs,
        traction_control,
        max_brake_torque,
        max_handbrake_torque,
        position)
  };

}
}
