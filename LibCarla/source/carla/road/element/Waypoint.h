// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h"
#include "carla/Memory.h"
#include "carla/road/element/RoadInfoList.h"
#include "carla/road/element/Types.h"

namespace carla {
namespace road {

  class Map;
  class WaypointGenerator;

namespace element {

  class Waypoint {
  public:

    ~Waypoint();

    geom::Transform ComputeTransform() const;

    id_type GetRoadId() const {
      return _road_id;
    }

    int GetLaneId() const {
      return _lane_id;
    }

    RoadInfoList GetRoadInfo() const;

  private:

    friend carla::road::Map;
    friend carla::road::WaypointGenerator;

    Waypoint(SharedPtr<const Map>, const geom::Location &location);

    Waypoint(
        SharedPtr<const Map> map,
        id_type road_id,
        id_type lane_id,
        double distance);

    SharedPtr<const Map> _map;

    id_type _road_id = 0;

    int _lane_id = 0;

    double _dist = 0.0;
  };

} // namespace element
} // namespace road
} // namespace carla
