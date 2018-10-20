// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/client/detail/Waypoint.h"

#include <string>

namespace carla {
namespace road {
  class Map;
}
namespace client {

  class Map
    : public EnableSharedFromThis<Map>,
      private NonCopyable {
  private:

  public:

    Map(Map &&) = default;
    Map &operator=(Map &&) = default;

    const std::string &GetOpenDrive() const;

    detail::Waypoint GetWaypoint(const geom::Location &) const;

  private:

    explicit Map(std::string name, std::string open_drive);

    explicit Map(std::shared_ptr<road::Map> map) : _map(map) {}

    std::string _name;

    std::string _open_drive;

    std::shared_ptr<road::Map> _map;
  };

} // namespace client
} // namespace carla
