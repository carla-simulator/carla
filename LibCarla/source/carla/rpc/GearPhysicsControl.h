// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Vehicle/VehiclePhysicsControl.h"
#include <compiler/disable-ue4-macros.h>
#endif

namespace carla {
namespace rpc {

  class GearPhysicsControl {
  public:

    GearPhysicsControl() = default;

    GearPhysicsControl(
    float in_ratio,
    float in_down_ratio,
    float in_up_ratio)
      : ratio(in_ratio),
        down_ratio(in_down_ratio),
        up_ratio(in_up_ratio) {}

    float ratio = 1.0f;
    float down_ratio = 0.5f;
    float up_ratio = 0.65f;

    bool operator!=(const GearPhysicsControl &rhs) const {
      return
      ratio != rhs.ratio ||
      down_ratio != rhs.down_ratio ||
      up_ratio != rhs.up_ratio;
    }

    bool operator==(const GearPhysicsControl &rhs) const {
      return !(*this != rhs);
    }
#ifdef LIBCARLA_INCLUDED_FROM_UE4

    GearPhysicsControl(const FGearPhysicsControl &Gear)
      : ratio(Gear.Ratio),
        down_ratio(Gear.DownRatio),
        up_ratio(Gear.UpRatio) {}

    operator FGearPhysicsControl() const {
      FGearPhysicsControl Gear;
      Gear.Ratio = ratio;
      Gear.DownRatio = down_ratio;
      Gear.UpRatio = up_ratio;
      return Gear;
    }
#endif

    MSGPACK_DEFINE_ARRAY(ratio,
    down_ratio,
    up_ratio)
  };

}
}
