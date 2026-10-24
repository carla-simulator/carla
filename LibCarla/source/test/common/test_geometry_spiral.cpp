// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/road/element/Geometry.h>

#include <array>
#include <cmath>

namespace {

using carla::geom::Location;
using namespace carla::road::element;

void ExpectPointNear(const DirectedPoint &actual, const DirectedPoint &expected) {
  ASSERT_TRUE(std::isfinite(actual.location.x));
  ASSERT_TRUE(std::isfinite(actual.location.y));
  ASSERT_TRUE(std::isfinite(actual.location.z));
  ASSERT_TRUE(std::isfinite(actual.tangent));
  EXPECT_NEAR(actual.location.x, expected.location.x, 2e-5);
  EXPECT_NEAR(actual.location.y, expected.location.y, 2e-5);
  EXPECT_NEAR(actual.location.z, expected.location.z, 1e-6);
  EXPECT_NEAR(actual.tangent, expected.tangent, 1e-12);
}

// Independent numerical integration of dx/ds = cos(heading), dy/ds = sin(heading).
// It also checks the ordinary clothoid path without copying its Fresnel algorithm.
DirectedPoint IntegrateSpiral(double length, double heading, const Location &start,
                             double start_curvature, double end_curvature, double distance) {
  constexpr int intervals = 10000;
  const double step = distance / intervals;
  const double curvature_rate = (end_curvature - start_curvature) / length;
  double x = 0.0;
  double y = 0.0;
  for (int i = 0; i <= intervals; ++i) {
    const double s = i * step;
    const double angle = heading + start_curvature * s + 0.5 * curvature_rate * s * s;
    const double weight = (i == 0 || i == intervals) ? 1.0 : (i % 2 == 0 ? 2.0 : 4.0);
    x += weight * std::cos(angle);
    y += weight * std::sin(angle);
  }
  return DirectedPoint(start.x + static_cast<float>(x * step / 3.0),
                       start.y + static_cast<float>(y * step / 3.0), start.z,
                       heading + start_curvature * distance +
                           0.5 * curvature_rate * distance * distance);
}

} // namespace

TEST(geometry_spiral, equal_zero_curvature_matches_line) {
  const Location start(7.0f, -4.0f, 3.0f);
  constexpr double length = 180.0;
  for (double heading : {0.0, 0.7, -1.2}) {
    for (double end_curvature : {0.0, -0.0}) {
      const GeometrySpiral spiral(5.0, length, heading, start, 0.0, end_curvature);
      const GeometryLine line(5.0, length, heading, start);
      for (double distance : {0.0, 1.0, 80.0, length}) {
        SCOPED_TRACE(::testing::Message() << heading << ", " << distance);
        ExpectPointNear(spiral.PosFromDist(distance), line.PosFromDist(distance));
      }
    }
  }
}

TEST(geometry_spiral, equal_nonzero_curvature_matches_arc) {
  const Location start(-10.0f, 12.0f, 2.0f);
  for (double curvature : {0.01, -0.02}) {
    const GeometrySpiral spiral(15.0, 60.0, 0.4, start, curvature, curvature);
    const GeometryArc arc(15.0, 60.0, 0.4, start, curvature);
    for (double distance : {0.0, 1.0, 23.0, 60.0}) {
      SCOPED_TRACE(::testing::Message() << curvature << ", " << distance);
      ExpectPointNear(spiral.PosFromDist(distance), arc.PosFromDist(distance));
    }
  }
}

TEST(geometry_spiral, equal_curvature_preserves_distance_clamping) {
  const Location start(3.0f, 4.0f, 5.0f);
  for (double curvature : {0.0, 0.03, -0.03}) {
    const GeometrySpiral spiral(0.0, 40.0, -0.6, start, curvature, curvature);
    ExpectPointNear(spiral.PosFromDist(-1.0),
                    IntegrateSpiral(40.0, -0.6, start, curvature, curvature, 0.0));
    ExpectPointNear(spiral.PosFromDist(50.0),
                    IntegrateSpiral(40.0, -0.6, start, curvature, curvature, 40.0));
  }
}

TEST(geometry_spiral, tiny_equal_curvature_has_a_finite_line_limit) {
  const Location start(2.0f, -5.0f, 8.0f);
  for (double curvature : {1e-20, -1e-20}) {
    const GeometrySpiral spiral(0.0, 180.0, 1.1, start, curvature, curvature);
    ExpectPointNear(spiral.PosFromDist(180.0),
                    IntegrateSpiral(180.0, 1.1, start, curvature, curvature, 180.0));
  }
}

TEST(geometry_spiral, varying_curvature_matches_numerical_integration) {
  const Location start(2.0f, -5.0f, 8.0f);
  const std::array<std::array<double, 2>, 4> curvatures{{
      {{0.0, 0.03}}, {{0.03, 0.0}}, {{-0.02, -0.04}}, {{-0.01, 0.02}}}};
  for (const auto &curvature : curvatures) {
    const GeometrySpiral spiral(8.0, 90.0, 0.7, start, curvature[0], curvature[1]);
    for (double distance : {0.0, 1.0, 40.0, 90.0}) {
      SCOPED_TRACE(::testing::Message() << curvature[0] << ", " << curvature[1] << ", " << distance);
      ExpectPointNear(spiral.PosFromDist(distance),
                      IntegrateSpiral(90.0, 0.7, start, curvature[0], curvature[1], distance));
    }
  }
}
