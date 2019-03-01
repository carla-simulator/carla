// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"

namespace carla {
namespace client {

  class TrafficSign : public Actor {
  public:

    explicit TrafficSign(ActorInitializer init) : Actor(std::move(init)) {}

    const geom::BoundingBox &GetTriggerVolume() const {
      return ActorState::GetBoundingBox();
    }
  };

} // namespace client
} // namespace carla
