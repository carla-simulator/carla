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
#include <carla/road/element/RoadInfoVisitor.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <fstream>

using namespace carla::road;
using namespace carla::road::element;
using namespace carla::geom;
using namespace carla::opendrive;

void print_roads(boost::optional<Map>& map) {
  std::ofstream file;
  file.open("roads.txt", std::ios::out | std::ios::trunc);

  for (auto &road : map->_data.GetRoads()) {
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

  auto& junctions = map->_data.GetJunctions();
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
    ASSERT_TRUE(map.has_value());

    // Test junctions
    // print_roads(map);
    test_junctions(map);
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
