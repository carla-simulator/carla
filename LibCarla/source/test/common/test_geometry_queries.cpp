// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"
#include <carla/road/element/Geometry.h>
#include <cmath>
#include <limits>

namespace {
namespace re = carla::road::element;
using carla::geom::Location;

double ClosedCurveLength() {
  constexpr int n = 10000;
  double sum = 0.0;
  for (int i = 0; i <= n; ++i) {
    const double p = static_cast<double>(i) / n;
    const double du = 3.0 - 18.0*p + 18.0*p*p;
    const double dv = 18.0*p - 27.0*p*p;
    sum += (i == 0 || i == n ? 1 : (i % 2 == 0 ? 2 : 4)) * std::hypot(du, dv);
  }
  return sum / (3.0*n);
}

void ExpectNearestAt(const re::Geometry &geometry, double s) {
  const auto point = geometry.PosFromDist(s).location;
  const auto nearest = geometry.DistanceTo(point);
  EXPECT_NEAR(nearest.first, s, 2e-3);
  EXPECT_NEAR(nearest.second, 0.0, 2e-4);
}

void ExpectNearDenseMinimum(const re::Geometry &geometry, const Location &query) {
  double reference = std::numeric_limits<double>::infinity();
  for (int i = 0; i <= 30000; ++i) {
    const auto p = geometry.PosFromDist(geometry.GetLength()*i/30000.0).location;
    reference = std::min(reference, std::hypot(double(query.x)-p.x, double(query.y)-p.y));
  }
  const auto result = geometry.DistanceTo(query);
  EXPECT_GE(result.first, 0.0f);
  EXPECT_LE(result.first, geometry.GetLength() + 1e-6);
  EXPECT_GE(result.second, 0.0f);
  EXPECT_LE(result.second, reference + 2.5e-4);
  const auto p = geometry.PosFromDist(result.first).location;
  EXPECT_NEAR(result.second, std::hypot(double(query.x)-p.x, double(query.y)-p.y), 2e-5);
}
}

TEST(geometry_queries, weak_arc_curvature_preserves_metre_scale_motion) {
  for (const double curvature : {1e-9, -1e-9, 1e-12, -1e-12}) {
    for (const double heading : {0.0, 0.7, -2.4}) {
      re::GeometryArc arc(0.0, 10.0, heading, Location{}, curvature);
      const auto p = arc.PosFromDist(1.0);
      EXPECT_NEAR(p.location.x, std::cos(heading), 1e-5);
      EXPECT_NEAR(p.location.y, std::sin(heading), 1e-5);
      EXPECT_NEAR(p.tangent, heading + curvature, 1e-14);
    }
  }
}

TEST(geometry_queries, ordinary_arc_and_distance_clamping_control) {
  re::GeometryArc arc(0.0, 10.0, 0.0, Location{3.f, 4.f, 5.f}, 0.1);
  const auto p = arc.PosFromDist(10.0);
  EXPECT_NEAR(p.location.x, 3.0 + 10.0*std::sin(1.0), 2e-6);
  EXPECT_NEAR(p.location.y, 4.0 + 10.0*(1.0-std::cos(1.0)), 2e-6);
  EXPECT_FLOAT_EQ(p.location.z, 5.f);
  EXPECT_EQ(arc.PosFromDist(-1.0).location, arc.PosFromDist(0.0).location);
  EXPECT_EQ(arc.PosFromDist(11.0).location, p.location);
}

TEST(geometry_queries, normalized_closed_polynomial_stops_at_parameter_endpoint) {
  re::GeometryParamPoly3 curve(0.0, ClosedCurveLength(), 0.4,
      Location{12.f, -8.f, 2.f}, 0., 3., -9., 6., 0., 0., 9., -9., false);
  const auto p = curve.PosFromDist(curve.GetLength());
  EXPECT_NEAR(p.location.x, 12.0, 1e-5);
  EXPECT_NEAR(p.location.y, -8.0, 1e-5);
  EXPECT_FLOAT_EQ(p.location.z, 2.f);
  EXPECT_NEAR(p.tangent, 0.4 + std::atan2(-9.0, 3.0), 1e-5);
}

TEST(geometry_queries, normalized_and_arc_length_straight_parameter_controls) {
  re::GeometryParamPoly3 normalized(0., 10., 0., Location{},
      0., 10., 0., 0., 0., 0., 0., 0., false);
  re::GeometryParamPoly3 arc_length(0., 10., 0., Location{},
      0., 1., 0., 0., 0., 0., 0., 0., true);
  for (const double s : {0., 1.25, 5., 10.}) {
    EXPECT_NEAR(normalized.PosFromDist(s).location.x, s, 1e-6);
    EXPECT_NEAR(arc_length.PosFromDist(s).location.x, s, 1e-6);
  }
}

TEST(geometry_queries, spiral_nearest_point_uses_the_curve) {
  re::GeometrySpiral curve(0., 20., 0.7, Location{10.f, -4.f, 0.f}, 0.02, 0.08);
  for (const double s : {0., 3.1, 8.25, 17.7, 20.}) ExpectNearestAt(curve, s);
}

TEST(geometry_queries, polynomial_nearest_point_uses_the_curve) {
  re::GeometryPoly3 curve(0., 20., 0.7, Location{10.f, -4.f, 0.f}, 0., 0., 0.05, 0.);
  for (const double s : {0., 3.1, 8.25, 17.7, 20.}) ExpectNearestAt(curve, s);
}

TEST(geometry_queries, parametric_polynomial_nearest_point_uses_the_curve) {
  re::GeometryParamPoly3 curve(0., 10.2606063043, -0.3, Location{10.f, -4.f, 0.f},
      0., 10., 0., 0., 0., 0., 2., 0., false);
  for (const double s : {0., 1.3, 4.25, 7.7, curve.GetLength()}) ExpectNearestAt(curve, s);
}

TEST(geometry_queries, nearest_distance_is_nonnegative_and_bounded_to_endpoints) {
  re::GeometryPoly3 curve(0., 10., 0., Location{10.f, -4.f, 0.f}, 0., 0., 0., 0.);
  const auto before = curve.DistanceTo(Location{7.f, 0.f, 0.f});
  const auto after = curve.DistanceTo(Location{23.f, 0.f, 0.f});
  EXPECT_NEAR(before.first, 0.0, 1e-4);
  EXPECT_NEAR(before.second, 5.0, 1e-4);
  EXPECT_NEAR(after.first, 10.0, 1e-4);
  EXPECT_NEAR(after.second, 5.0, 1e-4);
}

TEST(geometry_queries, line_and_arc_nearest_controls) {
  re::GeometryLine line(0., 10., 0., Location{});
  re::GeometryArc arc(0., 10., 0., Location{}, 0.1);
  ExpectNearestAt(line, 4.0);
  ExpectNearestAt(arc, 4.0);
}

TEST(geometry_queries, spiral_projection_searches_all_turns_in_world_coordinates) {
  re::GeometrySpiral curve(0., 30., -1.1, Location{12.f, -8.f, 5.f}, 0.3, 1.5);
  for (const Location q : {Location{12.f,-8.f,0.f}, Location{10.f,-7.f,15.f},
       Location{14.f,-10.f,0.f}, Location{9.f,-9.f,0.f}}) {
    ExpectNearDenseMinimum(curve, q);
  }
}

TEST(geometry_queries, parametric_projection_searches_the_entire_closed_curve) {
  re::GeometryParamPoly3 curve(0., ClosedCurveLength(), 0.7, Location{12.f,-8.f,0.f},
      0., 3., -9., 6., 0., 0., 9., -9., false);
  for (const Location q : {Location{12.f,-8.f,5.f}, Location{12.5f,-7.f,0.f},
       Location{11.f,-7.f,0.f}, Location{12.f,-6.f,0.f}}) {
    ExpectNearDenseMinimum(curve, q);
  }
}

TEST(geometry_queries, reversed_parametric_direction_keeps_positive_longitudinal_distance) {
  re::GeometryParamPoly3 curve(0., 10., 0.7, Location{12.f,-8.f,0.f},
      0., -10., 0., 0., 0., 0., 0., 0., false);
  ExpectNearestAt(curve, 6.25);
  ExpectNearDenseMinimum(curve, Location{7.f,-6.f,0.f});
}

TEST(geometry_queries, constant_curvature_flat_distance_and_zero_curvature_controls) {
  re::GeometrySpiral circle(0., 10., 0., Location{}, 0.1, 0.1);
  const auto result = circle.DistanceTo(Location{0.f,10.f,100.f});
  EXPECT_FLOAT_EQ(result.first, 0.f);
  EXPECT_FLOAT_EQ(result.second, 10.f);
  re::GeometrySpiral line(0., 10., 0., Location{}, 0., 0.);
  const auto nearest = line.DistanceTo(Location{4.f,3.f,100.f});
  EXPECT_FLOAT_EQ(nearest.first, 4.f);
  EXPECT_FLOAT_EQ(nearest.second, 3.f);
}
