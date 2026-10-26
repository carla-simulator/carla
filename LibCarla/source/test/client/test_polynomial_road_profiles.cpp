// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>

#include <string>

#include "test.h"

namespace {
auto ShiftedProfileMap(bool elevation) {
  const std::string profile = " a=\"3.5\" b=\"0.01\" c=\"0.0001\" d=\"0.001\"/>";
  return carla::opendrive::OpenDriveParser::Load(
      "<OpenDRIVE><header revMajor=\"1\" revMinor=\"4\"><geoReference>"
      "+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m"
      "</geoReference></header><road id=\"1\" junction=\"-1\" length=\"900010\">"
      "<planView><geometry s=\"0\" x=\"0\" y=\"0\" hdg=\"0\" length=\"900010\">"
      "<line/></geometry></planView><elevationProfile>"
      "<elevation s=\"0\" a=\"0\" b=\"0\" c=\"0\" d=\"0\"/>" +
      (elevation ? "<elevation s=\"900000\"" + profile : "") +
      "</elevationProfile><lanes><laneSection s=\"0\">"
      "<center><lane id=\"0\" type=\"none\"/></center></laneSection>"
      "<laneSection s=\"900000\"><center><lane id=\"0\" type=\"none\"/></center>"
      "<right><lane id=\"-1\" type=\"driving\" level=\"false\">" +
      (elevation ? "<width sOffset=\"0\" a=\"3.5\" b=\"0\" c=\"0\" d=\"0\"/>"
                 : "<width sOffset=\"0\"" + profile) +
      "</lane></right></laneSection></lanes></road></OpenDRIVE>");
}

double LocalValue(double s) {
  const double local = s - 900000.;
  return 3.5 + local * (.01 + local * (.0001 + local * .001));
}
}  // namespace

TEST(polynomial_road_profiles, shifted_lane_width_is_preserved_by_real_parser) {
  const auto map = ShiftedProfileMap(false);
  ASSERT_TRUE(map.has_value());
  for (double s : {900000.1, 900002., 900005.}) {
    const auto waypoint = map->GetWaypoint(1u, -1, s);
    ASSERT_TRUE(waypoint.has_value());
    // GetWaypoint accepts float s; compare the returned station, including its
    // existing float conversion, rather than the original double request.
    EXPECT_NEAR(map->GetLaneWidth(*waypoint), LocalValue(waypoint->s), 1e-10);
    EXPECT_NEAR(map->ComputeTransform(*waypoint).location.y, LocalValue(waypoint->s) / 2., 1e-5);
  }
}

TEST(polynomial_road_profiles, shifted_elevation_is_preserved_by_real_parser) {
  const auto map = ShiftedProfileMap(true);
  ASSERT_TRUE(map.has_value());
  for (double s : {900000.1, 900002., 900005.}) {
    const auto waypoint = map->GetWaypoint(1u, -1, s);
    ASSERT_TRUE(waypoint.has_value());
    EXPECT_NEAR(map->ComputeTransform(*waypoint).location.z, LocalValue(waypoint->s), 1e-5);
    EXPECT_DOUBLE_EQ(map->GetLaneWidth(*waypoint), 3.5);
  }
}
