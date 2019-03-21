// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
#include "OpenDrive.h"

#include <carla/road/MapBuilder.h>
#include <carla/geom/Location.h>
#include <carla/geom/Math.h>
#include <carla/road/element/RoadInfoGeometry.h>
#include <carla/road/element/RoadInfoVisitor.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/opendrive/parser/pugixml/pugixml.hpp>
#include <fstream>

using namespace carla::road;
using namespace carla::road::element;
using namespace carla::geom;
using namespace carla::opendrive;

const std::string BASE_PATH = LIBCARLA_TEST_CONTENT_FOLDER "/OpenDrive/";

// Geometry
void test_geometry(const pugi::xml_document &xml, boost::optional<Map>& map)
{
  pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

  for (pugi::xml_node road_node : open_drive_node.children("road")) {
    RoadId road_id = road_node.attribute("id").as_int();

    for (pugi::xml_node plan_view_nodes : road_node.children("planView")) {
      auto geometries_parser = plan_view_nodes.children("geometry");
      size_t total_geometries_parser = std::distance(geometries_parser.begin(), geometries_parser.end());

      size_t total_geometries = 0;
      for (pugi::xml_node geometry_node : plan_view_nodes.children("geometry")){
        float s = geometry_node.attribute("s").as_float();
        auto geometry = map->GetMap().GetRoad(road_id)->GetInfo<RoadInfoGeometry>(s);
        if (geometry != nullptr)
          ++total_geometries;
      }
      ASSERT_EQ(total_geometries, total_geometries_parser);
    }
  }
}


void test_roads(const pugi::xml_document &xml, boost::optional<Map>& map)
{
  pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

  // Check total Roads
  auto roads_parser = open_drive_node.children("road");
  size_t total_roads_parser = std::distance(roads_parser.begin(), roads_parser.end());
  size_t total_roads = map->GetMap().GetRoads().size();
  ASSERT_EQ(total_roads, total_roads_parser);

  for (pugi::xml_node road_node : roads_parser) {
    RoadId road_id = road_node.attribute("id").as_int();

    for (pugi::xml_node lanes_node : road_node.children("lanes")) {

      // Check total Lane Sections
      auto lane_sections_parser = lanes_node.children("laneSection");
      size_t total_lane_sections_parser = std::distance(lane_sections_parser.begin(), lane_sections_parser.end());
      size_t total_lane_sections = map->GetMap().GetRoad(road_id)->GetLaneSections().size();
      ASSERT_EQ(total_lane_sections, total_lane_sections_parser);

      for (pugi::xml_node lane_section_node : lane_sections_parser) {

        // Check total Lanes
        float s = lane_section_node.attribute("s").as_float();

        auto ls_begin = map->GetMap().GetRoad(road_id)->GetLaneSectionsAt(s).begin();
        auto ls_end = map->GetMap().GetRoad(road_id)->GetLaneSectionsAt(s).end();
        size_t total_lanes = 0u;
        for (auto& it = ls_begin; it != ls_end; ++it) {
          total_lanes = it->GetLanes().size();
        }
        auto left_nodes = lane_section_node.child("left").children("lane");
        auto center_nodes = lane_section_node.child("center").children("lane");
        auto right_nodes = lane_section_node.child("right").children("lane");
        size_t total_lanes_parser = std::distance(left_nodes.begin(), left_nodes.end());
        total_lanes_parser += std::distance(right_nodes.begin(), right_nodes.end());
        total_lanes_parser += std::distance(center_nodes.begin(), center_nodes.end());

        ASSERT_EQ(total_lanes, total_lanes_parser);
      }
    }
  }
}

// Junctions
void test_junctions(const pugi::xml_document &xml, boost::optional<Map>& map)
{

  pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

  // Check total number of junctions
  auto& junctions = map->GetMap().GetJunctions();
  size_t total_junctions_parser = std::distance(open_drive_node.children("junction").begin(), open_drive_node.children("junction").end());

  ASSERT_EQ(junctions.size(), total_junctions_parser);

  for (pugi::xml_node junction_node : open_drive_node.children("junction")) {
    // Check total number of connections
    size_t total_connections_parser = std::distance(junction_node.children("connection").begin(), junction_node.children("connection").end());

    JuncId junction_id = junction_node.attribute("id").as_int();
    auto& junction = junctions.find(junction_id)->second;

    auto& connections = junction.GetConnections();

    ASSERT_EQ(connections.size(), total_connections_parser);

    for (pugi::xml_node connection_node : junction_node.children("connection")) {
      size_t total_lane_links_parser = std::distance(connection_node.children("laneLink").begin(), connection_node.children("laneLink").end());

      ConId connection_id = connection_node.attribute("id").as_uint();
      auto& connection = connections.find(connection_id)->second;

    auto& lane_links = connection.lane_links;

    ASSERT_EQ(lane_links.size(), total_lane_links_parser);

    }
  }
}

void print_roads(boost::optional<Map>& map) {
  std::ofstream file;
  file.open("roads.txt", std::ios::out | std::ios::trunc);

  for (auto &road : map->GetMap().GetRoads()) {
    file << "Road: " << road.second.GetId() << std::endl;
    for (auto &section : road.second.GetLaneSections()) {
      file << " Section: " << section.GetDistance() << std::endl;
      for (auto &lane : section.GetLanes()) {
        file << "   Lane: " << lane.second.GetId() << std::endl;
        file << "     Next: " << lane.second.GetNextLanes().size() << std::endl;
        file << "       ";
        for (auto link : lane.second.GetNextLanes()) {
          file << " (" << link->GetRoad()->GetId() << "," << link->GetId() << ")";
        }
        file << std::endl;
        file << "     Prev: " << lane.second.GetPreviousLanes().size() << std::endl;
        file << "       ";
        for (auto link : lane.second.GetPreviousLanes()) {
          file << " (" << link->GetRoad()->GetId() << "," << link->GetId() << ")";
        }
        file << std::endl;
      }
    }
  }

  file.close();
}

void test_junctions(boost::optional<Map>& map) {

  struct LaneLink {
    LaneId from;
    LaneId to;
  };

  struct Connection {
    ConId id;
    RoadId incoming_road;
    RoadId connecting_road;
    std::vector<LaneLink> lane_links;
  };

  struct Junction {
    JuncId id;
    std::vector<Connection> connections;
  };

  // Lane Links
  LaneLink lane_link { -1, -1};

  // Connection
  Connection connection { 1u, 0u, 82u, {lane_link} };

  Junction junction_1 {80u, {connection} };

  auto& junctions = map->GetMap().GetJunctions();
  for (auto& junction : junctions) {
    ASSERT_EQ(junction.second.GetId(), junction_1.id);
    ASSERT_EQ(junction.second.GetConnection(1)->id, 1u);
    ASSERT_EQ(junction.second.GetConnection(1)->incoming_road, 0u);
    ASSERT_EQ(junction.second.GetConnection(1)->connecting_road, 82u);
    ASSERT_EQ(junction.second.GetConnection(1)->lane_links[0].from, -1);
    ASSERT_EQ(junction.second.GetConnection(1)->lane_links[0].from, -1);
  }
}

TEST(road, parse_files) {
  for (const auto &file : util::OpenDrive::GetAvailableFiles()) {
    std::cerr << file << std::endl;
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map);
  }
}

TEST(road, parse_junctions) {
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map.has_value());

    const std::string full_path = BASE_PATH + file;
    pugi::xml_document xml;

    pugi::xml_parse_result result = xml.load_file( full_path.c_str());
    ASSERT_TRUE(result);

    test_junctions(xml, map);
  }

}

TEST(road, parse_lanes) {
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map.has_value());

    const std::string full_path = BASE_PATH + file;
    pugi::xml_document xml;
    pugi::xml_parse_result result = xml.load_file( full_path.c_str());
    ASSERT_TRUE(result);

    test_roads(xml, map);
  }

}

TEST(road, parse_geometry) {
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map.has_value());

    const std::string full_path = BASE_PATH + file;
    pugi::xml_document xml;
    pugi::xml_parse_result result = xml.load_file( full_path.c_str());
    ASSERT_TRUE(result);

    test_geometry(xml, map);
  }

}

TEST(road, iterate_waypoints) {
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    carla::logging::log("Parsing", file);
    auto m = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(m.has_value());
    auto &map = *m;
    for (auto &&wp : map.GenerateWaypoints(5.0f)) {
      ASSERT_TRUE(map.IsValid(wp));
      for (auto &&next : map.GetNext(wp, 4.0f)) {
        ASSERT_TRUE(map.IsValid(next));
        auto right = map.GetRight(next);
        if (right.has_value()) {
          ASSERT_TRUE(map.IsValid(*right));
        }
        auto left = map.GetLeft(next);
        if (left.has_value()) {
          ASSERT_TRUE(map.IsValid(*left));
        }
      }
    }
  }
}

/*
TEST(road, add_geometry) {

}

TEST(road, add_information) {

}

TEST(road, set_and_get_connections_for) {

}

TEST(road, geom_line) {

}

TEST(road, geom_arc) {

}

TEST(road, geom_spiral) {

}

TEST(road, get_information) {

}
*/
