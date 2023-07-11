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
        _speed(speed),
        _type("Town") {}

    RoadInfoSpeed(double s, double speed, std::string& type)
      : RoadInfo(s),
        _speed(speed),
        _type(type) {}
    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    double GetSpeed() const {
      return _speed;
    }

    std::string GetType() const{
      return _type;
    }

  private:

    const double _speed;
    const std::string _type;
  };

} // namespace element
} // namespace road
} // namespace carla
