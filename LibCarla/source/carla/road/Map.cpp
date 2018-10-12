// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Map.h"

namespace carla {
namespace road {

  bool Map::ExistId(id_type id) const {
    return _elements.count(id);
  }

  const RoadSegment *Map::GetRoad(id_type id) const {
    if (ExistId(id)) {
      return _elements.find(id)->second.get();
    }
    return nullptr;
  }

  uint32_t Map::GetRoadCount() const {
    return _elements.size();
  }

} // namespace road
} // namespace carla
