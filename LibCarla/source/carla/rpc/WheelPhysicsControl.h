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
        float in_max_steer_angle,
        bool in_is_steerable)
      : tire_friction(in_tire_friction),
        damping_rate(in_damping_rate),
        max_steer_angle(in_max_steer_angle),
        is_steerable(in_is_steerable) {}

    float tire_friction = 2.0f;
    float damping_rate = 0.25f;
    float max_steer_angle = 70.0f;
    bool is_steerable = true;

    bool operator!=(const WheelPhysicsControl &rhs) const {
      return
        tire_friction != rhs.tire_friction ||
        damping_rate != rhs.damping_rate ||
        max_steer_angle != rhs.max_steer_angle ||
        is_steerable != rhs.is_steerable;
    }

    bool operator==(const WheelPhysicsControl &rhs) const {
      return !(*this != rhs);
    }
#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WheelPhysicsControl(const FWheelPhysicsControl &Wheel)
      : tire_friction(Wheel.TireFriction),
        damping_rate(Wheel.DampingRate),
        max_steer_angle(Wheel.MaxSteerAngle),
        is_steerable(Wheel.IsSteerable) {}

    operator FWheelPhysicsControl() const {
      FWheelPhysicsControl Wheel;
      Wheel.TireFriction = tire_friction;
      Wheel.DampingRate = damping_rate;
      Wheel.MaxSteerAngle = max_steer_angle;
      Wheel.IsSteerable = is_steerable;
      return Wheel;
    }
#endif

    MSGPACK_DEFINE_ARRAY(tire_friction,
        damping_rate,
        max_steer_angle,
        is_steerable)
  };

}
}
