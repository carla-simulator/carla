// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>

#include <sstream>
#include <string>
#include <tuple>

namespace {

// A short middle road has a longer neighbor at each end. A traversal that
// overshoots its boundary returns a waypoint on a different road instead of
// relying on a missing neighbor (and potentially dereferencing an empty vector).
std::string LinkedRoads(const std::string &rule) {
  std::ostringstream xml;
  xml << R"(<OpenDRIVE><header revMajor="1" revMinor="4" name="lane-start" version="1" date="" north="0" south="0" east="0" west="0"/>)";
  for (int road = 0; road < 3; ++road) {
    const int length = road == 1 ? 10 : 20;
    const int x = road == 0 ? 0 : (road == 1 ? 20 : 30);
    xml << "<road name=\"fixture\" length=\"" << length << "\" id=\""
        << road << "\" junction=\"-1\" rule=\"" << rule << "\"><link>";
    if (road > 0) {
      xml << "<predecessor elementType=\"road\" elementId=\"" << road - 1 << "\" contactPoint=\"end\"/>";
    }
    if (road < 2) {
      xml << "<successor elementType=\"road\" elementId=\"" << road + 1 << "\" contactPoint=\"start\"/>";
    }
    xml << "</link><planView><geometry s=\"0\" x=\"" << x << "\" y=\"0\" hdg=\"0\" length=\"" << length << "\"><line/></geometry></planView>"
        << R"(<elevationProfile><elevation s="0" a="0" b="0" c="0" d="0"/></elevationProfile><lanes><laneSection s="0">)";
    const auto add_lane = [&](const char *side, int id) {
      xml << '<' << side << "><lane id=\"" << id << "\" type=\"driving\" level=\"false\"><link>";
      if (road > 0) {
        xml << "<predecessor id=\"" << id << "\"/>";
      }
      if (road < 2) {
        xml << "<successor id=\"" << id << "\"/>";
      }
      xml << R"(</link><width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane></)" << side << '>';
    };
    add_lane("left", 1);
    xml << R"(<center><lane id="0" type="none" level="false"/></center>)";
    add_lane("right", -1);
    xml << "</laneSection></lanes></road>";
  }
  xml << "</OpenDRIVE>";
  return xml.str();
}

class WaypointUntilLaneStart : public ::testing::TestWithParam<std::tuple<std::string, int, double>> {};

TEST_P(WaypointUntilLaneStart, stays_on_the_original_road_and_reaches_its_start) {
  const auto &rule = std::get<0>(GetParam());
  const auto lane = std::get<1>(GetParam());
  const auto step = std::get<2>(GetParam());
  auto map = carla::MakeShared<carla::client::Map>("lane-start", LinkedRoads(rule));
  const bool positive = (rule == "RHT") == (lane < 0);
  const auto start = map->GetWaypointXODR(1u, lane, positive ? 9.9f : 0.1f);
  ASSERT_NE(start, nullptr);
  ASSERT_EQ(start->IsPositiveDirection(), positive);
  const auto waypoints = start->GetPreviousUntilLaneStart(step);
  ASSERT_FALSE(waypoints.empty());
  double previous_s = start->GetDistance();
  for (const auto &waypoint : waypoints) {
    EXPECT_EQ(waypoint->GetRoadId(), 1u);
    EXPECT_EQ(waypoint->GetLaneId(), lane);
    if (positive) {
      EXPECT_LT(waypoint->GetDistance(), previous_s);
    } else {
      EXPECT_GT(waypoint->GetDistance(), previous_s);
    }
    previous_s = waypoint->GetDistance();
  }
  EXPECT_NEAR(waypoints.back()->GetDistance(), positive ? 0.0 : 10.0, 1e-5);
}

INSTANTIATE_TEST_SUITE_P(
    both_traffic_rules_and_lane_signs,
    WaypointUntilLaneStart,
    ::testing::Combine(::testing::Values(std::string("RHT"), std::string("LHT")),
                       ::testing::Values(-1, 1), ::testing::Values(0.5, 20.0)));

TEST(waypoints, previous_until_lane_start_at_boundary_returns_no_waypoints) {
  for (const auto &rule : {std::string("RHT"), std::string("LHT")}) {
    auto map = carla::MakeShared<carla::client::Map>("lane-start", LinkedRoads(rule));
    const auto lane = rule == "RHT" ? -1 : 1;
    SCOPED_TRACE(rule + " lane " + std::to_string(lane));
    // s=0 is a valid GetWaypointXODR boundary; s=road_length is excluded.
    const auto start = map->GetWaypointXODR(1u, lane, 0.0f);
    ASSERT_NE(start, nullptr);
    EXPECT_TRUE(start->GetPreviousUntilLaneStart(0.5).empty());
  }
}

} // namespace
