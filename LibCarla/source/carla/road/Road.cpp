// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Road.h"
#include "carla/road/Lane.h"
#include "carla/road/MapData.h"
#include "carla/Logging.h"
#include "carla/ListView.h"

#include <boost/optional.hpp>

namespace carla {
namespace road {

  const MapData *Road::GetMap() const {
    return _map_data;
  }

  RoadId Road::GetId() const {
    return _id;
  }

  std::string Road::GetName() const {
    return _name;
  }

  float Road::GetLength() const {
    return _length;
  }

  bool Road::IsJunction() const {
    return _is_junction;
  }

  JuncId Road::GetJunction() const {
    return _junction_id;
  }

  std::vector<const Road *> Road::GetNexts() const {
    std::vector<const Road *> vec;
    for (auto &&next : _nexts) {
      vec.emplace_back(_map_data->GetRoad(next));
    }
    return vec;
  }

  std::vector<const Road *> Road::GetPrevs() const {
    std::vector<const Road *> vec;
    for (auto &&prev : _prevs) {
      vec.emplace_back(_map_data->GetRoad(prev));
    }
    return vec;
  }

  Lane *Road::GetLane(const LaneId id, const float s) {
    // Get a reversed list of elements that have key
    // value GetDistance() <= s
    auto sections = MakeListView(
        std::make_reverse_iterator(_lane_sections.upper_bound(s)),
        _lane_sections.rend());

    auto validate = [&sections](auto &&it) {
      return
          it != sections.end() &&
          it->second.GetDistance() == sections.begin()->second.GetDistance();
    };

    for (auto i = sections.begin(); validate(i); ++i) {
      auto search = i->second.GetLanes().find(id);
      if (search != i->second.GetLanes().end()) {
        return &search->second;
      }
    }

    log_warning("id", id, "at distance", s, "not found in road", _id);
    return nullptr;
  }

} // road
} // carla
