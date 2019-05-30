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
        float in_radius,
        geom::Vector3D in_position)
      : tire_friction(in_tire_friction),
        damping_rate(in_damping_rate),
        max_steer_angle(in_max_steer_angle),
        radius(in_radius),
        position(in_position) {}

    float tire_friction = 2.0f;
    float damping_rate = 0.25f;
    float max_steer_angle = 70.0f;
    float radius = 30.0f;
    geom::Vector3D position = {0.0f, 0.0f, 0.0f};

    bool operator!=(const WheelPhysicsControl &rhs) const {
      return
        tire_friction != rhs.tire_friction ||
        damping_rate != rhs.damping_rate ||
        max_steer_angle != rhs.max_steer_angle ||
        radius != rhs.radius ||
        position != rhs.position;
    }

    bool operator==(const WheelPhysicsControl &rhs) const {
      return !(*this != rhs);
    }
#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WheelPhysicsControl(const FWheelPhysicsControl &Wheel)
      : tire_friction(Wheel.TireFriction),
        damping_rate(Wheel.DampingRate),
        max_steer_angle(Wheel.MaxSteerAngle),
        radius(Wheel.Radius),
        position(Wheel.Position.X, Wheel.Position.Y, Wheel.Position.Z) {}

    operator FWheelPhysicsControl() const {
      FWheelPhysicsControl Wheel;
      Wheel.TireFriction = tire_friction;
      Wheel.DampingRate = damping_rate;
      Wheel.MaxSteerAngle = max_steer_angle;
      Wheel.Radius = radius;
      Wheel.Position = {position.x, position.y, position.z};
      return Wheel;
    }
#endif

    MSGPACK_DEFINE_ARRAY(tire_friction,
        damping_rate,
        max_steer_angle,
        radius,
        position)
  };

}
}
