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
