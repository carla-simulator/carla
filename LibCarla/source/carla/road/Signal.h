// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/RoadTypes.h"
#include "carla/road/LaneValidity.h"
#include "carla/geom/Transform.h"

#include <string>
#include <vector>

namespace carla {
namespace road {

  enum SignalOrientation {
    Positive,
    Negative,
    Both
  };

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
        RoadId road_id,
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

    RoadId GetRoadId() const {
      return _road_id;
    }

    const SignId &GetSignalId() const {
      return _signal_id;
    }

    double GetS() const {
      return _s;
    }

    double GetT() const {
      return _t;
    }

    bool GetDynamic() const {
      if(_dynamic == "yes") {
        return true;
      }else {
        return false;
      }
    }

    const std::string &GetName() const {
      return _name;
    }

    SignalOrientation GetOrientation() const {
      if(_orientation == "+") {
        return SignalOrientation::Positive;
      } else if(_orientation == "-") {
        return SignalOrientation::Negative;
      } else {
        return SignalOrientation::Both;
      }
    }

    double GetZOffset() const {
      return _zOffset;
    }

    const std::string &GetCountry() const {
      return _country;
    }

    const std::string &GetType() const {
      return _type;
    }

    const std::string &GetSubtype() const {
      return _subtype;
    }

    double GetValue() const {
      return _value;
    }

    const std::string &GetUnit() const {
      return _unit;
    }

    double GetHeight() const {
      return _height;
    }

    double GetWidth() const {
      return _width;
    }

    const std::string &GetText() const {
      return _text;
    }

    double GetHOffset() const {
      return _hOffset;
    }

    double GetPitch() const {
      return _pitch;
    }

    double GetRoll() const {
      return _roll;
    }

    const std::vector<SignalDependency> &
        GetDependencies() {
      return _dependencies;
    }

    const geom::Transform &GetTransform() const {
      return _transform;
    }

    const std::set<ContId>& GetControllers() const {
      return _controllers;
    }

  private:
    friend MapBuilder;

    RoadId _road_id;

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

    std::vector<SignalDependency> _dependencies;

    geom::Transform _transform;

    std::set<ContId> _controllers;

    bool _using_inertial_position = false;
  };

} // road
} // carla
