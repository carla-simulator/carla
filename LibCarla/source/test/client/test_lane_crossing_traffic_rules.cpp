// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"
#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>
#include <sstream>
#include <tuple>

namespace {
using Mark = carla::road::element::LaneMarking;

std::string CrossingMap(const std::string &rule, bool markings = true) {
  std::ostringstream xml;
  xml << R"(<OpenDRIVE><header revMajor="1" revMinor="4"/><road id="1" name="crossings" length="10" junction="-1" rule=")"
      << rule << R"("><planView><geometry s="0" x="0" y="0" hdg="0" length="10"><line/></geometry></planView><lanes><laneSection s="0">)";
  const auto lane = [&](int id) {
    xml << "<lane id=\"" << id << "\" type=\"" << (id == 0 ? "none" : "driving") << "\">";
    if (id != 0) xml << R"(<width sOffset="0" a="3.5" b="0" c="0" d="0"/>)";
    const char *types[] = {"broken", "solid", "broken broken", "curb"};
    if (markings) xml << R"(<roadMark sOffset="0" type=")" << types[std::abs(id)]
                      << R"(" color="white" laneChange="both"/>)";
    xml << "</lane>";
  };
  xml << "<left>"; for (int id : {3,2,1}) lane(id); xml << "</left><center>";
  lane(0);
  xml << "</center><right>"; for (int id : {-1,-2,-3}) lane(id); xml << "</right>";
  xml << "</laneSection></lanes></road></OpenDRIVE>";
  return xml.str();
}

using Params = std::tuple<std::string, int, std::pair<int,int>>;
class LaneCrossingTrafficRules : public ::testing::TestWithParam<Params> {};

TEST_P(LaneCrossingTrafficRules, returns_the_physical_boundary_marking) {
  const auto &[rule, sign, crossing] = GetParam();
  auto map = carla::MakeShared<carla::client::Map>("crossings", CrossingMap(rule));
  const auto origin = map->GetWaypointXODR(1u, sign*crossing.first, 5.0);
  const auto destination = map->GetWaypointXODR(1u, sign*crossing.second, 5.0);
  ASSERT_NE(origin, nullptr); ASSERT_NE(destination, nullptr);
  const auto result = map->CalculateCrossedLanes(origin->GetTransform().location,
                                                destination->GetTransform().location);
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(result.front().type, std::min(crossing.first,crossing.second) == 1
                                 ? Mark::Type::Solid : Mark::Type::BrokenBroken);
}

INSTANTIATE_TEST_SUITE_P(
    traffic_rules_lane_sides_and_crossing_directions,
    LaneCrossingTrafficRules,
    ::testing::Combine(::testing::Values(std::string("RHT"),std::string("LHT")),
                       ::testing::Values(-1,1),
                       ::testing::Values(std::make_pair(1,2),std::make_pair(2,1),
                                         std::make_pair(2,3),std::make_pair(3,2))));

TEST(lane_crossing_controls, same_lane_stationary_and_missing_markings) {
  for (const auto &rule : {std::string("RHT"),std::string("LHT")}) {
    auto map = carla::MakeShared<carla::client::Map>("crossings", CrossingMap(rule));
    auto origin = map->GetWaypointXODR(1u,-1,3.0)->GetTransform().location;
    auto ahead = map->GetWaypointXODR(1u,-1,4.0)->GetTransform().location;
    EXPECT_TRUE(map->CalculateCrossedLanes(origin,origin).empty());
    EXPECT_TRUE(map->CalculateCrossedLanes(origin,ahead).empty());
    auto unmarked = carla::MakeShared<carla::client::Map>("unmarked",CrossingMap(rule,false));
    auto other = unmarked->GetWaypointXODR(1u,-2,3.0)->GetTransform().location;
    EXPECT_TRUE(unmarked->CalculateCrossedLanes(origin,other).empty());
  }
}

TEST(lane_crossing_boundaries, entering_and_leaving_the_outer_road_edge) {
  for (const auto &rule : {std::string("RHT"),std::string("LHT")}) {
    auto map = carla::MakeShared<carla::client::Map>("crossings",CrossingMap(rule));
    for (int sign : {-1,1}) {
      auto edge = map->GetWaypointXODR(1u,3*sign,5.0)->GetTransform().location;
      const carla::geom::Location outside{5.f,-12.f*sign,0.f};
      for (const auto &crossing : {std::make_pair(edge,outside),std::make_pair(outside,edge)}) {
        const auto result = map->CalculateCrossedLanes(crossing.first,crossing.second);
        ASSERT_EQ(result.size(),1u);
        EXPECT_EQ(result.front().type,Mark::Type::Curb);
      }
    }
  }
}

TEST(lane_crossing_boundaries, crossing_the_center_line_uses_the_center_marking) {
  for (const auto &rule : {std::string("RHT"),std::string("LHT")}) {
    auto map = carla::MakeShared<carla::client::Map>("crossings",CrossingMap(rule));
    for (int sign : {-1,1}) {
      const auto first = map->GetWaypointXODR(1u,sign,5.0)->GetTransform().location;
      const auto last = map->GetWaypointXODR(1u,-sign,5.0)->GetTransform().location;
      const auto result = map->CalculateCrossedLanes(first,last);
      ASSERT_EQ(result.size(),1u);
      EXPECT_EQ(result.front().type,Mark::Type::Broken);
    }
  }
}
}
