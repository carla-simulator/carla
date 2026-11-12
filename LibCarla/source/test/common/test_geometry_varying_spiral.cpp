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

  struct SpiralCase {
    double length;
    double start_curvature;
    double end_curvature;
    double distance;
    double heading;
    carla::geom::Location start;
  };

  class VaryingSpiralTest : public testing::TestWithParam<SpiralCase> {};

} // namespace

TEST_P(VaryingSpiralTest, matches_integrated_tangent) {
  const auto &test = GetParam();
  ASSERT_NE(test.start_curvature, test.end_curvature);
  const carla::road::element::GeometrySpiral spiral(0.0, test.length,
      test.heading, test.start, test.start_curvature, test.end_curvature);
  const auto result = spiral.PosFromDist(test.distance);

  const long double distance = std::max(0.0,
      std::min(test.distance, test.length));
  const long double rate =
      (test.end_curvature - test.start_curvature) / test.length;
  const auto angle = [&](long double s) {
    return test.heading + s * (test.start_curvature + 0.5L * rate * s);
  };
  // Independent composite Simpson integration of the unit tangent.
  // Long double keeps the oracle independent of the Fresnel subtraction.
  constexpr int intervals = 4096;
  const long double step = distance / intervals;
  long double x = 0.0L;
  long double y = 0.0L;
  for (int i = 0; i <= intervals; ++i) {
    const int weight = (i == 0 || i == intervals) ? 1 : (i % 2 ? 4 : 2);
    const long double phase = angle(i * step);
    x += weight * std::cos(phase);
    y += weight * std::sin(phase);
  }
  x *= step / 3.0L;
  y *= step / 3.0L;
  const double expected_x = test.start.x + x;
  const double expected_y = test.start.y + y;
  const double tolerance = distance < 1e-6L ? 1e-17 : 2e-6;
  EXPECT_NEAR(result.location.x, expected_x, tolerance);
  EXPECT_NEAR(result.location.y, expected_y, tolerance);
  EXPECT_FLOAT_EQ(result.location.z, test.start.z);
  EXPECT_NEAR(result.tangent, static_cast<double>(angle(distance)), 2e-12);
}

INSTANTIATE_TEST_SUITE_P(Geometry, VaryingSpiralTest, testing::Values(
    SpiralCase{20.0, 0.01, 0.01000000000001, 10.0, 0.7, {3.25f, -8.5f, 1.0f}},
    SpiralCase{20.0, 0.01, 0.00999999999999, 10.0, -0.7, {-3.25f, 8.5f, 1.0f}},
    SpiralCase{20.0, -0.01, -0.00999999999999, 10.0, 0.7, {3.25f, -8.5f, 1.0f}},
    SpiralCase{20.0, -0.01, -0.01000000000001, 10.0, -0.7, {-3.25f, 8.5f, 1.0f}},
    SpiralCase{50.0, 0.5, 0.50000000001, 30.0, 0.7, {3.25f, -8.5f, 1.0f}},
    SpiralCase{50.0, -0.5, -0.50000000001, 30.0, -0.7, {-3.25f, 8.5f, 1.0f}},
    SpiralCase{100.0, 0.001, 0.002, 1e-10, 0.7, {0.0f, 0.0f, 1.0f}},
    SpiralCase{20.0, 0.01, 0.010001, 10.0, 0.7, {3.25f, -8.5f, 1.0f}},
    SpiralCase{20.0, 0.01, 0.01000001, 10.0, 0.7, {3.25f, -8.5f, 1.0f}},
    SpiralCase{20.0, 0.01, 0.0100000001, 10.0, 0.7, {3.25f, -8.5f, 1.0f}},
    SpiralCase{20.0, 0.0, 0.01, 10.0, 0.7, {3.25f, -8.5f, 1.0f}},
    SpiralCase{20.0, -0.1, 0.2, 10.0, -0.7, {-3.25f, 8.5f, 1.0f}},
    SpiralCase{20.0, 0.1, 0.2, 10.0, 0.7, {3.25f, -8.5f, 1.0f}},
    SpiralCase{20.0, -0.1, -0.2, 10.0, -0.7, {-3.25f, 8.5f, 1.0f}},
    SpiralCase{20.0, 0.01, 0.01000000000001, -5.0, 0.7, {3.25f, -8.5f, 1.0f}},
    SpiralCase{20.0, 0.01, 0.01000000000001, 25.0, 0.7, {3.25f, -8.5f, 1.0f}}
));

TEST(geometry_spiral, large_phase_keeps_constant_cost_evaluation) {
  // These finite turn bounds exceed both the integration budget and, in
  // the second case, size_t's range. They must not become loop counts.
  for (const auto &[length, curvature] :
       {std::pair{1e6, 1e6}, std::pair{1e10, 1e30}}) {
    const carla::road::element::GeometrySpiral spiral(0.0, length, 0.7,
        {3.25f, -8.5f, 1.0f}, curvature, curvature * (1.0 + 1e-12));
    const auto result = spiral.PosFromDist(length);
    EXPECT_TRUE(std::isfinite(result.location.x));
    EXPECT_TRUE(std::isfinite(result.location.y));
    EXPECT_TRUE(std::isfinite(result.tangent));
    EXPECT_FLOAT_EQ(result.location.z, 1.0f);
  }
}
