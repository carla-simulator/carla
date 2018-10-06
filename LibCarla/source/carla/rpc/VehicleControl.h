// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

namespace carla {
namespace rpc {

  class VehicleControl {
  public:

    VehicleControl() = default;

    VehicleControl(
        float in_throttle,
        float in_steer,
        float in_brake,
        bool in_hand_brake,
        bool in_reverse)
      : throttle(in_throttle),
        steer(in_steer),
        brake(in_brake),
        hand_brake(in_hand_brake),
        reverse(in_reverse) {}

    float throttle = 0.0f;
    float steer = 0.0f;
    float brake = 0.0f;
    bool hand_brake = false;
    bool reverse = false;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    VehicleControl(const FVehicleControl &Control)
      : throttle(Control.Throttle),
        steer(Control.Steer),
        brake(Control.Brake),
        hand_brake(Control.bHandBrake),
        reverse(Control.bReverse) {}

    operator FVehicleControl() const {
      FVehicleControl Control;
      Control.Throttle = throttle;
      Control.Steer = steer;
      Control.Brake = brake;
      Control.bHandBrake = hand_brake;
      Control.bReverse = reverse;
      return Control;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    bool operator!=(const VehicleControl &rhs) const {
      return
          throttle != rhs.throttle ||
          steer != rhs.steer ||
          brake != rhs.brake ||
          hand_brake != rhs.hand_brake ||
          reverse != rhs.reverse;
    }

    bool operator==(const VehicleControl &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(
        throttle,
        steer,
        brake,
        hand_brake,
        reverse);
  };

} // namespace rpc
} // namespace carla
