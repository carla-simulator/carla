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

  boost::optional<Map> MapBuilder::Build() {

    SetTotalRoadSegmentLength();

    CreatePointersBetweenRoadSegments();

    // _map_data is a memeber of MapBuilder so you must especify if
    // you want to keep it (will return copy -> Map(const Map &))
    // or move it (will return move -> Map(Map &&))
    return Map{std::move(_map_data)};
  }

  void MapBuilder::AddJunction(const int32_t id, const std::string name) {
    _map_data.GetJunctions().emplace(id, Junction(id, name));
  }

  void MapBuilder::AddConnection(
      const int32_t junction_id,
      const int32_t connection_id,
      const int32_t incoming_road,
      const int32_t connecting_road) {
    _map_data.GetJunction(junction_id)->GetConnections().emplace(connection_id,
        Junction::Connection(connection_id, incoming_road, connecting_road));
  }

  void MapBuilder::AddLaneLink(
      const int32_t junction_id,
      const int32_t connection_id,
      const int32_t from,
      const int32_t to) {

    _map_data.GetJunction(junction_id)->GetConnection(connection_id)->AddLaneLink(from, to);
  }

} // namespace road
} // namespace carla
