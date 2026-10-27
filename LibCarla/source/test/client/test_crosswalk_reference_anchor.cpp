// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>

#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>

#include "test.h"

namespace {
  using Parameters = std::tuple<bool, std::string, double, double, double, double>;

  std::string CrosswalkMap(const Parameters &parameters) {
    const auto &[arc, rule, offset, slope, heading, t] = parameters;
    std::ostringstream xml;
    xml << std::setprecision(17)
        << "<OpenDRIVE><header revMajor=\"1\" revMinor=\"4\"><geoReference>"
           "+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m"
           "</geoReference></header><road id=\"1\" length=\"100\" junction=\"-1\" rule=\""
        << rule << "\"><planView><geometry s=\"0\" x=\"10\" y=\"20\" hdg=\"" << heading
        << "\" length=\"100\">" << (arc ? "<arc curvature=\".02\"/>" : "<line/>")
        << "</geometry></planView><elevationProfile><elevation s=\"0\" a=\"2\" b=\"0\" c=\"0\" "
           "d=\"0\"/>"
           "</elevationProfile><lanes><laneOffset s=\"0\" a=\""
        << offset << "\" b=\"" << slope
        << "\" c=\"0\" d=\"0\"/><laneSection s=\"0\"><left><lane id=\"1\" type=\"driving\">"
           "<width sOffset=\"0\" a=\"3.5\" b=\"0\" c=\"0\" d=\"0\"/></lane></left>"
           "<center><lane id=\"0\" type=\"none\"/></center><right><lane id=\"-1\" type=\"driving\">"
           "<width sOffset=\"0\" a=\"3.5\" b=\"0\" c=\"0\" "
           "d=\"0\"/></lane></right></laneSection></lanes>"
           "<objects><object id=\"1\" name=\"crosswalk\" type=\"crosswalk\" s=\"30\" t=\""
        << t
        << "\" hdg=\"0\" zOffset=\"0\" orientation=\"none\" length=\"4\" width=\"2\">"
           "<outline><cornerLocal u=\"-2\" v=\"-1\" z=\"0\"/><cornerLocal u=\"-2\" v=\"1\" "
           "z=\"0\"/>"
           "<cornerLocal u=\"2\" v=\"1\" z=\"0\"/><cornerLocal u=\"2\" v=\"-1\" z=\"0\"/>"
           "<cornerLocal u=\"-2\" v=\"-1\" "
           "z=\"0\"/></outline></object></objects></road></OpenDRIVE>";
    return xml.str();
  }

  class CrosswalkReferenceAnchor : public testing::TestWithParam<Parameters> {};

  TEST_P(CrosswalkReferenceAnchor, object_anchor_and_axes_ignore_the_lane_offset_profile) {
    const auto &[arc, rule, offset, slope, heading, t] = GetParam();
    (void)rule;
    (void)offset;
    (void)slope;
    const auto map = carla::opendrive::OpenDriveParser::Load(CrosswalkMap(GetParam()));
    ASSERT_TRUE(map.has_value());
    const auto points = map->GetAllCrosswalkZones();
    ASSERT_EQ(points.size(), 5u);
    constexpr double station = 30.;
    const double local_x = arc ? std::sin(.02 * station) / .02 : station;
    const double local_y = arc ? (1. - std::cos(.02 * station)) / .02 : 0.;
    const double reference_x = 10. + local_x * std::cos(heading) - local_y * std::sin(heading);
    const double reference_y = 20. + local_x * std::sin(heading) + local_y * std::cos(heading);
    const double road_heading = heading + (arc ? .02 * station : 0.);
    const double anchor_x = reference_x - t * std::sin(road_heading);
    const double anchor_y = -reference_y - t * std::cos(road_heading);
    const double center_x = (points[0].x + points[1].x + points[2].x + points[3].x) / 4.;
    const double center_y = (points[0].y + points[1].y + points[2].y + points[3].y) / 4.;
    EXPECT_NEAR(center_x, anchor_x, 1e-4);
    EXPECT_NEAR(center_y, anchor_y, 1e-4);
    // Preserve the existing one-metre u padding. The symmetric outline makes
    // this reference-anchor oracle independent of the separate #9173 local-v sign correction.
    std::array<bool, 4> used{};
    const std::array<std::pair<double, double>, 4> corners = {
        {{-3., -1.}, {-3., 1.}, {3., 1.}, {3., -1.}}};
    for (size_t i = 0; i < 4u; ++i) {
      bool matched = false;
      for (size_t j = 0; j < 4u; ++j) {
        const auto [u, v] = corners[j];
        const double expected_x =
            anchor_x + u * std::cos(road_heading) - v * std::sin(road_heading);
        const double expected_y =
            anchor_y - u * std::sin(road_heading) - v * std::cos(road_heading);
        if (!used[j] && std::hypot(points[i].x - expected_x, points[i].y - expected_y) < 1e-4) {
          used[j] = true;
          matched = true;
          break;
        }
      }
      EXPECT_TRUE(matched);
      EXPECT_FLOAT_EQ(points[i].z, 2.f);
    }
    EXPECT_EQ(points.front(), points.back());
  }

  INSTANTIATE_TEST_SUITE_P(
      valid_reference_lines_rules_offsets_and_object_positions, CrosswalkReferenceAnchor,
      testing::Combine(testing::Bool(), testing::Values(std::string("RHT"), std::string("LHT")),
                       testing::Values(-4., 0., 4.), testing::Values(0., .04),
                       testing::Values(0., .5, -1.2), testing::Values(-5., 0., 5.)));
}  // namespace
