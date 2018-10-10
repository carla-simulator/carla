// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "RoadSegment.h"

#include <boost/graph/adjacency_list.hpp>

#include <map>

namespace carla {
namespace road {

  class Rect {
  public:

    geom::Location GetLocation() {
      return _location;
    }
    geom::Location GetExtend() {
      return _extend;
    }

  private:

    geom::Location _location;
    geom::Location _extend;
  };

  class RoadGraph {
  public:

    RoadGraph() : _bounding_box(),
                  _temp_sections(),
                  _sections() {}

    void AddNode(id_type id, const RoadSegmentDefinition &rs) {
      _temp_sections.emplace(std::pair<id_type, RoadSegmentDefinition>(id, rs));
    }

  private:

    Rect _bounding_box;
    std::multimap<id_type, RoadSegmentDefinition> _temp_sections;
    std::multimap<id_type, RoadSegment> _sections;
  };

  class Map {
  public:

    const RoadSegment &GetRoad(id_type id);

    const RoadSegment &NearestRoad(const geom::Location &loc);

    RoadGraph GetGraph();

    bool AddRoadSegment(const RoadSegmentDefinition &def);

  private:

    RoadGraph _graph;
  };

} // namespace road
} // namespace carla
