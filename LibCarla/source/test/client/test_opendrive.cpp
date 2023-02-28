// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
#include "OpenDrive.h"
#include "Random.h"

#include <carla/StopWatch.h>
#include <carla/ThreadPool.h>
#include <carla/geom/Location.h>
#include <carla/geom/Math.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/MapBuilder.h>
#include <carla/road/element/RoadInfoElevation.h>
#include <carla/road/element/RoadInfoGeometry.h>
#include <carla/road/element/RoadInfoMarkRecord.h>
#include <carla/road/element/RoadInfoVisitor.h>

#include <pugixml/pugixml.hpp>

#include <fstream>
#include <string>

using namespace carla::road;
using namespace carla::road::element;
using namespace carla::geom;
using namespace carla::opendrive;
using namespace util;

const std::string BASE_PATH = LIBCARLA_TEST_CONTENT_FOLDER "/OpenDrive/";

// Road Elevation
static void test_road_elevation(const pugi::xml_document &xml, boost::optional<Map>& map) {
  pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

  for (pugi::xml_node road_node : open_drive_node.children("road")) {
    RoadId road_id = road_node.attribute("id").as_uint();
    auto elevation_profile_nodes = road_node.children("elevationProfile");

    for (pugi::xml_node elevation_profile_node : elevation_profile_nodes) {
      auto total_elevations = 0;
      auto elevation_nodes = elevation_profile_node.children("elevation");
      auto total_elevation_parser = std::distance(elevation_nodes.begin(), elevation_nodes.end());

      for (pugi::xml_node elevation_node : elevation_nodes) {
        float s = elevation_node.attribute("s").as_float();
        const auto elevation = map->GetMap().GetRoad(road_id).GetInfo<RoadInfoElevation>(s);
        if (elevation != nullptr)
          ++total_elevations;
      }
      ASSERT_EQ(total_elevations, total_elevation_parser);
    }
  }
}

// Geometry
static void test_geometry(const pugi::xml_document &xml, boost::optional<Map>& map) {
  pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

  for (pugi::xml_node road_node : open_drive_node.children("road")) {
    RoadId road_id = road_node.attribute("id").as_uint();

    for (pugi::xml_node plan_view_nodes : road_node.children("planView")) {
      auto geometries_parser = plan_view_nodes.children("geometry");
      auto total_geometries_parser = std::distance(geometries_parser.begin(), geometries_parser.end());
      auto total_geometries = 0;
      for (pugi::xml_node geometry_node : plan_view_nodes.children("geometry")){
        float s = geometry_node.attribute("s").as_float();
        auto geometry = map->GetMap().GetRoad(road_id).GetInfo<RoadInfoGeometry>(s);
        if (geometry != nullptr)
          ++total_geometries;
      }
      ASSERT_EQ(total_geometries, total_geometries_parser);
    }
  }
}

// Road test
static auto get_total_road_marks(
    pugi::xml_object_range<pugi::xml_named_node_iterator> &lane_nodes,
    LaneSection& lane_section) {
  constexpr auto error = 1e-5;
  auto total_road_mark = 0;
  auto total_road_mark_parser = 0;
  for (pugi::xml_node lane_node : lane_nodes) {
    // Check Road Mark
    auto road_mark_nodes = lane_node.children("roadMark");
    total_road_mark_parser += std::distance(road_mark_nodes.begin(), road_mark_nodes.end());

    const int lane_id = lane_node.attribute("id").as_int();
    Lane* lane = nullptr;
    lane = lane_section.GetLane(lane_id);
    EXPECT_NE(lane, nullptr);

    // <roadMark sOffset="0.0000000000000000e+0" type="none" material="standard" color="white" laneChange="none"/>
    for (pugi::xml_node road_mark_node : road_mark_nodes) {
      const auto s_offset = road_mark_node.attribute("sOffset").as_double();
      const auto type = road_mark_node.attribute("type").as_string();
      const auto material = road_mark_node.attribute("material").as_string();
      const auto color = road_mark_node.attribute("color").as_string();
      const auto road_mark = lane->GetInfo<RoadInfoMarkRecord>(lane->GetDistance() + s_offset);
      if (road_mark != nullptr) {
        EXPECT_NEAR(lane->GetDistance() + s_offset, road_mark->GetDistance(), error);
        EXPECT_EQ(type, road_mark->GetType());
        EXPECT_EQ(material, road_mark->GetMaterial());
        EXPECT_EQ(color, road_mark->GetColor());
        ++total_road_mark;
      }
    }
  }
  return std::make_pair(total_road_mark, total_road_mark_parser);
}

static void test_roads(const pugi::xml_document &xml, boost::optional<Map>& map) {
  pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

  // Check total Roads
  auto roads_parser = open_drive_node.children("road");
  auto total_roads_parser = std::distance(roads_parser.begin(), roads_parser.end());
  auto total_roads = map->GetMap().GetRoads().size();
  ASSERT_EQ(total_roads, total_roads_parser);

  for (pugi::xml_node road_node : roads_parser) {
    RoadId road_id = road_node.attribute("id").as_uint();

    for (pugi::xml_node lanes_node : road_node.children("lanes")) {

      // Check total Lane Sections
      auto lane_sections_parser = lanes_node.children("laneSection");
      auto total_lane_sections_parser = std::distance(lane_sections_parser.begin(), lane_sections_parser.end());
      auto total_lane_sections = map->GetMap().GetRoad(road_id).GetLaneSections().size();
      ASSERT_EQ(total_lane_sections, total_lane_sections_parser);

      for (pugi::xml_node lane_section_node : lane_sections_parser) {

        // Check total Lanes
        const double s = lane_section_node.attribute("s").as_double();
        auto lane_section = map->GetMap().GetRoad(road_id).GetLaneSectionsAt(s);
        size_t total_lanes = 0u;
        for (auto it = lane_section.begin(); it != lane_section.end(); ++it) {
          total_lanes = it->GetLanes().size();
        }
        auto left_nodes = lane_section_node.child("left").children("lane");
        auto center_nodes = lane_section_node.child("center").children("lane");
        auto right_nodes = lane_section_node.child("right").children("lane");
        auto total_lanes_parser = std::distance(left_nodes.begin(), left_nodes.end());
        total_lanes_parser += std::distance(right_nodes.begin(), right_nodes.end());
        total_lanes_parser += std::distance(center_nodes.begin(), center_nodes.end());

        ASSERT_EQ(total_lanes, total_lanes_parser);


        auto total_road_mark = 0;
        auto total_road_mark_parser = 0;
        for (auto it = lane_section.begin(); it != lane_section.end(); ++it) {
          auto total_left = get_total_road_marks(left_nodes, *it);
          auto total_center = get_total_road_marks(center_nodes, *it);
          auto total_right = get_total_road_marks(right_nodes, *it);
          total_road_mark = total_left.first + total_center.first + total_right.first;
          total_road_mark_parser = total_left.first + total_center.first + total_right.first;
        }
        ASSERT_EQ(total_road_mark, total_road_mark_parser);
      }
    }
  }
}

// Junctions
static void test_junctions(const pugi::xml_document &xml, boost::optional<Map>& map) {
  pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

  // Check total number of junctions
  auto& junctions = map->GetMap().GetJunctions();
  auto total_junctions_parser = std::distance(open_drive_node.children("junction").begin(), open_drive_node.children("junction").end());

  ASSERT_EQ(junctions.size(), total_junctions_parser);

  for (pugi::xml_node junction_node : open_drive_node.children("junction")) {
    // Check total number of connections
    auto total_connections_parser = std::distance(junction_node.children("connection").begin(), junction_node.children("connection").end());

    JuncId junction_id = junction_node.attribute("id").as_int();
    auto& junction = junctions.find(junction_id)->second;

    auto& connections = junction.GetConnections();

    ASSERT_EQ(connections.size(), total_connections_parser);

    for (pugi::xml_node connection_node : junction_node.children("connection")) {
      auto total_lane_links_parser = std::distance(connection_node.children("laneLink").begin(), connection_node.children("laneLink").end());

      ConId connection_id = connection_node.attribute("id").as_uint();
      auto& connection = connections.find(connection_id)->second;

    auto& lane_links = connection.lane_links;

    ASSERT_EQ(lane_links.size(), total_lane_links_parser);

    }
  }
}

static void test_road_links(boost::optional<Map>& map) {

  // process all roads, sections and lanes
  for (auto &road : map->GetMap().GetRoads()) {
    for (auto &section : road.second.GetLaneSections()) {
      for (auto &lane : section.GetLanes()) {
        // check all nexts
        for (auto link : lane.second.GetNextLanes()) {
          ASSERT_TRUE(link != nullptr);
        }
        // check all prevs
        for (auto link : lane.second.GetPreviousLanes()) {
          ASSERT_TRUE(link != nullptr);
        }
      }
    }
  }
}

TEST(road, parse_files) {
  for (const auto &file : util::OpenDrive::GetAvailableFiles()) {
    // std::cerr << file << std::endl;
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map);
    // print_roads(map, file);
  }
}

TEST(road, parse_road_links) {
  for (const auto &file : util::OpenDrive::GetAvailableFiles()) {
    // std::cerr << file << std::endl;
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map);
    test_road_links(map);
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

TEST(road, parse_road) {
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

TEST(road, parse_road_elevation) {
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map.has_value());

    const std::string full_path = BASE_PATH + file;
    pugi::xml_document xml;
    pugi::xml_parse_result result = xml.load_file( full_path.c_str());
    ASSERT_TRUE(result);

    test_road_elevation(xml, map);
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
  carla::ThreadPool pool;
  pool.AsyncRun();
  std::vector<std::future<void>> results;
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    carla::logging::log("Parsing", file);
    results.push_back(pool.Post([file]() {
      carla::StopWatch stop_watch;
      auto m = OpenDriveParser::Load(util::OpenDrive::Load(file));
      ASSERT_TRUE(m.has_value());
      auto &map = *m;
      const auto topology = map.GenerateTopology();
      ASSERT_FALSE(topology.empty());
      auto count = 0u;
      auto waypoints = map.GenerateWaypoints(0.5);
      ASSERT_FALSE(waypoints.empty());
      Random::Shuffle(waypoints);
      const auto number_of_waypoints_to_explore =
          std::min<size_t>(2000u, waypoints.size());
      for (auto i = 0u; i < number_of_waypoints_to_explore; ++i) {
        auto wp = waypoints[i];
        map.ComputeTransform(wp);
        if (i != 0u) {
          ASSERT_NE(wp, waypoints[0u]);
        }
        for (auto &&successor : map.GetSuccessors(wp)) {
          ASSERT_TRUE(
              successor.road_id != wp.road_id ||
              successor.section_id != wp.section_id ||
              successor.lane_id != wp.lane_id ||
              successor.s != wp.s);
        }
        auto origin = wp;
        for (auto j = 0u; j < 200u; ++j) {
          auto next_wps = map.GetNext(origin, Random::Uniform(0.0001, 150.0));
          if (next_wps.empty()) {
            break;
          }
          const auto number_of_next_wps_to_explore =
              std::min<size_t>(10u, next_wps.size());
          Random::Shuffle(next_wps);
          for (auto k = 0u; k < number_of_next_wps_to_explore; ++k) {
            auto next = next_wps[k];
            ++count;
            ASSERT_TRUE(
                next.road_id != wp.road_id ||
                next.section_id != wp.section_id ||
                next.lane_id != wp.lane_id ||
                next.s != wp.s);
            auto right = map.GetRight(next);
            if (right.has_value()) {
              ASSERT_EQ(right->road_id, next.road_id);
              ASSERT_EQ(right->section_id, next.section_id);
              ASSERT_NE(right->lane_id, next.lane_id);
              ASSERT_EQ(right->s, next.s);
            }
            auto left = map.GetLeft(next);
            if (left.has_value()) {
              ASSERT_EQ(left->road_id, next.road_id);
              ASSERT_EQ(left->section_id, next.section_id);
              ASSERT_NE(left->lane_id, next.lane_id);
              ASSERT_EQ(left->s, next.s);
            }
          }
          origin = next_wps[0u];
        }
      }
      ASSERT_GT(count, 0u);
      float seconds = 1e-3f * stop_watch.GetElapsedTime();
      carla::logging::log(file, "done in", seconds, "seconds.");
    }));
  }
  for (auto &result : results) {
    result.get();
  }
}

TEST(road, get_waypoint) {
  carla::ThreadPool pool;
  pool.AsyncRun();
  std::vector<std::future<void>> results;
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    carla::logging::log("Parsing", file);
    results.push_back(pool.Post([file]() {
      carla::StopWatch stop_watch;
      auto m = OpenDriveParser::Load(util::OpenDrive::Load(file));
      ASSERT_TRUE(m.has_value());
      auto &map = *m;
      for (auto i = 0u; i < 10'000u; ++i) {
        const auto location = Random::Location(-500.0f, 500.0f);
        auto owp = map.GetClosestWaypointOnRoad(location);
        ASSERT_TRUE(owp.has_value());
        auto &wp = *owp;
        for (auto &next : map.GetNext(wp, 0.5)) {
          ASSERT_TRUE(
              next.road_id != wp.road_id ||
              next.section_id != wp.section_id ||
              next.lane_id != wp.lane_id ||
              next.s != wp.s);
        }
        auto left = map.GetLeft(wp);
        if (left.has_value()) {
          ASSERT_EQ(left->road_id, wp.road_id);
          ASSERT_EQ(left->section_id, wp.section_id);
          ASSERT_NE(left->lane_id, wp.lane_id);
          ASSERT_EQ(left->s, wp.s);
        }
        auto right = map.GetRight(wp);
        if (right.has_value()) {
          ASSERT_EQ(right->road_id, wp.road_id);
          ASSERT_EQ(right->section_id, wp.section_id);
          ASSERT_NE(right->lane_id, wp.lane_id);
          ASSERT_EQ(right->s, wp.s);
        }
      }
      float seconds = 1e-3f * stop_watch.GetElapsedTime();
      carla::logging::log(file, "done in", seconds, "seconds.");
    }));
  }
  for (auto &result : results) {
    result.get();
  }
}
