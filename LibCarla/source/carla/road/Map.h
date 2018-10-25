// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Math.h"
#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/Optional.h"
#include "carla/road/element/Waypoint.h"
#include "carla/road/MapData.h"

namespace carla {
namespace road {

  class Map
    : public EnableSharedFromThis<Map>,
      private MovableNonCopyable {
  public:

    element::Waypoint GetClosestWaypointOnRoad(const geom::Location &) const {
      return element::Waypoint();
    }

    Optional<element::Waypoint> GetWaypoint(const geom::Location &) const {
      return Optional<element::Waypoint>();
    }

    const MapData &GetData() {
      return _data;
    }

    Map(MapData m)
      : _data(std::move(m)) {}

  private:

    MapData _data;

  };

} // namespace road
} // namespace carla
