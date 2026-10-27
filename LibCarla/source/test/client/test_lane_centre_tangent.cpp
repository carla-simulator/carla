// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>

#include "test.h"

namespace {
  using Spiral = carla::road::element::GeometrySpiral;
  static_assert(std::is_same_v<decltype(&Spiral::GetCurveStart), double (Spiral::*)()>);
  static_assert(std::is_same_v<decltype(&Spiral::GetCurveEnd), double (Spiral::*)()>);
  using Parameters = std::tuple<int, std::string, int, bool, bool, bool, bool>;

  std::string TangentMap(const Parameters &parameters) {
    const auto &[kind, rule, lane, grade, width, offset, shape] = parameters;
    (void)lane;
    const std::array<std::string, 6> geometries = {
        "<line/>",
        "<arc curvature=\"0.02\"/>",
        "<spiral curvStart=\"0.005\" curvEnd=\"0.015\"/>",
        "<poly3 a=\"0\" b=\"0.1\" c=\"0.001\" d=\"0.00001\"/>",
        "<paramPoly3 aU=\"0\" bU=\"100\" cU=\"5\" dU=\"0\" "
        "aV=\"0\" bV=\"20\" cV=\"5\" dV=\"5\" pRange=\"normalized\"/>",
        "<paramPoly3 aU=\"0\" bU=\"-100\" cU=\"0\" dU=\"0\" "
        "aV=\"0\" bV=\"1\" cV=\"-1\" dV=\"0\" pRange=\"normalized\"/>"};
    double length = 100.0;
    if (kind >= 4) {
      // A valid normalized parametric curve's declared length must equal its
      // actual arc length. Compute fixture metadata independently of CARLA.
      constexpr int intervals = 10000;
      const auto speed = [kind](double p) {
        return kind == 4 ? std::hypot(100. + 10. * p, 20. + 10. * p + 15. * p * p)
                         : std::hypot(-100., 1. - 2. * p);
      };
      double sum = speed(0.) + speed(1.);
      for (int i = 1; i < intervals; ++i) {
        sum += (i % 2 ? 4. : 2.) * speed(static_cast<double>(i) / intervals);
      }
      length = sum / (3. * intervals);
    }
    std::ostringstream xml;
    xml << std::setprecision(17);
    xml << "<OpenDRIVE><header revMajor=\"1\" revMinor=\"4\"><geoReference>"
           "+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m"
           "</geoReference></header><road id=\"1\" length=\""
        << length << "\" junction=\"-1\" rule=\"" << rule
        << "\"><planView><geometry s=\"0\" x=\"10\" y=\"20\" hdg=\"0.4\" length=\"" << length << "\">"
        << geometries.at(static_cast<size_t>(kind)) << "</geometry></planView>"
        << "<elevationProfile><elevation s=\"0\" a=\"2\" b=\"" << (grade ? .2 : 0.) << "\" c=\""
        << (grade ? .001 : 0.) << "\" d=\"0\"/></elevationProfile>";
    if (shape) {
      xml << "<lateralProfile><shape s=\"0\" t=\"-40\" a=\"0\" b=\".03\" c=\".001\" d=\"0\"/>"
             "<shape s=\"100\" t=\"-40\" a=\"2\" b=\".04\" c=\".001\" d=\"0\"/>"
             "</lateralProfile>";
    }
    xml << "<lanes><laneOffset s=\"0\" a=\".5\" b=\"" << (offset ? .08 : 0.)
        << "\" c=\"0\" d=\"0\"/><laneSection s=\"0\"><left>";
    const auto add_lane = [&](int id) {
      xml << "<lane id=\"" << id
          << "\" type=\"driving\" level=\"false\"><width sOffset=\"0\" a=\"3.5\" b=\""
          << (width ? .05 : 0.) << "\" c=\"0\" d=\"0\"/></lane>";
    };
    add_lane(2);
    add_lane(1);
    xml << "</left><center><lane id=\"0\" type=\"none\"/></center><right>";
    add_lane(-1);
    add_lane(-2);
    xml << "</right></laneSection></lanes></road></OpenDRIVE>";
    return xml.str();
  }

  class LaneCentreTangent : public ::testing::TestWithParam<Parameters> {};

  TEST_P(LaneCentreTangent, forward_vector_follows_native_lane_centre_positions) {
    const auto &[kind, rule, lane_id, grade, width, offset, shape] = GetParam();
    (void)kind;
    (void)grade;
    (void)width;
    (void)offset;
    (void)shape;
    const auto map = carla::opendrive::OpenDriveParser::Load(TangentMap(GetParam()));
    ASSERT_TRUE(map.has_value());
    for (double station : {20., 50., 70.}) {
      const auto position = [&](double s) {
        const auto waypoint = map->GetWaypoint(1u, lane_id, static_cast<float>(s));
        EXPECT_TRUE(waypoint.has_value());
        return map->ComputeTransform(*waypoint).location;
      };
      const double h = .25;
      const auto p0 = position(station - 2 * h);
      const auto p1 = position(station - h);
      const auto p2 = position(station + h);
      const auto p3 = position(station + 2 * h);
      const double sign = (lane_id < 0) == (rule == "RHT") ? 1. : -1.;
      const auto derivative = [=](double a, double b, double c, double d) {
        return sign * (a - 8 * b + 8 * c - d) / (12 * h);
      };
      const double dx = derivative(p0.x, p1.x, p2.x, p3.x);
      const double dy = derivative(p0.y, p1.y, p2.y, p3.y);
      const double dz = derivative(p0.z, p1.z, p2.z, p3.z);
      const double length = std::hypot(dx, dy, dz);
      const auto waypoint = map->GetWaypoint(1u, lane_id, static_cast<float>(station));
      ASSERT_TRUE(waypoint.has_value());
      const auto forward = map->ComputeTransform(*waypoint).rotation.GetForwardVector();
      EXPECT_NEAR(forward.x, dx / length, 2e-4) << station;
      EXPECT_NEAR(forward.y, dy / length, 2e-4) << station;
      EXPECT_NEAR(forward.z, dz / length, 2e-4) << station;
    }
  }

  INSTANTIATE_TEST_SUITE_P(
      valid_geometries_rules_profiles_and_lane_signs, LaneCentreTangent,
      ::testing::Combine(::testing::Values(0, 1, 2, 3, 4, 5),
                         ::testing::Values(std::string("RHT"), std::string("LHT")),
                         ::testing::Values(-2, -1, 1, 2), ::testing::Bool(), ::testing::Bool(),
                         ::testing::Bool(), ::testing::Bool()));

  TEST(lane_tangent_controls, piecewise_surface_keeps_height_and_partial_slopes) {
    auto xml = TangentMap({0, "RHT", -1, false, false, false, false});
    const auto marker = xml.find("<lanes>");
    ASSERT_NE(marker, std::string::npos);
    xml.insert(marker,
               "<lateralProfile>"
               "<shape s=\"10\" t=\"-10\" a=\"1\" b=\".1\" c=\".01\" d=\"0\"/>"
               "<shape s=\"10\" t=\"0\" a=\"2\" b=\".2\" c=\".02\" d=\"0\"/>"
               "<shape s=\"60\" t=\"-10\" a=\"3\" b=\".3\" c=\".03\" d=\"0\"/>"
               "<shape s=\"60\" t=\"0\" a=\"4\" b=\".4\" c=\".04\" d=\"0\"/>"
               "</lateralProfile>");
    const auto map = carla::opendrive::OpenDriveParser::Load(xml);
    ASSERT_TRUE(map.has_value());
    for (int lane_id : {-2, -1, 1, 2}) {
      const double t = .5 - std::copysign((std::abs(lane_id) - .5) * 3.5, -lane_id);
      const double local = t < 0. ? t + 10. : t;
      const double first =
          t < 0. ? 1. + .1 * local + .01 * local * local : 2. + .2 * local + .02 * local * local;
      const double last =
          t < 0. ? 3. + .3 * local + .03 * local * local : 4. + .4 * local + .04 * local * local;
      for (float station : {5.f, 30.f, 80.f}) {
        const auto waypoint = map->GetWaypoint(1u, lane_id, station);
        ASSERT_TRUE(waypoint.has_value());
        const auto transform = map->ComputeTransform(*waypoint);
        const double fraction = std::clamp((station - 10.) / 50., 0., 1.);
        EXPECT_NEAR(transform.location.z, 2. + (1. - fraction) * first + fraction * last, 1e-5);
        const double dz = station > 10.f && station < 60.f ? (last - first) / 50. : 0.;
        const double direction = lane_id < 0 ? 1. : -1.;
        EXPECT_NEAR(transform.rotation.GetForwardVector().z, direction * dz / std::hypot(1., dz),
                    1e-5);
      }
    }
  }

  TEST(lane_tangent_controls, lane_endpoints_use_one_sided_position_tangents) {
    for (int kind : {0, 1, 2, 3}) {
      for (const std::string rule : {"RHT", "LHT"}) {
        const auto map = carla::opendrive::OpenDriveParser::Load(
            TangentMap({kind, rule, -2, true, true, true, true}));
        ASSERT_TRUE(map.has_value());
        for (int lane_id : {-2, 2}) {
          for (bool end : {false, true}) {
            SCOPED_TRACE(testing::Message() << "kind=" << kind << " rule=" << rule
                                            << " lane=" << lane_id << " end=" << end);
            const float station = end ? std::nextafter(100.f, 0.f) : 0.f;
            // A half-metre stencil keeps float position quantization below the
            // tangent tolerance at the far endpoint; the analytic profiles here
            // are smooth over its two-metre span.
            const double h = end ? -.5 : .5;
            std::array<carla::geom::Location, 5> points;
            for (size_t i = 0; i < points.size(); ++i) {
              const auto waypoint = map->GetWaypoint(
                  1u, lane_id, static_cast<float>(station + h * static_cast<double>(i)));
              ASSERT_TRUE(waypoint.has_value());
              points[i] = map->ComputeTransform(*waypoint).location;
            }
            const double direction = (lane_id < 0) == (rule == "RHT") ? 1. : -1.;
            const auto derivative = [=](double a, double b, double c, double d, double e) {
              return direction * (-25 * a + 48 * b - 36 * c + 16 * d - 3 * e) / (12 * h);
            };
            const double dx =
                derivative(points[0].x, points[1].x, points[2].x, points[3].x, points[4].x);
            const double dy =
                derivative(points[0].y, points[1].y, points[2].y, points[3].y, points[4].y);
            const double dz =
                derivative(points[0].z, points[1].z, points[2].z, points[3].z, points[4].z);
            const double length = std::hypot(dx, dy, dz);
            const auto waypoint = map->GetWaypoint(1u, lane_id, station);
            ASSERT_TRUE(waypoint.has_value());
            const auto forward = map->ComputeTransform(*waypoint).rotation.GetForwardVector();
            // Poly3 positions retain the existing 0.3 m chord approximation while
            // heading interpolates the smooth polynomial slope. Bound that
            // inherited endpoint discretization separately from analytic curves.
            const double tolerance = kind == 3 ? 1e-3 : 4e-4;
            EXPECT_NEAR(forward.x, dx / length, tolerance);
            EXPECT_NEAR(forward.y, dy / length, tolerance);
            EXPECT_NEAR(forward.z, dz / length, tolerance);
          }
        }
      }
    }
  }
}  // namespace
