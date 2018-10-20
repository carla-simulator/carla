// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/detail/Waypoint.h"

namespace carla {
namespace client {
namespace detail {

  Waypoint::~Waypoint() = default; // to ensure we delete _map

  // @todo
  /*std::vector<Waypoint> Waypoint::NextWaypoints(double dist) {
    std::vector<Waypoint> v;

    return v;
  }*/

} // namespace detail
} // namespace client
} // namespace carla