// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
  #include <util/enable-ue4-macros.h>
  #include "Carla/Walker/WalkerControl.h"
  #include <util/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class WalkerControl {
  public:

    WalkerControl() = default;

    WalkerControl(
        geom::Vector3D in_direction,
        float in_speed,
        bool in_jump)
      : direction(in_direction),
        speed(in_speed),
        jump(in_jump) {}

    geom::Vector3D direction = {1.0f, 0.0f, 0.0f};

    float speed = 0.0f;

    bool jump = false;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WalkerControl(const FWalkerControl &Control)
      : direction(Control.Direction.X, Control.Direction.Y, Control.Direction.Z),
        speed(1e-2f * Control.Speed),
        jump(Control.Jump) {}

    operator FWalkerControl() const {
      FWalkerControl Control;
      Control.Direction = {direction.x, direction.y, direction.z};
      Control.Speed = 1e2f * speed;
      Control.Jump = jump;
      return Control;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    bool operator!=(const WalkerControl &rhs) const {
      return direction != rhs.direction || speed != rhs.speed || jump != rhs.jump;
    }

    bool operator==(const WalkerControl &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(direction, speed, jump);
  };

} // namespace rpc
} // namespace carla
