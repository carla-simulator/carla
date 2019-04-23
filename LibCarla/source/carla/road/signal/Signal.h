// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/road/RoadTypes.h"
#include "carla/road/general/Validity.h"
#include "carla/road/signal/SignalDependency.h"

#include <string>
#include <vector>

namespace carla {
namespace road {
namespace signal {

  class Signal : private MovableNonCopyable {
  public:

    Signal(
        road::RoadId road_id,
        road::SignId signal_id,
        double s,
        double t,
        std::string name,
        std::string dynamic,
        std::string orientation,
        double zOffset,
        std::string country,
        std::string type,
        std::string subtype,
        double value,
        std::string unit,
        double height,
        double width,
        std::string text,
        double hOffset,
        double pitch,
        double roll)
      : _road_id(road_id),
        _signal_id(signal_id),
        _s(s),
        _t(t),
        _name(name),
        _dynamic(dynamic),
        _orientation(orientation),
        _zOffset(zOffset),
        _country(country),
        _type(type),
        _subtype(subtype),
        _value(value),
        _unit(unit),
        _height(height),
        _width(width),
        _text(text),
        _hOffset(hOffset),
        _pitch(pitch),
        _roll(roll) {}

    void AddValidity(general::Validity &&validity) {
      _validities.push_back(std::move(validity));
    }

    void AddDependency(signal::SignalDependency &&dependency) {
      _dependencies.push_back(std::move(dependency));
    }

  private:

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-private-field"
#endif
    road::RoadId _road_id;
    road::SignId _signal_id;
    double _s;
    double _t;
    std::string _name;
    std::string _dynamic;
    std::string _orientation;
    double _zOffset;
    std::string _country;
    std::string _type;
    std::string _subtype;
    double _value;
    std::string _unit;
    double _height;
    double _width;
    std::string _text;
    double _hOffset;
    double _pitch;
    double _roll;
    std::vector<general::Validity> _validities;
    std::vector<signal::SignalDependency> _dependencies;
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif
  };

} // object
} // road
} // carla
