// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Types.h"
#include "RoadElement.h"
#include "carla/geom/Location.h"

#include <set>
#include <memory>
#include <vector>

namespace carla {
namespace road {
namespace element {

  class RoadSegment : public RoadElement {
  public:

    RoadSegment(RoadSegmentDefinition &&def)
      : _id(def.GetId()),
        _geom(std::move(def._geom)) {
      for (auto &&a : def._info) {
        _info.insert(std::move(a));
      }
    }

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

    struct LessComp {
      using is_transparent = void;
      bool operator()(
          const std::unique_ptr<RoadInfo> &a,
          const std::unique_ptr<RoadInfo> &b) const {
        return a->d < b->d;
      }
    };

    id_type _id;
    std::vector<RoadSegment *> _predecessors;
    std::vector<RoadSegment *> _successors;
    std::vector<std::unique_ptr<Geometry>> _geom;
    std::multiset<std::unique_ptr<RoadInfo>, LessComp> _info;
  };

} // namespace element
} // namespace road
} // namespace carla
