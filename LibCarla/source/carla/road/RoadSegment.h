// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Types.h"
#include "carla/geom/Location.h"

#include <vector>
#include <map>

namespace carla {
namespace road {

  struct Direction {
    double angle = 0.0; // [radians]
  };

  class RoadElement {
  public:

    geom::Location GetLocation() const {
      return _loc;
    }
    Direction GetDirection() const {
      return _dir;
    }

  private:

    geom::Location _loc;
    Direction _dir;
  };

  class RoadSegment : public RoadElement {
  public:

    id_type GetId() {
      return _id;
    }
    std::vector<RoadInfo> GetInfo();

    std::vector<RoadInfo> GetInfo(double dist);

    // returns single info given a type and a distance
    template <typename T>
    T GetInfo(double dist);

    // returns info vector given a type and a distance
    template <typename T>
    std::vector<T> GetInfo(double dist);

  private:

    id_type _id = -1;
    std::vector<RoadSegment *> _next_list;
    std::vector<RoadSegment *> _prev_list;
    std::vector<Geometry> geom;
    std::multimap<double, RoadInfo> _info_list;
  };

} // namespace road
} // namespace carla
