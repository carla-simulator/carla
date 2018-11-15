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

  class JunctionParser {
  private:

    void ParseConnection(
        const pugi::xml_node &xmlNode,
        std::vector<carla::opendrive::types::JunctionConnection> &out_connections);

    void ParseLaneLink(
        const pugi::xml_node &xmlNode,
        std::vector<carla::opendrive::types::JunctionLaneLink> &out_lane_link);

  public:

    static void Parse(
        const pugi::xml_node &xmlNode,
        std::vector<carla::opendrive::types::Junction> &out_junction);
  };

}
}
}
