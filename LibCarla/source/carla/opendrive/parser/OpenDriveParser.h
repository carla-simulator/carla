// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "TrafficSignalsParser.h"
#include "RoadLinkParser.h"
#include "JunctionParser.h"

#include "ProfilesParser.h"
#include "GeometryParser.h"
#include "LaneParser.h"

#include "TrafficGroupParser.h"
#include "TrafficSignParser.h"

#include "./pugixml/pugixml.hpp"

#include <iostream>

enum class XmlInputType : int {
  FILE,
  CONTENT
};

struct OpenDriveParser {
  static bool Parse(
      const char *xml,
      carla::opendrive::types::OpenDriveData &out_open_drive_data,
      XmlInputType inputType,
      std::string *out_error = nullptr) {
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

      carla::opendrive::parser::ProfilesParser::Parse(road, openDriveRoadInformation.road_profiles);

      carla::opendrive::parser::RoadLinkParser::Parse(road.child("link"), openDriveRoadInformation.road_link);
      carla::opendrive::parser::TrafficSignalsParser::Parse(road.child("signals"),
          openDriveRoadInformation.trafic_signals);

      carla::opendrive::parser::LaneParser::Parse(road.child("lanes"), openDriveRoadInformation.lanes);
      carla::opendrive::parser::GeometryParser::Parse(road.child("planView"),
          openDriveRoadInformation.geometry_attributes);

      out_open_drive_data.roads.emplace_back(std::move(openDriveRoadInformation));
    }

    for (pugi::xml_node junction = xmlDoc.child("OpenDRIVE").child("junction");
        junction;
        junction = junction.next_sibling("junction")) {
      carla::opendrive::parser::JunctionParser::Parse(junction, out_open_drive_data.junctions);
    }

    for (pugi::xml_node tlgroup = xmlDoc.child("OpenDRIVE").child("tlGroup");
        tlgroup;
        tlgroup = tlgroup.next_sibling("tlGroup")) {
      carla::opendrive::parser::TrafficGroupParser::Parse(tlgroup, out_open_drive_data.trafficlightgroups);
    }

    for (pugi::xml_node trafficsigns = xmlDoc.child("OpenDRIVE").child("trafficsign");
        trafficsigns;
        trafficsigns = trafficsigns.next_sibling("trafficsign")) {
      carla::opendrive::parser::TrafficSignParser::Parse(trafficsigns, out_open_drive_data.trafficsigns);
    }

    out_open_drive_data.geoReference = xmlDoc.child("OpenDRIVE").child("header").child_value("geoReference");

    return true;
  }
};
