// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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

    RoadInfoSpeed(double s, double speed, std::string type, std::string unit)
      : RoadInfo(s),
        _speed(speed),
        _type(std::move(type)),
        _unit(std::move(unit)) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    /// The max speed as written in the OpenDRIVE file, in GetUnit() units.
    double GetSpeed() const {
      return _speed;
    }

    std::string GetType() const{
      return _type;
    }

    /// The OpenDRIVE unit attribute ("m/s", "km/h" or "mph"); empty when the
    /// file left it out, which OpenDRIVE defines as m/s.
    const std::string &GetUnit() const {
      return _unit;
    }

    /// The max speed converted to km/h.
    double GetSpeedKmh() const {
      if (_unit == "km/h") {
        return _speed;
      }
      if (_unit == "mph") {
        return _speed * 1.609344;
      }
      return _speed * 3.6;
    }

  private:

    const double _speed;
    const std::string _type;

    const std::string _unit;
  };

} // namespace element
} // namespace road
} // namespace carla
