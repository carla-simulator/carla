// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/Optional.h"
#include "carla/road/MapData.h"
#include "carla/road/element/LaneMarking.h"
#include "carla/road/element/Waypoint.h"

#include <vector>

namespace carla {
namespace road {

  class Map
    : public EnableSharedFromThis<Map>,
      private MovableNonCopyable {

  public:

    Map(MapData m)
      : _data(std::move(m)) {}

    element::Waypoint GetClosestWaypointOnRoad(const geom::Location &) const;

    Optional<element::Waypoint> GetWaypoint(const geom::Location &) const;

    std::vector<element::LaneMarking> CalculateCrossedLanes(
        const geom::Location &origin,
        const geom::Location &destination) const;

    const MapData &GetData() const;

  private:

    MapData _data;
  };

} // namespace road
} // namespace carla
