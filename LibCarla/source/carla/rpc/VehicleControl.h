// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/MsgPack.h"

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

    MSGPACK_DEFINE_ARRAY(
        throttle,
        steer,
        brake,
        hand_brake,
        reverse);
  };

} // namespace rpc
} // namespace carla
