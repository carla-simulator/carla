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

  class RoadInfoSpeed final : public RoadInfo {
  public:

    RoadInfoSpeed(double s, double speed)
      : RoadInfo(s),
        _speed(speed) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    double GetSpeed() const {
      return _speed;
    }

  private:

    const double _speed;
  };

} // namespace element
} // namespace road
} // namespace carla
