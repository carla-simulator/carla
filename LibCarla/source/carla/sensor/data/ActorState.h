// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"

#include <cstdint>

namespace carla {
namespace sensor {
namespace data {

#pragma pack(push, 1)

  /// Dynamic state of an actor.
  struct ActorState {

    uint32_t id;

    geom::Transform transform;

    geom::Vector3D velocity;
  };

#pragma pack(pop)

  static_assert(sizeof(ActorState) == 10u * sizeof(uint32_t), "Invalid ActorState size!");

} // namespace data
} // namespace sensor
} // namespace carla
