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

  class GeometryParser {
  private:

    void ParseArc(
        const pugi::xml_node &xmlNode,
        carla::opendrive::types::GeometryAttributesArc *out_geometry_arc);

    void ParseLine(
        const pugi::xml_node &xmlNode,
        carla::opendrive::types::GeometryAttributesLine *out_geometry_line);

    void ParseSpiral(
        const pugi::xml_node &xmlNode,
        carla::opendrive::types::GeometryAttributesSpiral *out_geometry_spiral);

  public:

    static void Parse(
        const pugi::xml_node &xmlNode,
        std::vector<std::unique_ptr<carla::opendrive::types::GeometryAttributes>> &out_geometry_attributes);
  };

}
}
}
