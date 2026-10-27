// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
#include "carla/agents/navigation/GlobalRoutePlanner.h"
#include "carla/client/Lane.h"
#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <tuple>

namespace {

  std::shared_ptr<carla::client::Map> TopologyMap(
      bool left_hand, int lane, double length, double split = 0.0, bool driving = true) {
    std::ostringstream xml;
    xml.imbue(std::locale::classic());
    xml << std::setprecision(17);
    xml << "<OpenDRIVE><header revMajor=\"1\" revMinor=\"4\"><geoReference>"
           "+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m"
           "</geoReference></header><road id=\"1\" length=\"" << length
        << "\" junction=\"-1\" rule=\"" << (left_hand ? "LHT" : "RHT")
        << "\"><planView><geometry s=\"0\" x=\"0\" y=\"0\" hdg=\"0\" length=\""
        << length << "\"><line/></geometry></planView><lanes>";
    const auto section = [&](double station, bool first) {
      const auto side = lane < 0 ? "right" : "left";
      xml << "<laneSection s=\"" << station << "\"><center><lane id=\"0\" type=\"none\"/></center><"
          << side << "><lane id=\"" << lane << "\" type=\"" << (driving ? "driving" : "shoulder") << "\">";
      if (split != 0.0) {
        xml << "<link><" << (first ? "successor" : "predecessor") << " id=\"" << lane << "\"/></link>";
      }
      xml << "<width sOffset=\"0\" a=\"3.5\" b=\"0\" c=\"0\" d=\"0\"/></lane></"
          << side << "></laneSection>";
    };
    section(0.0, true);
    if (split != 0.0) {
      section(split, false);
    }
    xml << "</lanes></road></OpenDRIVE>";
    return std::make_shared<carla::client::Map>("terminal-topology", xml.str());
  }

  class TerminalTopology : public testing::TestWithParam<std::tuple<bool, int, double>> {};

  TEST_P(TerminalTopology, isolated_driving_lane_has_an_interior_terminal_pair) {
    const auto [left_hand, lane, length] = GetParam();
    const auto map = TopologyMap(left_hand, lane, length);
    const auto topology = map->GetTopology();
    ASSERT_EQ(topology.size(), 1u);
    const auto &start = topology.front().first;
    const auto &end = topology.front().second;
    ASSERT_NE(start, nullptr);
    ASSERT_NE(end, nullptr);
    EXPECT_EQ(start->GetRoadId(), 1u);
    EXPECT_EQ(end->GetRoadId(), 1u);
    EXPECT_EQ(start->GetSectionId(), end->GetSectionId());
    EXPECT_EQ(start->GetLaneId(), lane);
    EXPECT_EQ(end->GetLaneId(), lane);
    EXPECT_GT(end->GetDistance(), 0.0);
    EXPECT_LT(end->GetDistance(), length);
    const bool positive = (lane < 0) != left_hand;
    EXPECT_EQ(start->GetDistance() < end->GetDistance(), positive);
    EXPECT_TRUE(std::isfinite(end->GetTransform().location.x));
  }

  INSTANTIATE_TEST_SUITE_P(
      road_rules_signs_and_lengths, TerminalTopology,
      testing::Combine(testing::Bool(), testing::Values(-1, 1), testing::Values(100.0, 1000.0, 16384.25)));

  class SectionTopology : public testing::TestWithParam<std::tuple<bool, int>> {};

  TEST_P(SectionTopology, terminal_endpoint_retains_its_actual_lane_section) {
    const auto [left_hand, lane] = GetParam();
    const bool positive = (lane < 0) != left_hand;
    constexpr double split = 10000.25;
    constexpr double length = split + 50.5;
    const auto map = TopologyMap(left_hand, lane, length, split);
    const auto topology = map->GetTopology();
    ASSERT_EQ(topology.size(), 2u);
    bool found = false;
    for (const auto &pair : topology) {
      if (pair.first->GetSectionId() == pair.second->GetSectionId()) {
        found = true;
        EXPECT_EQ(pair.second->GetSectionId(), positive ? 1u : 0u);
        EXPECT_GT(pair.second->GetDistance(), positive ? split : 0.0);
        EXPECT_LT(pair.second->GetDistance(), positive ? length : split);
        EXPECT_EQ(pair.second->GetLaneId(), lane);
      }
    }
    EXPECT_TRUE(found);
  }

  INSTANTIATE_TEST_SUITE_P(road_rules_and_signs, SectionTopology,
                          testing::Combine(testing::Bool(), testing::Values(-1, 1)));

  TEST(terminal_topology, actual_route_planner_can_bootstrap_an_isolated_driving_lane) {
    for (const bool left_hand : {false, true}) {
      const int lane = left_hand ? 1 : -1;
      const auto map = TopologyMap(left_hand, lane, 100.0);
      const auto origin = map->GetWaypointXODR(1u, lane, 20.0f);
      const auto destination = map->GetWaypointXODR(1u, lane, 80.0f);
      ASSERT_NE(origin, nullptr);
      ASSERT_NE(destination, nullptr);
      ASSERT_EQ(origin->GetNext(60.0).size(), 1u);
      carla::agents::navigation::GlobalRoutePlanner planner(map, 2.0);
      const auto route = planner.TraceRoute(origin->GetTransform().location, destination->GetTransform().location);
      EXPECT_GT(route.size(), 10u) << "LHT=" << left_hand;
    }
  }

  TEST(terminal_topology, a_tapered_lane_ending_before_the_road_keeps_its_section) {
    for (const bool left_hand : {false, true}) {
      const int inner = left_hand ? 1 : -1;
      const int outer = 2 * inner;
      const auto side = left_hand ? "left" : "right";
      auto template_map = TopologyMap(left_hand, inner, 100.0, 30.5);
      auto xml = template_map->GetOpenDrive();
      const std::string closing = std::string("</") + side + "></laneSection>";
      const auto insertion = xml.find(closing);
      ASSERT_NE(insertion, std::string::npos);
      std::ostringstream lane;
      lane.imbue(std::locale::classic());
      lane << std::setprecision(17) << "<lane id=\"" << outer << "\" type=\"driving\">"
           << "<width sOffset=\"0\" a=\"3.5\" b=\"" << -3.5 / 30.5
           << "\" c=\"0\" d=\"0\"/></lane>";
      xml.insert(insertion, lane.str());
      const auto map = std::make_shared<carla::client::Map>("tapered-terminal", xml);
      bool found = false;
      for (const auto &pair : map->GetTopology()) {
        if (pair.first->GetLaneId() == outer) {
          found = true;
          EXPECT_EQ(pair.second->GetLaneId(), outer);
          EXPECT_EQ(pair.second->GetSectionId(), 0u);
          EXPECT_GT(pair.second->GetDistance(), 0.0);
          EXPECT_LT(pair.second->GetDistance(), 30.5);
        }
      }
      EXPECT_TRUE(found) << "LHT=" << left_hand;
    }
  }

  TEST(terminal_topology, nondriving_lane_control_remains_excluded) {
    EXPECT_TRUE(TopologyMap(false, -1, 100.0, 0.0, false)->GetTopology().empty());
  }

} // namespace
