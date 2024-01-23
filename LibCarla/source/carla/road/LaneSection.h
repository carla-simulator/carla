// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/CubicPolynomial.h"
#include "carla/NonCopyable.h"
#include "carla/road/Lane.h"
#include "carla/road/RoadElementSet.h"
#include "carla/road/RoadTypes.h"

#include <map>
#include <vector>

namespace carla {
namespace road {

  class Road;
  class MapBuilder;

  class LaneSection : private MovableNonCopyable {
  public:

    explicit LaneSection(SectionId id, double s) : _id(id), _s(s) {}

    double GetDistance() const;

    double GetLength() const;

    Road *GetRoad() const;

    Lane *GetLane(const LaneId id);

    const Lane *GetLane(const LaneId id) const;

    bool ContainsLane(LaneId id) const {
      return (_lanes.find(id) != _lanes.end());
    }

    SectionId GetId() const;

    std::map<LaneId, Lane> &GetLanes();

    const std::map<LaneId, Lane> &GetLanes() const;

    std::vector<Lane *> GetLanesOfType(Lane::LaneType type);

  private:

    friend MapBuilder;

    const SectionId _id = 0u;

    const double _s = 0.0;

    Road *_road = nullptr;

    std::map<LaneId, Lane> _lanes;

    geom::CubicPolynomial _lane_offset;
  };

} // road
} // carla
