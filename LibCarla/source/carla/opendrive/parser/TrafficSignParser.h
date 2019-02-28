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

  class TrafficSignParser {
  private:

    void ParseBoxAreas(
        const pugi::xml_node &xmlNode,
        std::vector<carla::opendrive::types::BoxComponent> &out_boxareas);

  public:

    static void Parse(
        const pugi::xml_node &xmlNode,
        std::vector<carla::opendrive::types::TrafficSign> &out_trafficsigns);

  };

}
}
}
