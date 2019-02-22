// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "../types.h"

#include "./pugixml/pugixml.hpp"

namespace carla {
namespace opendrive {
namespace parser {

  class LaneParser {
  private:

    void ParseLane(
        const pugi::xml_node &xmlNode,
        std::vector<types::LaneInfo> &out_lane);

    void ParseLaneSpeed(
        const pugi::xml_node &xmlNode,
        std::vector<types::LaneSpeed> &out_lane_speed);

    void ParseLaneLink(
        const pugi::xml_node &xmlNode,
        std::unique_ptr<types::LaneLink> &out_lane_link);

    void ParseLaneOffset(
        const pugi::xml_node &xmlNode,
        std::vector<types::LaneOffset> &out_lane_offset);

    void ParseLaneWidth(
        const pugi::xml_node &xmlNode,
        std::vector<types::LaneWidth> &out_lane_width);

    void ParseLaneRoadMark(
        const pugi::xml_node &xmlNode,
        std::vector<types::LaneRoadMark> &out_lane_mark);

  public:

    static void Parse(
        const pugi::xml_node &xmlNode,
        types::Lanes &out_lanes);
  };

} // parser
} // opendrive
} // carla
