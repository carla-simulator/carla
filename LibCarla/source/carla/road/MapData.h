// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Iterator.h"
#include "carla/ListView.h"
#include "carla/NonCopyable.h"
#include "carla/road/element/RoadSegment.h"

#include <boost/iterator/transform_iterator.hpp>

#include <unordered_map>

namespace carla {
namespace road {

  struct lane_junction_t {
    std::string contact_point = "start";
    int junction_id = -1;

    int connection_road = -1;
    int incomming_road = -1;

    std::vector<int> from_lane;
    std::vector<int> to_lane;
  };

  class MapData
    : private MovableNonCopyable {
  public:

    const element::RoadSegment *GetRoad(element::id_type id) const {
      auto it = _elements.find(id);
      return it != _elements.end() ? it->second.get() : nullptr;
    }

    auto GetAllIds() const {
      return MakeListView(
          iterator::make_map_keys_iterator(_elements.begin()),
          iterator::make_map_keys_iterator(_elements.end()));
    }

    size_t GetRoadCount() const {
      return _elements.size();
    }

    const std::vector<lane_junction_t> &GetJunctionInformation() const {
      return _junction_information;
    }

    auto GetRoadSegments() const {
      using const_ref = const element::RoadSegment &;
      auto get = [](auto &pair) -> const_ref { return *pair.second; };
      return MakeListView(
          boost::make_transform_iterator(_elements.begin(), get),
          boost::make_transform_iterator(_elements.end(), get));
    }

  private:

    friend class MapBuilder;

    MapData() = default;

    void SetJunctionInformation(const std::vector<lane_junction_t> &junctionInfo) {
      _junction_information = junctionInfo;
    }

    std::vector<lane_junction_t> _junction_information;

    std::unordered_map<
        element::id_type,
        std::unique_ptr<element::RoadSegment>> _elements;
  };

} // namespace road
} // namespace carla
