// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/road/Lane.h"
#include "carla/road/RoadElementSet.h"
#include "carla/road/RoadTypes.h"

#include <unordered_map>
#include <vector>

namespace carla {
namespace road {

  class Road;

  class LaneSection : private MovableNonCopyable {
  public:

    LaneSection() = default;

    LaneSection(Road *road, const float s, std::vector<Lane> &&lanes)
      : _road(road),
        _s(s) {
      for (auto &lane : lanes) {
        const auto id = lane.GetId();
        _lanes.emplace(id, std::move(lane));
      }
    }

    float GetDistance() const {
      return _s;
    }

    const Road *GetRoad() const {
      return _road;
    }

  private:

    Road *_road;

    float _s;

    std::unordered_map<LaneId, Lane> _lanes;

  };

} // road
} // carla
