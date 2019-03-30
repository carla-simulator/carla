// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"
#include <string>

namespace carla {
namespace road {
namespace element {

  class RoadInfoMarkTypeLine final : public RoadInfo {
  public:

    RoadInfoMarkTypeLine(
        double s,
        int road_mark_id,
        double length,
        double space,
        double tOffset,
        std::string rule,
        double width)
      : RoadInfo(s),
        _road_mark_id(road_mark_id),
        _length(length),
        _space(space),
        _tOffset(tOffset),
        _rule(rule),
        _width(width) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    int GetRoadMarkId() const {
      return _road_mark_id;
    }

    double GetLength() const {
      return _length;
    }

    double GetSpace() const {
      return _space;
    }

    double GetTOffset() const {
      return _tOffset;
    }

    const std::string &GetRule() const {
      return _rule;
    }

    double GetWidth() const {
      return _width;
    }

  private:

    const int _road_mark_id;

    const double _length;

    const double _space;

    const double _tOffset;

    const std::string _rule;

    const double _width;
  };

} // namespace element
} // namespace road
} // namespace carla
