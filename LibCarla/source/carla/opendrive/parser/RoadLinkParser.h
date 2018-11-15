#pragma once

#include "../types.h"

#include "./pugixml/pugixml.hpp"

namespace carla {
namespace opendrive {
namespace parser {

  class RoadLinkParser {
  private:

    void ParseLink(
        const pugi::xml_node &xmlNode,
        carla::opendrive::types::RoadLinkInformation *out_link_information);

  public:

    static void Parse(
        const pugi::xml_node &xmlNode,
        carla::opendrive::types::RoadLink &out_road_link);
  };

}
}
}
