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

// Road length100; the driving lane may end at0.5 or99.5, independently of
// its reference road. Connected sections exercise coarse steps crossing them.
std::string LaneSections(const std::string &rule, int lane_id, bool early_end,
                         bool connected_sections) {
  const bool positive = (rule == "RHT") == (lane_id < 0);
  std::ostringstream xml;
  xml << R"(<OpenDRIVE><header revMajor="1" revMinor="4"/><road id="1" name="lane-end" length="100" junction="-1" rule=")"
      << rule << R"("><planView><geometry s="0" x="0" y="0" hdg="0" length="100"><line/></geometry></planView>)"
      << R"(<elevationProfile><elevation s="0" a="0" b="0" c="0" d="0"/></elevationProfile><lanes>)";
  const auto section = [&](double s, bool driving, bool predecessor = false, bool successor = false) {
    xml << "<laneSection s=\"" << s << R"(\"><center><lane id="0" type="none"/></center>)";
    if (driving) {
      const char *side = lane_id < 0 ? "right" : "left";
      xml << '<' << side << "><lane id=\"" << lane_id << R"(\" type="driving" level="false"><link>)";
      if (predecessor) {
        xml << "<predecessor id=\"" << lane_id << "\"/>";
      }
      if (successor) {
        xml << "<successor id=\"" << lane_id << "\"/>";
      }
      xml << R"(</link><width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane></)" << side << '>';
    }
    xml << "</laneSection>";
  };
  if (early_end && !positive) {
    section(0.0, false);
    section(99.5, true, false, connected_sections);
    if (connected_sections) {
      section(99.75, true, true, false);
    }
  } else {
    section(0.0, true, false, connected_sections);
    if (connected_sections) {
      section(early_end ? 0.25 : 40.0, true, true, false);
    }
    if (early_end) {
      section(0.5, false);
    }
  }
  xml << "</lanes></road></OpenDRIVE>";
  return xml.str();
}

class WaypointUntilLaneEnd : public ::testing::TestWithParam<std::tuple<std::string, int, double>> {
protected:
  void Check(bool early_end, bool connected_sections) const {
    const auto &rule = std::get<0>(GetParam());
    const int lane = std::get<1>(GetParam());
    const double spacing = std::get<2>(GetParam());
    const bool positive = (rule == "RHT") == (lane < 0);
    auto map = carla::MakeShared<carla::client::Map>(
        "lane-end", LaneSections(rule, lane, early_end, connected_sections));
    const double start_s = positive ? 0.1 : 99.9;
    const auto start = map->GetWaypointXODR(1u, lane, start_s);
    ASSERT_NE(start, nullptr);
    ASSERT_EQ(start->IsPositiveDirection(), positive);
    const auto waypoints = start->GetNextUntilLaneEnd(spacing);
    ASSERT_FALSE(waypoints.empty());
    double previous_s = start->GetDistance();
    for (const auto &waypoint : waypoints) {
      EXPECT_EQ(waypoint->GetRoadId(), 1u);
      EXPECT_EQ(waypoint->GetLaneId(), lane);
      if (positive) {
        EXPECT_GT(waypoint->GetDistance(), previous_s);
      } else {
        EXPECT_LT(waypoint->GetDistance(), previous_s);
      }
      previous_s = waypoint->GetDistance();
    }
    const double expected = early_end ? (positive ? 0.5 : 99.5) : (positive ? 100.0 : 0.0);
    EXPECT_NEAR(waypoints.back()->GetDistance(), expected, 1e-6);
    EXPECT_EQ(waypoints.back()->GetSectionId(),
              early_end ? (positive ? (connected_sections ? 1u : 0u) : 1u)
                        : (positive && connected_sections ? 1u : 0u));
  }
};

TEST_P(WaypointUntilLaneEnd, reaches_early_lane_termination) {
  Check(true, false);
}

TEST_P(WaypointUntilLaneEnd, follows_connected_sections_before_early_termination) {
  Check(true, true);
}

TEST_P(WaypointUntilLaneEnd, whole_road_control) {
  Check(false, false);
}

TEST_P(WaypointUntilLaneEnd, connected_whole_road_control) {
  Check(false, true);
}

INSTANTIATE_TEST_SUITE_P(
    traffic_rules_lane_signs_and_spacing,
    WaypointUntilLaneEnd,
    ::testing::Combine(::testing::Values(std::string("RHT"), std::string("LHT")),
                       ::testing::Values(-1, 1), ::testing::Values(0.1, 200.0)));

TEST(waypoints, next_until_lane_end_at_interior_boundary_returns_no_waypoints) {
  for (const auto &rule : {std::string("RHT"), std::string("LHT")}) {
    const int lane = rule == "RHT" ? 1 : -1;
    auto map = carla::MakeShared<carla::client::Map>("lane-end", LaneSections(rule, lane, true, false));
    const auto start = map->GetWaypointXODR(1u, lane, 99.5);
    ASSERT_NE(start, nullptr);
    EXPECT_TRUE(start->GetNextUntilLaneEnd(0.1).empty());
  }
}

} // namespace
