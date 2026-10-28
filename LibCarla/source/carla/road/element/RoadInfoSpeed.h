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

  /// Converts an OpenDRIVE speed to km/h. @a unit is "m/s", "km/h" or "mph";
  /// when it is empty, @a default_unit is used instead.
  inline double SpeedToKmh(
      const double speed,
      const std::string &unit,
      const std::string &default_unit = "m/s") {
    const std::string &u = unit.empty() ? default_unit : unit;
    if (u == "km/h") {
      return speed;
    }
    if (u == "mph") {
      return speed * 1.609344;
    }
    return speed * 3.6;
  }

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
      return SpeedToKmh(_speed, _unit);
    }

  private:

    const double _speed;
    const std::string _type;

    const std::string _unit;
  };

} // namespace element
} // namespace road
} // namespace carla
