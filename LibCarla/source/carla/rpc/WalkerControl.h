// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
  #include <compiler/enable-ue4-macros.h>
  #include "Carla/Walker/WalkerControl.h"
  #include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class WalkerControl {
  public:

    WalkerControl() = default;

    WalkerControl(
        geom::Vector3D in_direction,
        float in_speed,
        bool in_jump,
        float in_timestamp)
      : direction(in_direction),
        speed(in_speed),
        jump(in_jump),
        timestamp(in_timestamp) {}

    geom::Vector3D direction = {1.0f, 0.0f, 0.0f};

    float speed = 0.0f;

    bool jump = false;

    float timestamp = 0.f;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WalkerControl(const FWalkerControl &Control)
      : direction(Control.Direction.X, Control.Direction.Y, Control.Direction.Z),
        speed(1e-2f * Control.Speed),
        jump(Control.Jump),
        timestamp(Control.Timestamp) {}

    operator FWalkerControl() const {
      FWalkerControl Control;
      Control.Direction = {direction.x, direction.y, direction.z};
      Control.Speed = 1e2f * speed;
      Control.Jump = jump;
      Control.Timestamp = timestamp;
      return Control;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    bool operator!=(const WalkerControl &rhs) const {
      return direction != rhs.direction || speed != rhs.speed || jump != rhs.jump ||
          timestamp != rhs.timestamp;
    }

    bool operator==(const WalkerControl &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(direction, speed, jump, timestamp);
  };

} // namespace rpc
} // namespace carla
