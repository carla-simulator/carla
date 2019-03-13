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

  class RoadInfoVelocity : public RoadInfo {

  public:

    void AcceptVisitor(RoadInfoVisitor &v) override final {
      v.Visit(*this);
    }

    RoadInfoVelocity(double vel) : velocity(vel) {}
    RoadInfoVelocity(double d, double vel)
      : RoadInfo(d),
        velocity(vel) {}

    double GetVelocity() {
      return velocity;
    }

  private:

    double velocity;
  };

} // namespace element
} // namespace road
} // namespace carla
