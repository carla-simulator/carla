// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/OpenDriveParser.h"

#include "carla/Logging.h"
#include "carla/road/MapBuilder.h"

#include "carla/opendrive/parser/GeoReferenceParser.h"
#include "carla/opendrive/parser/GeometryParser.h"
#include "carla/opendrive/parser/JunctionParser.h"
#include "carla/opendrive/parser/LaneParser.h"
#include "carla/opendrive/parser/ProfilesParser.h"
#include "carla/opendrive/parser/RoadLinkParser.h"
#include "carla/opendrive/parser/TrafficGroupParser.h"
#include "carla/opendrive/parser/TrafficSignParser.h"
#include "carla/opendrive/parser/TrafficSignalsParser.h"

#include "carla/opendrive/pugixml/pugixml.hpp"

namespace carla {
namespace opendrive {

  SharedPtr<road::Map> OpenDriveParser::Load(const std::string &opendrive) {
    pugi::xml_document xml;
    pugi::xml_parse_result parse_result = xml.load_string(opendrive.c_str());

    if (parse_result == false) {
      log_error("unable to parse the XML string");
      return {};
    }

    carla::road::MapBuilder map_builder;

    // GeoReferenceParser::Parse(xml, map_builder):
    parser::GeometryParser::Parse(xml, map_builder);
    parser::JunctionParser::Parse(xml, map_builder);
    parser::LaneParser::Parse(xml, map_builder);
    parser::ProfilesParser::Parse(xml, map_builder);
    parser::RoadLinkParser::Parse(xml, map_builder);
    parser::TrafficGroupParser::Parse(xml, map_builder);
    parser::TrafficSignParser::Parse(xml, map_builder);
    parser::TrafficSignalsParser::Parse(xml, map_builder);

    return map_builder.Build();
  }

} // namespace opendrive
} // namespace carla
