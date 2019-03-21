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
        float s,
        int road_mark_id,
        float length,
        float space,
        float tOffset,
        std::string rule,
        float width)
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

    float GetLength() const {
      return _length;
    }

    float GetSpace() const {
      return _space;
    }

    float GetTOffset() const {
      return _tOffset;
    }

    const std::string& GetRule() const {
      return _rule;
    }

    float GetWidth() const {
      return _width;
    }

  private:

    const int _road_mark_id;

    const float _length;

    const float _space;

    const float _tOffset;

    const std::string _rule;

    const float _width;
  };

} // namespace element
} // namespace road
} // namespace carla
