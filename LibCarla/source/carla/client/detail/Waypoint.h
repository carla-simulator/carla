// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/geom/Location.h"

namespace carla {
namespace client {
  class Map;
namespace detail {

  struct Info {
    size_t _id;
    double _speed_limit;
    double _width;
    double _heading;
  };

  class Waypoint {
  public:

    Waypoint()
      : _pos(),
        _heading(0.0) {}

    Waypoint(const geom::Location &p)
      : _pos(p),
        _heading(0.0) {}

    Waypoint(const geom::Location &p, double heading)
      : _pos(p),
        _heading(heading) {}

    ~Waypoint();

    std::vector<Waypoint> NextWaypoints(double dist);

    const Info &GetInfo() {
      return _info;
    }

  private:

    geom::Location _pos;

    double _heading;

    Info _info;

    SharedPtr<Map> _map;

  };

} // namespace detail
} // namespace client
} // namespace carla
