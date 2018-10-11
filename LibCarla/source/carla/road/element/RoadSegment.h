// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Types.h"
#include "RoadElement.h"
#include "carla/geom/Location.h"

#include <map>
#include <memory>
#include <vector>

namespace carla {
namespace road {
namespace element {

  class RoadSegment : public RoadElement {
  public:

    RoadSegment() : _id(-1) {}
    RoadSegment(const RoadSegmentDefinition &def) : _id(def.GetId()) {}

    id_type GetId() const {
      return _id;
    }
    const std::vector<RoadInfo> GetInfo() const;

    const std::vector<RoadInfo> GetInfo(double dist) const;

    // returns single info given a type and a distance
    template <typename T>
    T GetInfo(double dist);

    // returns info vector given a type and a distance
    template <typename T>
    std::vector<T> GetInfo(double dist);

  private:

    id_type _id;
    std::vector<RoadSegment *> _next_list;
    std::vector<RoadSegment *> _prev_list;
    std::vector<std::unique_ptr<Geometry>> geom;
    std::multimap<double, std::unique_ptr<RoadInfo>> _info_list;
  };

} // namespace element
} // namespace road
} // namespace carla
