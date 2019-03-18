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
#include "carla/geom/CubicPolynomial.h"

#include <unordered_map>
#include <vector>

namespace carla {
namespace road {

  class Road;
  class MapBuilder;

  class LaneSection : private MovableNonCopyable {
  public:

    LaneSection() = default;

    float GetDistance() const;

    Road *GetRoad();

    Lane *GetLane(const LaneId id);

    std::unordered_map<LaneId, Lane> &GetLanes();

  private:

    friend MapBuilder;

    Road *_road;

    float _s;

    std::unordered_map<LaneId, Lane> _lanes;

    geom::CubicPolynomial _lane_offset;

  };

} // road
} // carla
