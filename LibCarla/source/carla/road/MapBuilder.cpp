// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/MapBuilder.h"
#include "carla/road/element/RoadInfoVisitor.h"

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

    SetTotalRoadSegmentLength();

    CreatePointersBetweenRoadSegments();

    ComputeLaneCenterOffset();

    // _map_data is a memeber of MapBuilder so you must especify if
    // you want to keep it (will return copy -> Map(const Map &))
    // or move it (will return move -> Map(Map &&))
    return SharedPtr<Map>(new Map{std::move(_map_data)});
  }

  void MapBuilder::SetTotalRoadSegmentLength() {
    for (auto &&id_seg : _map_data._elements) {
      double total_length = 0.0;
      for (auto &&geom : id_seg.second.get()->_geom) {
        total_length += geom.get()->GetLength();
      }
      id_seg.second.get()->_length = total_length;
    }
  }

  void MapBuilder::CreatePointersBetweenRoadSegments() {
    for (auto &&id_seg : _temp_sections) {
      for (auto &t : id_seg.second.GetPredecessorID()) {
        _map_data._elements[id_seg.first]->PredEmplaceBack(_map_data._elements[t].get());
      }
      for (auto &t : id_seg.second.GetSuccessorID()) {
        _map_data._elements[id_seg.first]->SuccEmplaceBack(_map_data._elements[t].get());
      }
    }
  }

  void MapBuilder::ComputeLaneCenterOffset() {
    for (auto &&element : _map_data._elements) {
      RoadSegment *road_seg = element.second.get();

      // get all the RoadGeneralInfo given the type and a distance 0.0
      auto up_bound_g = decltype(road_seg->_info)::reverse_iterator(road_seg->_info.upper_bound(0.0));
      auto general_info = MakeRoadInfoIterator<RoadGeneralInfo>(up_bound_g, road_seg->_info.rend());

      // get all the RoadInfoLane given the type and a distance 0.0
      auto up_bound_l = decltype(road_seg->_info)::reverse_iterator(road_seg->_info.upper_bound(0.0));
      auto lane_info = MakeRoadInfoIterator<RoadInfoLane>(up_bound_l, road_seg->_info.rend());

      // check that have a RoadGeneralInfo
      if (!lane_info.IsAtEnd()) {

        double lane_offset = 0.0;
        if (!general_info.IsAtEnd()) {
          lane_offset = (*general_info)->GetLanesOffset().at(0).second;
        }

        double current_width = lane_offset;

        for (auto &&current_lane_id :
            (*lane_info)->getLanesIDs(element::RoadInfoLane::which_lane_e::Left)) {
          const double half_width = (*lane_info)->getLane(current_lane_id)->_width * 0.5;

          current_width += half_width;
          (*lane_info)->_lanes[current_lane_id]._lane_center_offset = current_width;
          current_width += half_width;
        }

        current_width = lane_offset;

        for (auto &&current_lane_id :
            (*lane_info)->getLanesIDs(element::RoadInfoLane::which_lane_e::Right)) {
          const double half_width = (*lane_info)->getLane(current_lane_id)->_width * 0.5;

          current_width -= half_width;
          (*lane_info)->_lanes[current_lane_id]._lane_center_offset = current_width;
          current_width -= half_width;
        }
      }
    }
  }

} // namespace road
} // namespace carla
