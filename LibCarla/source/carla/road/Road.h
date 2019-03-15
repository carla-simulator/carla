// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/road/InformationSet.h"
#include "carla/road/Junction.h"
#include "carla/road/LaneSection.h"
#include "carla/road/RoadTypes.h"
#include "carla/road/RoadElementSet.h"
#include "carla/road/signal/Signal.h"

#include <vector>
#include <memory>

namespace carla {
namespace road {

  class MapData;
  class Elevation;

  class Road : private MovableNonCopyable {
  public:

    Road() = default;

    Road(
        MapData *map_data,
        const RoadId id,
        const bool is_junction,
        const JuncId junction_id,
        std::vector<LaneSection> &&lane_sections,
        std::vector<std::unique_ptr<element::RoadInfo>> &&info)
      : _map_data(map_data),
        _id(id),
        _is_junction(is_junction),
        _junction_id(junction_id),
        _lane_sections(std::move(lane_sections)),
        _info(std::move(info)) {}

    const MapData *GetMap() const;

    RoadId GetId() const;

    bool IsJunction() const;

    JuncId GetJunction() const;

    const Lane *GetLane(const LaneId id, const float s) const;

    std::vector<const Road *> GetNexts() const;

    std::vector<const Road *> GetPrevs() const;

    signal::Signal GetSignal(const SignId id) const;

    std::unordered_map<SignId, signal::Signal> getSignals() const;

  private:

    MapData *_map_data;

    RoadId _id;

    bool _is_junction;

    JuncId _junction_id;

    RoadElementSet<LaneSection> _lane_sections;

    InformationSet _info;

    std::vector<RoadId> _nexts;

    std::vector<RoadId> _prevs;

    std::unordered_map<SignId, signal::Signal> _signals;

  };

} // road
} // carla
