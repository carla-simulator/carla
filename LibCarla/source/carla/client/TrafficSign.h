// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/road/RoadTypes.h"

namespace carla {
namespace client {

  class TrafficSign : public Actor {
  public:

    explicit TrafficSign(ActorInitializer init) : Actor(std::move(init)) {}

    const geom::BoundingBox &GetTriggerVolume() const {
      return ActorState::GetBoundingBox();
    }

    carla::road::SignId GetSignId() const;

  };

} // namespace client
} // namespace carla
