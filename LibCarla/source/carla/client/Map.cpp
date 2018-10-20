// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Map.h"
#include "carla/client/Map.h"
#include "carla/opendrive/OpenDrive.h"

namespace carla {
namespace client {

  Map::Map(std::string name, std::string open_drive)
    : _name(std::move(name)),
      _open_drive(std::move(open_drive)) {
    _map = std::make_shared<road::Map>(opendrive::OpenDrive::Load(open_drive));
  }

  const std::string &Map::GetOpenDrive() const {
    return _open_drive;
  }

  // @todo
  /*detail::Waypoint Map::GetWaypoint(const geom::Location &loc) const {

  }*/

} // namespace client
} // namespace carla
