// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include <string>
#include <vector>
#include "carla/road/RoadTypes.h"
#include "carla/road/general/Validity.h"

namespace carla {
namespace road {
namespace signal {

  class Signal : private MovableNonCopyable {
  public:

    Signal(
        int32_t road_id,
        int32_t signal_id,
        float s,
        float t,
        std::string name,
        std::string dynamic,
        std::string orientation,
        float zOffset,
        std::string country,
        std::string type,
        std::string subtype,
        float value,
        std::string unit,
        float height,
        float width,
        std::string text,
        float hOffset,
        float pitch,
        float roll)
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
          _roll(roll) {
            _validities = std::vector<general::Validity>();
          }

    void AddValidity(general::Validity &&validity);

  private:

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-private-field"
#endif
    int32_t _road_id;
    int32_t _signal_id;
    float _s;
    float _t;
    std::string _name;
    std::string _dynamic;
    std::string _orientation;
    float _zOffset;
    std::string _country;
    std::string _type;
    std::string _subtype;
    float _value;
    std::string _unit;
    float _height;
    float _width;
    std::string _text;
    float _hOffset;
    float _pitch;
    float _roll;
    std::vector<general::Validity> _validities;
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif
  };



} // object
} // road
} // carla
