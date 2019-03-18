// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"

namespace carla {
namespace road {
namespace element {

  class RoadInfoSpeed : public RoadInfo {

  public:

    void AcceptVisitor(RoadInfoVisitor &v) override final {
      v.Visit(*this);
    }

    RoadInfoSpeed(float speed) : _speed(speed) {}
    RoadInfoSpeed(float s, float speed)
      : RoadInfo(s),
        _speed(speed) {}

    float GetSpeed() {
      return _speed;
    }

  private:

    float _speed;
  };

} // namespace element
} // namespace road
} // namespace carla
