#pragma once

#include "../types.h"

#include "./pugixml/pugixml.hpp"

namespace carla {
namespace opendrive {
namespace parser {

  class TrafficSignalsParser {
  public:

    static void Parse(
        const pugi::xml_node &xmlNode,
        std::vector<carla::opendrive::types::TrafficSignalInformation> &out_traffic_signals);
  };

}
}
}
