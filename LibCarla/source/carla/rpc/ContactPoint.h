// Copyright (c) 2025 AVL List GmbH
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/rpc/Location.h"
#include "carla/rpc/Vector3D.h"

namespace carla {
namespace rpc {

  struct ContactPoint {

    ContactPoint () {}
    ContactPoint(Location location, Vector3D normal, float friction)
     : _location(location), _normal(normal), _friction(friction)
     {}

    Location _location;
    Vector3D _normal;
    float _friction = -1.0f;

    MSGPACK_DEFINE_ARRAY(_location, _normal, _friction);

  };

}
}
