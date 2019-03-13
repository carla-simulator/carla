// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/GeometryParser.h"

#include "carla/opendrive/parser/pugixml/pugixml.hpp"

#include "carla/road/MapBuilder.h"

namespace carla {
namespace opendrive {
namespace parser {

  void ParseArc() {}

  void ParseLine() {}

  void ParseSpiral() {}

  void GeometryParser::Parse(
      const pugi::xml_document &/* xml */,
      carla::road::MapBuilder &/* map_builder */) {}

} // namespace parser
} // namespace opendrive
} // namespace carla
