// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/MapData.h"

namespace carla {
namespace road {

  const Road *MapData::GetRoad(const RoadId id) const {
    const auto search = _roads.find(id);
    if (search != _roads.end()) {
      return &search->second;
    }
    return nullptr;
  }

} // namespace road
} // namespace carla
