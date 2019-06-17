// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"
#include "carla/rpc/ActorId.h"
#include "carla/sensor/data/ActorDynamicState.h"

namespace carla {
namespace client {

  struct ActorSnapshot {
    ActorId id = 0u;
    geom::Transform transform;
    geom::Vector3D velocity;
    geom::Vector3D angular_velocity;
    geom::Vector3D acceleration;
    sensor::data::ActorDynamicState::TypeDependentState state;
  };

} // namespace client
} // namespace carla
