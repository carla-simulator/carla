// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Vehicle/VehicleAckermannControl.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class VehicleAckermannControl {
  public:

    VehicleAckermannControl() = default;

    VehicleAckermannControl(
        float in_steer,
        float in_steer_speed,
        float in_speed,
        float in_acceleration,
        float in_jerk,
        float in_timestamp)
      : steer(in_steer),
        steer_speed(in_steer_speed),
        speed(in_speed),
        acceleration(in_acceleration),
        jerk(in_jerk),
        timestamp(in_timestamp) {}

    float steer = 0.0f;
    float steer_speed = 0.0f;
    float speed = 0.0f;
    float acceleration = 0.0f;
    float jerk = 0.0f;
    float timestamp = 0.f;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    VehicleAckermannControl(const FVehicleAckermannControl &Control)
      : steer(Control.Steer),
        steer_speed(Control.SteerSpeed),
        speed(Control.Speed),
        acceleration(Control.Acceleration),
        jerk(Control.Jerk),
        timestamp(Control.Timestamp) {}

    operator FVehicleAckermannControl() const {
      FVehicleAckermannControl Control;
      Control.Steer = steer;
      Control.SteerSpeed = steer_speed;
      Control.Speed = speed;
      Control.Acceleration = acceleration;
      Control.Jerk = jerk;
      Control.Timestamp = timestamp;
      return Control;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    bool operator!=(const VehicleAckermannControl &rhs) const {
      return
          steer != rhs.steer ||
          steer_speed != rhs.steer_speed ||
          speed != rhs.speed ||
          acceleration != rhs.acceleration ||
          jerk != rhs.jerk ||
          timestamp != rhs.timestamp;
    }

    bool operator==(const VehicleAckermannControl &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(
        steer,
        steer_speed,
        speed,
        acceleration,
        jerk,
        timestamp);
  };

} // namespace rpc
} // namespace carla
