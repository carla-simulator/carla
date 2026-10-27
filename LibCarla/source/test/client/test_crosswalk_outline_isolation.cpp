// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/client/Map.h"

#include <string>
#include <vector>

namespace {

  const std::string rectangle =
      "<outline><cornerLocal u='-2' v='-1' z='0'/><cornerLocal u='-2' v='1' z='0'/>"
      "<cornerLocal u='2' v='1' z='0'/><cornerLocal u='2' v='-1' z='0'/>"
      "<cornerLocal u='-2' v='-1' z='0'/></outline>";

  std::string Crosswalk(const char *id, const char *station, const std::string &outline) {
    return std::string("<object id='") + id + "' name='crosswalk' type='crosswalk' s='" + station +
        "' t='0' zOffset='0' hdg='0' pitch='0' roll='0' orientation='none' width='2' length='4'>" +
        outline + "</object>";
  }

  std::string Road(const char *id, const char *x, const char *rule, const std::string &objects) {
    return std::string("<road id='") + id + "' length='100' junction='-1' rule='" + rule +
        "'><planView><geometry s='0' x='" + x + "' y='0' hdg='0' length='100'><line/>"
        "</geometry></planView><lanes><laneSection s='0'><center><lane id='0' type='none'/>"
        "</center><right><lane id='-1' type='driving'><width sOffset='0' a='3' b='0' c='0' d='0'/>"
        "</lane></right></laneSection></lanes><objects>" + objects + "</objects></road>";
  }

  std::vector<carla::geom::Location> GetCrosswalks(const std::string &roads) {
    const auto xml =
        "<OpenDRIVE><header revMajor='1' revMinor='4'><geoReference>"
        "+proj=tmerc +lat_0=0 +lon_0=0 +datum=WGS84</geoReference></header>" + roads + "</OpenDRIVE>";
    const auto map = carla::MakeShared<carla::client::Map>("crosswalk-outlines", xml);
    return map->GetAllCrosswalkZones();
  }

  class CrosswalkOutlineIsolation : public testing::TestWithParam<const char *> {};

  TEST_P(CrosswalkOutlineIsolation, following_outline_less_object_cannot_clone_the_polygon) {
    const auto points = GetCrosswalks(Road(
        "1", "0", GetParam(), Crosswalk("1", "20", rectangle) + Crosswalk("2", "60", "")));
    ASSERT_EQ(points.size(), 5u);
    for (const auto &point : points) {
      EXPECT_LT(point.x, 30.0f);
    }
    EXPECT_EQ(points.front(), points.back());
  }

  TEST_P(CrosswalkOutlineIsolation, another_road_cannot_inherit_previous_outline_points) {
    const auto points = GetCrosswalks(
        Road("1", "0", GetParam(), Crosswalk("1", "20", rectangle)) +
        Road("2", "200", GetParam(), Crosswalk("2", "20", "")));
    ASSERT_EQ(points.size(), 5u);
    for (const auto &point : points) {
      EXPECT_LT(point.x, 30.0f);
    }
  }

  TEST_P(CrosswalkOutlineIsolation, preceding_outline_less_object_control) {
    const auto points = GetCrosswalks(Road(
        "1", "0", GetParam(), Crosswalk("1", "20", "") + Crosswalk("2", "60", rectangle)));
    ASSERT_EQ(points.size(), 5u);
    for (const auto &point : points) {
      EXPECT_GT(point.x, 50.0f);
    }
  }

  TEST_P(CrosswalkOutlineIsolation, explicit_empty_outline_control) {
    const auto points = GetCrosswalks(Road(
        "1", "0", GetParam(), Crosswalk("1", "20", rectangle) + Crosswalk("2", "60", "<outline/>")));
    EXPECT_EQ(points.size(), 5u);
  }

  TEST_P(CrosswalkOutlineIsolation, distinct_defined_outlines_control) {
    const std::string triangle =
        "<outline><cornerLocal u='-1' v='-1' z='0'/><cornerLocal u='1' v='-1' z='0'/>"
        "<cornerLocal u='0' v='1' z='0'/><cornerLocal u='-1' v='-1' z='0'/></outline>";
    const auto points = GetCrosswalks(Road(
        "1", "0", GetParam(), Crosswalk("1", "20", rectangle) + Crosswalk("2", "60", triangle)));
    ASSERT_EQ(points.size(), 9u);
    EXPECT_EQ(points[0], points[4]);
    EXPECT_EQ(points[5], points[8]);
  }

  INSTANTIATE_TEST_SUITE_P(traffic_rules, CrosswalkOutlineIsolation, testing::Values("RHT", "LHT"));

} // namespace
