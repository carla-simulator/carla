// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/RoadTypes.h"

namespace pugi {
  class xml_document;
  class xml_node;
} // namespace pugi

namespace carla {

namespace road {
  class MapBuilder;
} // namespace road

namespace opendrive {
namespace parser {

  class LaneParser {
  private:

    static void ParseLanes(
        road::RoadId road_id,
        float s,
        const pugi::xml_node &parent_node,
        carla::road::MapBuilder &map_builder);

  public:

    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);

  };

} // namespace parser
} // namespace opendrive
} // namespace carla
