// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/road/RoadTypes.h"

#include <string>

namespace carla {
namespace road {

  class Object : private MovableNonCopyable {
  public:

    Object() = default;

  private:

    ObjId _id = 0u;
    std::string _type;
    std::string _name;
    double _s = 0.0;
    double _t = 0.0;
    double _zOffset = 0.0;
    double _validLength = 0.0;
    std::string _orientation;
    double _lenght = 0.0;
    double _width = 0.0;
    double _hdg = 0.0;
    double _pitch = 0.0;
    double _roll = 0.0;
  };

} // road
} // carla
