// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/OpenDriveParser.h"

#include "carla/geom/GeoLocation.h"
#include "carla/opendrive/parser/GeoReferenceParser.h"
#include "carla/opendrive/parser/GeometryParser.h"
#include "carla/opendrive/parser/JunctionParser.h"
#include "carla/opendrive/parser/LaneParser.h"
#include "carla/opendrive/parser/ProfilesParser.h"
#include "carla/opendrive/parser/RoadLinkParser.h"
#include "carla/opendrive/parser/TrafficGroupParser.h"
#include "carla/opendrive/parser/TrafficSignParser.h"
#include "carla/opendrive/parser/TrafficSignalsParser.h"

#include "./pugixml/pugixml.hpp"

bool OpenDriveParser::Parse(
    const char *xml,
    carla::opendrive::types::OpenDriveData &out_open_drive_data,
    XmlInputType inputType,
    std::string *out_error) {
  namespace odp = carla::opendrive::parser;

  pugi::xml_document xmlDoc;
  pugi::xml_parse_result pugiParseResult;

  switch (inputType) {
    case XmlInputType::FILE: {
      pugiParseResult = xmlDoc.load_file(xml);
    } break;

    case XmlInputType::CONTENT: {
      pugiParseResult = xmlDoc.load_string(xml);
    } break;

    default: {
      // TODO(Andrei): Log some kind of error
      return false;
    } break;
  }

  if (pugiParseResult == false) {
    if (out_error != nullptr) {
      *out_error = pugiParseResult.description();
    }

    return false;
  }

  for (pugi::xml_node road = xmlDoc.child("OpenDRIVE").child("road");
      road;
      road = road.next_sibling("road")) {
    carla::opendrive::types::RoadInformation openDriveRoadInformation;

    openDriveRoadInformation.attributes.name = road.attribute("name").value();
    openDriveRoadInformation.attributes.id = std::atoi(road.attribute("id").value());
    openDriveRoadInformation.attributes.length = std::stod(road.attribute("length").value());
    openDriveRoadInformation.attributes.junction = std::atoi(road.attribute("junction").value());

    ///////////////////////////////////////////////////////////////////////////////

    odp::ProfilesParser::Parse(road, openDriveRoadInformation.road_profiles);

    odp::RoadLinkParser::Parse(road.child("link"), openDriveRoadInformation.road_link);
    odp::TrafficSignalsParser::Parse(road.child("signals"),
        openDriveRoadInformation.trafic_signals);

    odp::LaneParser::Parse(road.child("lanes"), openDriveRoadInformation.lanes);
    odp::GeometryParser::Parse(road.child("planView"),
        openDriveRoadInformation.geometry_attributes);

    out_open_drive_data.roads.emplace_back(std::move(openDriveRoadInformation));
  }

  for (pugi::xml_node junction = xmlDoc.child("OpenDRIVE").child("junction");
      junction;
      junction = junction.next_sibling("junction")) {
    odp::JunctionParser::Parse(junction, out_open_drive_data.junctions);
  }

  for (pugi::xml_node tlgroup = xmlDoc.child("OpenDRIVE").child("tlGroup");
      tlgroup;
      tlgroup = tlgroup.next_sibling("tlGroup")) {
    odp::TrafficGroupParser::Parse(tlgroup, out_open_drive_data.trafficlightgroups);
  }

  for (pugi::xml_node trafficsigns = xmlDoc.child("OpenDRIVE").child("trafficsign");
      trafficsigns;
      trafficsigns = trafficsigns.next_sibling("trafficsign")) {
    odp::TrafficSignParser::Parse(trafficsigns, out_open_drive_data.trafficsigns);
  }

  out_open_drive_data.geoReference = odp::GeoReferenceParser::Parse(
      xmlDoc.child("OpenDRIVE").child("header").child_value("geoReference"));

  return true;
}
