// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"

#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>

#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string Width(double a, double b = 0.0, double c = 0.0) {
  return "<width sOffset=\"0\" a=\"" + std::to_string(a) +
      "\" b=\"" + std::to_string(b) + "\" c=\"" + std::to_string(c) + "\" d=\"0\"/>";
}

std::string Road(int id, const std::string &width, const std::string &offset = "",
                 double y = 0.0, const std::string &outer_width = "", bool positive_lane = false) {
  const std::string side = positive_lane ? "left" : "right";
  const std::string lane_id = positive_lane ? "1" : "-1";
  const std::string outer_id = positive_lane ? "2" : "-2";
  std::string outer = outer_width.empty() ? "" :
      "<lane id=\"" + outer_id + "\" type=\"driving\" level=\"false\">" + outer_width + "</lane>";
  return "<road id=\"" + std::to_string(id) + "\" junction=\"-1\" length=\"100\">"
      "<planView><geometry s=\"0\" x=\"0\" y=\"" + std::to_string(y) +
      "\" hdg=\"0\" length=\"100\"><line/></geometry></planView>"
      "<elevationProfile><elevation s=\"0\" a=\"0\" b=\"0\" c=\"0\" d=\"0\"/></elevationProfile>"
      "<lanes>" + offset + "<laneSection s=\"0\"><center><lane id=\"0\" type=\"none\"/></center>"
      "<" + side + "><lane id=\"" + lane_id + "\" type=\"driving\" level=\"false\">" + width +
      "</lane>" + outer + "</" + side + "></laneSection></lanes></road>";
}

std::optional<carla::road::Map> Parse(const std::string &roads) {
  return carla::opendrive::OpenDriveParser::Load(
      "<OpenDRIVE><header revMajor=\"1\" revMinor=\"4\"><geoReference>"
      "+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m"
      "</geoReference></header>" + roads + "</OpenDRIVE>");
}

void ExpectCenterProjectsToItself(const carla::road::Map &map, float s, int lane_id = -1) {
  const auto waypoint = map.GetWaypoint(1, lane_id, s);
  ASSERT_TRUE(waypoint.has_value());
  const auto location = map.ComputeTransform(*waypoint).location;
  const auto projected = map.GetClosestWaypointOnRoad(location);
  ASSERT_TRUE(projected.has_value());
  EXPECT_EQ(projected->road_id, 1u);
  EXPECT_EQ(projected->lane_id, lane_id);
  EXPECT_NEAR(projected->s, s, 0.001);
  EXPECT_LT(map.ComputeTransform(*projected).location.Distance(location), 0.001);
}

} // namespace

TEST(lane_projection, linear_width_uses_reference_line_s) {
  auto map = Parse(Road(1, Width(3.5, 0.4)));
  ASSERT_TRUE(map.has_value());
  ExpectCenterProjectsToItself(*map, 40.0f);
}

TEST(lane_projection, linear_lane_offset_uses_reference_line_s) {
  auto map = Parse(Road(1, Width(3.5),
      "<laneOffset s=\"0\" a=\"0\" b=\"0.4\" c=\"0\" d=\"0\"/>"));
  ASSERT_TRUE(map.has_value());
  ExpectCenterProjectsToItself(*map, 40.0f);
}

TEST(lane_projection, positive_lane_uses_reference_line_s) {
  auto map = Parse(Road(1, Width(3.5, 0.4), "", 0.0, "", true));
  ASSERT_TRUE(map.has_value());
  ExpectCenterProjectsToItself(*map, 40.0f, 1);
}

TEST(lane_projection, town04_closest_waypoint_regression_4173) {
  std::ifstream input(LIBCARLA_TEST_CONTENT_FOLDER "/OpenDrive/Town04.xodr");
  ASSERT_TRUE(input.good()) << "Install the official opendrive-test-files tag0.1.4 fixture";
  const std::string source{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
  auto map = carla::opendrive::OpenDriveParser::Load(source);
  ASSERT_TRUE(map.has_value());
  for (const auto &location : {carla::geom::Location(-480.0f, 25.0f, 0.5f),
                              carla::geom::Location(-472.43f, 356.81f, 0.0f),
                              carla::geom::Location(-501.42f, 302.58f, 0.0f)}) {
    const auto projected = map->GetClosestWaypointOnRoad(location);
    ASSERT_TRUE(projected.has_value());
    const auto returned_distance = map->ComputeTransform(*projected).location.Distance(location);
    for (int step = -20; step <= 20; ++step) {
      auto candidate = *projected;
      candidate.s += 0.05 * step;
      const auto distance = map->ComputeTransform(candidate).location.Distance(location);
      EXPECT_LE(returned_distance, distance + 0.005f);
    }
  }
}

TEST(lane_projection, quadratic_width_is_not_straight) {
  auto map = Parse(Road(1, Width(3.5, 0.0, 0.02)));
  ASSERT_TRUE(map.has_value());
  const auto waypoint = map->GetWaypoint(1, -1, 50.0f);
  ASSERT_TRUE(waypoint.has_value());
  EXPECT_FALSE(map->GetLane(*waypoint).IsStraight());
}

TEST(lane_projection, inner_width_curves_outer_lane) {
  auto map = Parse(Road(1, Width(3.5, 0.0, 0.02), "", 0.0, Width(3.5)));
  ASSERT_TRUE(map.has_value());
  const auto waypoint = map->GetWaypoint(1, -2, 50.0f);
  ASSERT_TRUE(waypoint.has_value());
  EXPECT_FALSE(map->GetLane(*waypoint).IsStraight());
}

TEST(lane_projection, quadratic_road_offset_is_not_straight) {
  auto map = Parse(Road(1, Width(3.5),
      "<laneOffset s=\"0\" a=\"0\" b=\"0\" c=\"0.02\" d=\"0\"/>"));
  ASSERT_TRUE(map.has_value());
  const auto waypoint = map->GetWaypoint(1, -1, 50.0f);
  ASSERT_TRUE(waypoint.has_value());
  EXPECT_FALSE(map->GetLane(*waypoint).IsStraight());
}

TEST(lane_projection, curved_width_query_chooses_lane_at_the_query) {
  auto map = Parse(Road(1, Width(3.5, 0.0, 0.02)) + Road(2, Width(3.5), "", -27.0));
  ASSERT_TRUE(map.has_value());
  const auto waypoint = map->GetWaypoint(1, -1, 50.0f);
  ASSERT_TRUE(waypoint.has_value());
  const auto projected = map->GetClosestWaypointOnRoad(map->ComputeTransform(*waypoint).location);
  ASSERT_TRUE(projected.has_value());
  EXPECT_EQ(projected->road_id, 1u);
}

TEST(lane_projection, constant_width_line_control) {
  auto map = Parse(Road(1, Width(3.5)));
  ASSERT_TRUE(map.has_value());
  ExpectCenterProjectsToItself(*map, 40.0f);
  const auto waypoint = map->GetWaypoint(1, -1, 40.0f);
  ASSERT_TRUE(waypoint.has_value());
  EXPECT_TRUE(map->GetLane(*waypoint).IsStraight());
}

TEST(lane_projection, linear_width_stays_straight_control) {
  auto map = Parse(Road(1, Width(3.5, 0.4)));
  ASSERT_TRUE(map.has_value());
  const auto waypoint = map->GetWaypoint(1, -1, 40.0f);
  ASSERT_TRUE(waypoint.has_value());
  EXPECT_TRUE(map->GetLane(*waypoint).IsStraight());
}

TEST(lane_projection, line_projection_endpoint_control) {
  auto map = Parse(Road(1, Width(3.5)));
  ASSERT_TRUE(map.has_value());
  const auto before = map->GetClosestWaypointOnRoad({-5.0f, 1.75f, 0.0f});
  const auto after = map->GetClosestWaypointOnRoad({105.0f, 1.75f, 0.0f});
  ASSERT_TRUE(before.has_value());
  ASSERT_TRUE(after.has_value());
  EXPECT_NEAR(before->s, 0.0, 0.001);
  EXPECT_NEAR(after->s, 100.0, 0.001);
}
