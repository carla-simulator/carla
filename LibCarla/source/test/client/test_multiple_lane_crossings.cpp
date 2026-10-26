// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>

#include <algorithm>
#include <sstream>
#include <tuple>
#include <vector>

#include "test.h"

namespace {
using Mark = carla::road::element::LaneMarking;

std::string MultiLaneMap(const std::string &rule) {
  std::ostringstream xml;
  xml << R"(<OpenDRIVE><header revMajor="1" revMinor="4"/><road id="1" length="10" junction="-1" rule=")"
      << rule
      << R"("><planView><geometry s="0" x="0" y="0" hdg="0" length="10"><line/></geometry></planView><lanes><laneSection s="0">)";
  const auto lane = [&](int id) {
    xml << "<lane id=\"" << id << "\" type=\"" << (id == 0 ? "none" : "driving") << "\">";
    if (id != 0) {
      xml << R"(<width sOffset="0" a="3.5" b="0" c="0" d="0"/>)";
    }
    const char *colors[] = {"white", "yellow", "blue", "green"};
    xml << R"(<roadMark sOffset="0" type="solid" color=")" << colors[std::abs(id)]
        << R"(" laneChange="both"/></lane>)";
  };
  xml << "<left>";
  for (int id : {3, 2, 1}) {
    lane(id);
  }
  xml << "</left><center>";
  lane(0);
  xml << "</center><right>";
  for (int id : {-1, -2, -3}) {
    lane(id);
  }
  xml << "</right></laneSection></lanes></road></OpenDRIVE>";
  return xml.str();
}

using Params = std::tuple<std::string, std::pair<int, int>>;
class MultipleLaneCrossing : public ::testing::TestWithParam<Params> {};

TEST_P(MultipleLaneCrossing, all_physical_boundaries_are_returned_in_crossing_order) {
  const auto &[rule, lanes] = GetParam();
  auto map = carla::MakeShared<carla::client::Map>("multi-lane", MultiLaneMap(rule));
  const auto origin = map->GetWaypointXODR(1u, lanes.first, 5.0);
  const auto destination = map->GetWaypointXODR(1u, lanes.second, 5.0);
  ASSERT_NE(origin, nullptr);
  ASSERT_NE(destination, nullptr);
  const auto result = map->CalculateCrossedLanes(origin->GetTransform().location,
                                                 destination->GetTransform().location);
  // This oracle enumerates the known physical positions of the five internal
  // boundaries in the six-lane fixture, independently of lane-ID stepping.
  const double y0 = origin->GetTransform().location.y;
  const double y1 = destination->GetTransform().location.y;
  const std::vector<std::pair<double, Mark::Color>> boundaries{{-7., Mark::Color::Blue},
                                                               {-3.5, Mark::Color::Yellow},
                                                               {0., Mark::Color::White},
                                                               {3.5, Mark::Color::Yellow},
                                                               {7., Mark::Color::Blue}};
  std::vector<Mark::Color> expected;
  for (const auto &[y, color] : boundaries) {
    if (y > std::min(y0, y1) && y < std::max(y0, y1)) {
      expected.push_back(color);
    }
  }
  if (y1 < y0) {
    std::reverse(expected.begin(), expected.end());
  }
  ASSERT_EQ(result.size(), expected.size());
  for (size_t index = 0; index < expected.size(); ++index) {
    EXPECT_EQ(result[index].color, expected[index]);
    EXPECT_EQ(result[index].type, Mark::Type::Solid);
  }
}

INSTANTIATE_TEST_SUITE_P(
    traffic_rules_and_both_crossing_directions, MultipleLaneCrossing,
    ::testing::Combine(::testing::Values(std::string("RHT"), std::string("LHT")),
                       ::testing::Values(std::make_pair(1, 3), std::make_pair(3, 1),
                                         std::make_pair(-1, -3), std::make_pair(-3, -1),
                                         std::make_pair(1, -2), std::make_pair(-2, 1),
                                         std::make_pair(3, -3), std::make_pair(-3, 3))));

TEST(multiple_lane_crossing_controls, adjacent_and_stationary_rht_moves) {
  auto map = carla::MakeShared<carla::client::Map>("controls", MultiLaneMap("RHT"));
  const auto a = map->GetWaypointXODR(1u, -1, 5.0)->GetTransform().location;
  const auto b = map->GetWaypointXODR(1u, -2, 5.0)->GetTransform().location;
  const auto result = map->CalculateCrossedLanes(a, b);
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(result[0].color, Mark::Color::Yellow);
  EXPECT_TRUE(map->CalculateCrossedLanes(a, a).empty());
}
}  // namespace

namespace {
using OutsideParams = std::tuple<std::string, int, int, bool>;
class MultipleLaneRoadEdge : public ::testing::TestWithParam<OutsideParams> {};

TEST_P(MultipleLaneRoadEdge, entry_and_exit_include_all_internal_marks_and_the_edge) {
  const auto &[rule, inside_lane, outside_side, entering] = GetParam();
  auto map = carla::MakeShared<carla::client::Map>("road-edge", MultiLaneMap(rule));
  const auto inside = map->GetWaypointXODR(1u, inside_lane, 5.0)->GetTransform().location;
  const carla::geom::Location outside(5.f, -12.f * outside_side, 0.f);
  const auto origin = entering ? outside : inside;
  const auto destination = entering ? inside : outside;
  const auto result = map->CalculateCrossedLanes(origin, destination);
  const std::vector<std::pair<double, Mark::Color>> boundaries{
      {-10.5, Mark::Color::Green}, {-7., Mark::Color::Blue},   {-3.5, Mark::Color::Yellow},
      {0., Mark::Color::White},    {3.5, Mark::Color::Yellow}, {7., Mark::Color::Blue},
      {10.5, Mark::Color::Green}};
  std::vector<Mark::Color> expected;
  for (const auto &[y, color] : boundaries) {
    if (y > std::min(origin.y, destination.y) && y < std::max(origin.y, destination.y)) {
      expected.push_back(color);
    }
  }
  if (destination.y < origin.y) {
    std::reverse(expected.begin(), expected.end());
  }
  ASSERT_EQ(result.size(), expected.size());
  for (size_t index = 0; index < expected.size(); ++index) {
    EXPECT_EQ(result[index].color, expected[index]);
  }
}

INSTANTIATE_TEST_SUITE_P(both_rules_inner_lanes_outer_edges_and_directions, MultipleLaneRoadEdge,
                         ::testing::Combine(::testing::Values(std::string("RHT"),
                                                              std::string("LHT")),
                                            ::testing::Values(-2, -1, 1, 2),
                                            ::testing::Values(-1, 1), ::testing::Bool()));

TEST(multiple_lane_crossing_controls, both_offroad_endpoints_keep_existing_contract) {
  for (const auto &rule : {std::string("RHT"), std::string("LHT")}) {
    auto map = carla::MakeShared<carla::client::Map>("offroad", MultiLaneMap(rule));
    EXPECT_TRUE(map->CalculateCrossedLanes({5.f, -12.f, 0.f}, {5.f, 12.f, 0.f}).empty());
  }
}
}  // namespace
