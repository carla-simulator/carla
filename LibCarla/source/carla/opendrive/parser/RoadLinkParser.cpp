// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "RoadLinkParser.h"

#include <cstdlib>

void carla::opendrive::parser::RoadLinkParser::ParseLink(
    const pugi::xml_node &xmlNode,
    carla::opendrive::types::RoadLinkInformation *out_link_information) {
  out_link_information->id = std::atoi(xmlNode.attribute("elementId").value());
  out_link_information->element_type = xmlNode.attribute("elementType").value();
  out_link_information->contact_point = xmlNode.attribute("contactPoint").value();
}

void carla::opendrive::parser::RoadLinkParser::Parse(
    const pugi::xml_node &xmlNode,
    carla::opendrive::types::RoadLink &out_road_link) {
  RoadLinkParser parser;

  const pugi::xml_node predecessorNode = xmlNode.child("predecessor");
  const pugi::xml_node successorNode = xmlNode.child("successor");

  if (predecessorNode) {
    out_road_link.predecessor = std::make_unique<carla::opendrive::types::RoadLinkInformation>();
    parser.ParseLink(predecessorNode, out_road_link.predecessor.get());
  }

  if (successorNode) {
    out_road_link.successor = std::make_unique<carla::opendrive::types::RoadLinkInformation>();
    parser.ParseLink(successorNode, out_road_link.successor.get());
  }
}
