// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/MapBuilder.h"

using namespace carla::road::element;

namespace carla {
namespace road {

  bool MapBuilder::AddRoadSegmentDefinition(RoadSegmentDefinition &seg) {
    _temp_sections.emplace(seg.GetId(), std::move(seg));
    return true;
  }

  SharedPtr<Map> MapBuilder::Build() {
    // Move the RoadSegmentDefinitions needed information to a RoadSegments
    for (auto &&id_seg : _temp_sections) {
      MakeElement<RoadSegment>(id_seg.first, std::move(id_seg.second));
    }

    // Set the total length of each road based on the geometries
    for (auto &&id_seg :_map_data._elements) {
      double total_length = 0.0;
      for (auto &&geom : id_seg.second.get()->_geom) {
        total_length += geom.get()->GetLength();
      }
      id_seg.second.get()->_length = total_length;
    }

    // Create the pointers between RoadSegments based on the ids
    for (auto &&id_seg : _temp_sections) {
      for (auto &t : id_seg.second.GetPredecessorID()) {
        _map_data._elements[id_seg.first]->PredEmplaceBack(_map_data._elements[t].get());
      }
      for (auto &t : id_seg.second.GetSuccessorID()) {
        _map_data._elements[id_seg.first]->SuccEmplaceBack(_map_data._elements[t].get());
      }
    }

    // _map_data is a memeber of MapBuilder so you must especify if
    // you want to keep it (will return copy -> Map(const Map &))
    // or move it (will return move -> Map(Map &&))
    return SharedPtr<Map>(new Map{std::move(_map_data)});
  }

} // namespace road
} // namespace carla
