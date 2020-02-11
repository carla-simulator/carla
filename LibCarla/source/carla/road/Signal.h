// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/RoadTypes.h"
#include "carla/road/LaneValidity.h"

#include <string>
#include <vector>

namespace carla {
namespace road {

  struct SignalDependency {
  public:

    SignalDependency( std::string dependency_id, std::string type)
      : _dependency_id(dependency_id), _type(type) {}

    std::string _dependency_id;

    std::string _type;

  };

  class Signal : private MovableNonCopyable {
  public:
    Signal(
        SignId signal_id,
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
      : _signal_id(signal_id),
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

    void AddValidity(LaneValidity &&validity) {
      _validities.push_back(std::move(validity));
    }

    void AddDependency(SignalDependency &&dependency) {
      _dependencies.push_back(std::move(dependency));
    }

    SignId _signal_id;

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

    std::vector<LaneValidity> _validities;

    std::vector<SignalDependency> _dependencies;

  };

} // road
} // carla
