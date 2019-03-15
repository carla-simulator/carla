// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include <string>
#include <vector>
#include "carla/road/RoadTypes.h"

namespace carla {
namespace road {

  class Object : private MovableNonCopyable {
  public:

    Object() {}

  private:

    ObjId _id;
    std::string _type;
    std::string _name;
    float _s;
    float _t;
    float _zOffset;
    float _validLength;
    std::string _orientation;
    float _lenght;
    float _width;
    float _hdg;
    float _pitch;
    float _roll;

  };

} // road
} // carla
