// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Map.h"

namespace carla {
namespace road {

  bool Map::ExistId(id_type id) const {
    return _sections.count(id);
  }

  const RoadElement *Map::GetRoad(id_type id) {
    if (ExistId(id)) {
      return _sections.find(id)->second.get();
    }
    return nullptr;
  }

} // namespace road
} // namespace carla
