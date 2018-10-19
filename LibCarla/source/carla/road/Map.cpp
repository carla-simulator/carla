// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Map.h"

using namespace carla::road::element;

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

  std::vector<id_type> Map::GetAllIds() const {
    std::vector<id_type> result;
    for (auto &&e : _elements) {
      result.emplace_back(e.first);
    }
    return result;
  }

  uint32_t Map::GetRoadCount() const {
    return _elements.size();
  }

} // namespace road
} // namespace carla
