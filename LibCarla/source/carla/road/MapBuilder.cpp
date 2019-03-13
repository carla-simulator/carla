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

} // namespace road
} // namespace carla
