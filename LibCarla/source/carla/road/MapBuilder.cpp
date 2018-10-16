// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapBuilder.h"

namespace carla {
namespace road {

  bool MapBuilder::AddRoadSegmentDefinition(RoadSegmentDefinition &seg) {
    _temp_sections.emplace(seg.GetId(), std::move(seg));
    return true;
  }

  Map MapBuilder::Build() {
    // Move the RoadSegmentDefinitions needed information to a RoadSegments
    for (auto &&id_seg : _temp_sections) {
      MakeElement<RoadSegment>(id_seg.first, std::move(id_seg.second));
    }

    // Set the total length of each road based on the geometries
    for (auto &&id_seg :_map._elements) {
      double total_length = 0.0;
      for (auto &&geom : id_seg.second.get()->_geom) {
        total_length += geom.get()->GetLength();
      }
      id_seg.second.get()->_length = total_length;
    }

    // Create the pointers between RoadSegments based on the ids
    for (auto &&id_seg : _temp_sections) {
      for (auto &t : id_seg.second.GetPredecessorID()) {
        _map._elements[id_seg.first]->PredEmplaceBack(_map._elements[t].get());
      }
      for (auto &t : id_seg.second.GetSuccessorID()) {
        _map._elements[id_seg.first]->SuccEmplaceBack(_map._elements[t].get());
      }
    }

    // _map is a memeber of MapBuilder so you must especify if
    // you want to keep it (will return copy -> Map(const Map &))
    // or move it (will return move -> Map(Map &&))
    return std::move(_map);
  }

  bool MapBuilder::InterpretRoadFlow() {
    // todo
    return false;
  }

} // namespace road
} // namespace carla
