// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/OpenDriveParser.h"

#include "carla/Logging.h"
#include "carla/opendrive/parser/ControllerParser.h"
#include "carla/opendrive/parser/GeoReferenceParser.h"
#include "carla/opendrive/parser/GeometryParser.h"
#include "carla/opendrive/parser/JunctionParser.h"
#include "carla/opendrive/parser/LaneParser.h"
#include "carla/opendrive/parser/ObjectParser.h"
#include "carla/opendrive/parser/ProfilesParser.h"
#include "carla/opendrive/parser/RoadParser.h"
#include "carla/opendrive/parser/SignalParser.h"
#include "carla/opendrive/parser/TrafficGroupParser.h"
#include "carla/road/MapBuilder.h"

#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {

  boost::optional<road::Map> OpenDriveParser::Load(const std::string &opendrive) {
    pugi::xml_document xml;
    pugi::xml_parse_result parse_result = xml.load_string(opendrive.c_str());

    if (parse_result == false) {
      log_error("unable to parse the XML string");
      return {};
    }

    carla::road::MapBuilder map_builder;

    parser::GeoReferenceParser::Parse(xml, map_builder);
    parser::RoadParser::Parse(xml, map_builder);
    parser::JunctionParser::Parse(xml, map_builder);
    parser::GeometryParser::Parse(xml, map_builder);
    parser::LaneParser::Parse(xml, map_builder);
    parser::ProfilesParser::Parse(xml, map_builder);
    parser::TrafficGroupParser::Parse(xml, map_builder);
    parser::SignalParser::Parse(xml, map_builder);
    parser::ObjectParser::Parse(xml, map_builder);
    parser::ControllerParser::Parse(xml, map_builder);

    return map_builder.Build();
  }

} // namespace opendrive
} // namespace carla
