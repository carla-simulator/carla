// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/JunctionParser.h"

#include "carla/road/MapBuilder.h"

#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  void JunctionParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    struct LaneLink {
      road::LaneId from;
      road::LaneId to;
    };

    struct Connection {
      road::ConId id;
      road::RoadId incoming_road;
      road::RoadId connecting_road;
      std::vector<LaneLink> lane_links;
    };

    struct Junction {
      road::JuncId id;
      std::string name;
      std::vector<Connection> connections;
      std::set<road::ContId> controllers;
    };

    pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

    // Junctions
    std::vector<Junction> junctions;
    for (pugi::xml_node junction_node : open_drive_node.children("junction")) {
      Junction junction;
      junction.id = junction_node.attribute("id").as_int();
      junction.name = junction_node.attribute("name").value();

      // Connections
      for (pugi::xml_node connection_node : junction_node.children("connection")) {

        Connection connection;
        connection.id = connection_node.attribute("id").as_uint();
        connection.incoming_road = connection_node.attribute("incomingRoad").as_uint();
        connection.connecting_road = connection_node.attribute("connectingRoad").as_uint();

        // Lane Links
        for (pugi::xml_node lane_link_node : connection_node.children("laneLink")) {

          LaneLink lane_link;
          lane_link.from = lane_link_node.attribute("from").as_int();
          lane_link.to = lane_link_node.attribute("to").as_int();

          connection.lane_links.push_back(lane_link);
        }

        junction.connections.push_back(connection);
      }

      // Controller
      for (pugi::xml_node controller_node : junction_node.children("controller")) {
        const road::ContId controller_id = controller_node.attribute("id").value();
        // const std::string controller_name = controller_node.attribute("name").value();
        // const uint32_t controller_sequence = controller_node.attribute("sequence").as_uint();
        junction.controllers.insert(controller_id);
      }

      junctions.push_back(junction);
    }

    // Fill Map Builder
    for (auto &junction : junctions) {
      map_builder.AddJunction(junction.id, junction.name);
      for (auto &connection : junction.connections) {
        map_builder.AddConnection(
            junction.id,
            connection.id,
            connection.incoming_road,
            connection.connecting_road);
        for (auto &lane_link : connection.lane_links) {
          map_builder.AddLaneLink(junction.id,
              connection.id,
              lane_link.from,
              lane_link.to);
        }
      }
      map_builder.AddJunctionController(junction.id, std::move(junction.controllers));
    }
  }
} // namespace parser
} // namespace opendrive
} // namespace carla
