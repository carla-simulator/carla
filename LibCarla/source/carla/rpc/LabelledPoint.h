// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/rpc/Location.h"
#include "carla/rpc/ObjectLabel.h"

namespace carla {
namespace rpc {

  struct LabelledPoint {

    LabelledPoint () {}
    LabelledPoint (Location location, CityObjectLabel label)
     : _location(location), _label(label)
     {}

    Location _location;

    CityObjectLabel _label;

    MSGPACK_DEFINE_ARRAY(_location, _label);

  };

}
}
