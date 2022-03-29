// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
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
        float in_damping_rate,
        float in_max_steer_angle,
        float in_radius,
        float in_max_brake_torque,
        float in_max_handbrake_torque,
        float in_lat_stiff_max_load,
        float in_lat_stiff_value,
        float in_long_stiff_value,
        geom::Vector3D in_position)
      : tire_friction(in_tire_friction),
        damping_rate(in_damping_rate),
        max_steer_angle(in_max_steer_angle),
        radius(in_radius),
        max_brake_torque(in_max_brake_torque),
        max_handbrake_torque(in_max_handbrake_torque),
        lat_stiff_max_load(in_lat_stiff_max_load),
        lat_stiff_value(in_lat_stiff_value),
        long_stiff_value(in_long_stiff_value),
        position(in_position) {}

    float tire_friction = 2.0f;
    float damping_rate = 0.25f;
    float max_steer_angle = 70.0f;
    float radius = 30.0f;
    float max_brake_torque = 1500.0f;
    float max_handbrake_torque = 3000.0f;
    float lat_stiff_max_load = 2.0f;
    float lat_stiff_value = 17.0f;
    float long_stiff_value = 1000.0f;
    geom::Vector3D position = {0.0f, 0.0f, 0.0f};

    bool operator!=(const WheelPhysicsControl &rhs) const {
      return
        tire_friction != rhs.tire_friction ||
        damping_rate != rhs.damping_rate ||
        max_steer_angle != rhs.max_steer_angle ||
        radius != rhs.radius ||
        max_brake_torque != rhs.max_brake_torque ||
        max_handbrake_torque != rhs.max_handbrake_torque ||
        lat_stiff_max_load != rhs.lat_stiff_max_load ||
        lat_stiff_value != rhs.lat_stiff_value ||
        long_stiff_value != rhs.long_stiff_value ||
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
        max_brake_torque(Wheel.MaxBrakeTorque),
        max_handbrake_torque(Wheel.MaxHandBrakeTorque),
        lat_stiff_max_load(Wheel.LatStiffMaxLoad),
        lat_stiff_value(Wheel.LatStiffValue),
        long_stiff_value(Wheel.LongStiffValue),
        position(Wheel.Position.X, Wheel.Position.Y, Wheel.Position.Z) {}

    operator FWheelPhysicsControl() const {
      FWheelPhysicsControl Wheel;
      Wheel.TireFriction = tire_friction;
      Wheel.DampingRate = damping_rate;
      Wheel.MaxSteerAngle = max_steer_angle;
      Wheel.Radius = radius;
      Wheel.MaxBrakeTorque = max_brake_torque;
      Wheel.MaxHandBrakeTorque = max_handbrake_torque;
      Wheel.LatStiffMaxLoad = lat_stiff_max_load;
      Wheel.LatStiffValue = lat_stiff_value;
      Wheel.LongStiffValue = long_stiff_value;
      Wheel.Position = {position.x, position.y, position.z};
      return Wheel;
    }
#endif

    MSGPACK_DEFINE_ARRAY(tire_friction,
        damping_rate,
        max_steer_angle,
        radius,
        max_brake_torque,
        max_handbrake_torque,
        lat_stiff_max_load,
        lat_stiff_value,
        long_stiff_value,
        position)
  };

}
}
