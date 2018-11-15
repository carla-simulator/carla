// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "JunctionParser.h"

void carla::opendrive::parser::JunctionParser::Parse(
    const pugi::xml_node &xmlNode,
    std::vector<carla::opendrive::types::Junction> &out_junction) {
  carla::opendrive::parser::JunctionParser parser;
  carla::opendrive::types::Junction junction;

  junction.attributes.id = std::atoi(xmlNode.attribute("id").value());
  junction.attributes.name = xmlNode.attribute("name").value();

  parser.ParseConnection(xmlNode, junction.connections);
  out_junction.emplace_back(junction);
}

void carla::opendrive::parser::JunctionParser::ParseConnection(
    const pugi::xml_node &xmlNode,
    std::vector<carla::opendrive::types::JunctionConnection> &out_connections) {
  for (pugi::xml_node junctionConnection = xmlNode.child("connection");
      junctionConnection;
      junctionConnection = junctionConnection.next_sibling("connection")) {
    carla::opendrive::types::JunctionConnection jConnection;

    jConnection.attributes.id = std::atoi(junctionConnection.attribute("id").value());
    jConnection.attributes.contact_point = junctionConnection.attribute("contactPoint").value();

    jConnection.attributes.incoming_road = std::atoi(junctionConnection.attribute("incomingRoad").value());
    jConnection.attributes.connecting_road =
        std::atoi(junctionConnection.attribute("connectingRoad").value());

    ParseLaneLink(junctionConnection, jConnection.links);
    out_connections.emplace_back(jConnection);
  }
}

void carla::opendrive::parser::JunctionParser::ParseLaneLink(
    const pugi::xml_node &xmlNode,
    std::vector<carla::opendrive::types::JunctionLaneLink> &out_lane_link) {
  for (pugi::xml_node junctionLaneLink = xmlNode.child("laneLink");
      junctionLaneLink;
      junctionLaneLink = junctionLaneLink.next_sibling("laneLink")) {
    carla::opendrive::types::JunctionLaneLink jLaneLink;

    jLaneLink.from = std::atoi(junctionLaneLink.attribute("from").value());
    jLaneLink.to = std::atoi(junctionLaneLink.attribute("to").value());

    out_lane_link.emplace_back(jLaneLink);
  }
}
