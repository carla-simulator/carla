// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

namespace carla {
namespace rpc {

  class WheelPhysicsControl {
  public:

    WheelPhysicsControl() = default;

    WheelPhysicsControl(
        float in_tire_friction,
        float in_damping_rate,
        float in_steer_angle,
        bool in_disable_steering)
      : tire_friction(in_tire_friction),
        damping_rate(in_damping_rate),
        steer_angle(in_steer_angle),
        disable_steering(in_disable_steering) {}

    float tire_friction = 2.0f;
    float damping_rate = 0.25f;
    float steer_angle = 70.0f;
    bool disable_steering = false;

    bool operator!=(const WheelPhysicsControl &rhs) const {
      return
        tire_friction != rhs.tire_friction ||
        damping_rate != rhs.damping_rate ||
        steer_angle != rhs.steer_angle ||
        disable_steering != rhs.disable_steering;
    }

    bool operator==(const WheelPhysicsControl &rhs) const {
      return !(*this != rhs);
    }
#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WheelPhysicsControl(const FWheelPhysicsControl &Wheel)
      : tire_friction(Wheel.TireFriction),
        damping_rate(Wheel.DampingRate),
        steer_angle(Wheel.SteerAngle),
        disable_steering(Wheel.bDisableSteering) {}

    operator FWheelPhysicsControl() const {
      FWheelPhysicsControl Wheel;
      Wheel.TireFriction = tire_friction;
      Wheel.DampingRate = damping_rate;
      Wheel.SteerAngle = steer_angle;
      Wheel.bDisableSteering = disable_steering;
      return Wheel;
    }
#endif

    MSGPACK_DEFINE_ARRAY(tire_friction,
        damping_rate,
        steer_angle,
        disable_steering)
  };

}
}
