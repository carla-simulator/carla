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

  class RoadInfoVelocity : public RoadInfo {

  public:

    void AcceptVisitor(RoadInfoVisitor &v) override final {
      v.Visit(*this);
    }

    RoadInfoVelocity(
      float s,
      float vel,
      std::string unit)
      : RoadInfo(s),
        _velocity(vel),
        _unit(unit) {}

    float GetVelocity() const {
      return _velocity;
    }

    std::string GetUnit() const {
      return _unit;
    }

  private:

    float _velocity;

    std::string _unit;
  };

} // namespace element
} // namespace road
} // namespace carla
