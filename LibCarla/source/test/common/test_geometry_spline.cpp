// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/road/element/Geometry.h>

#include <array>
#include <cmath>

using carla::road::element::GeometryPoly3;
using carla::road::element::GeometryParamPoly3;

TEST(geometry_spline, poly3_linear_positions_and_extrapolation) {
  for (double slope : {-0.75, 0.0, 0.75}) {
    const GeometryPoly3 geometry(0.0, 10.0, 0.7, {1.0f, 2.0f, 3.0f},
        0.0, slope, 0.0, 0.0);
    const double knot = 0.3 * std::sqrt(1.0 + slope * slope);
    for (double s : {-0.5, 0.0, std::nextafter(knot, 0.0), knot,
         std::nextafter(knot, 1.0), 2.0, 9.75, 10.0, 10.5}) {
      const auto point = geometry.PosFromDist(s);
      const double tangent = 0.7 + std::atan(slope);
      EXPECT_NEAR(point.location.x, 1.0 + s * std::cos(tangent), 2e-6);
      EXPECT_NEAR(point.location.y, 2.0 + s * std::sin(tangent), 2e-6);
      EXPECT_FLOAT_EQ(point.location.z, 3.0f);
      EXPECT_NEAR(point.tangent, tangent, 1e-12);
    }
  }
  // This short, steep line has a single precomputed spline interval.
  const GeometryPoly3 short_line(0.0, 0.1, 0.0, {0.0f, 0.0f, 0.0f},
      0.0, 4.0, 0.0, 0.0);
  for (double s : {-0.1, 0.0, 0.05, 0.1, 0.2}) {
    const auto point = short_line.PosFromDist(s);
    EXPECT_NEAR(point.location.x, s / std::sqrt(17.0), 1e-7);
    EXPECT_NEAR(point.location.y, 4.0 * s / std::sqrt(17.0), 1e-7);
    EXPECT_NEAR(point.tangent, std::atan(4.0), 1e-12);
  }
}

TEST(geometry_spline, param_poly3_linear_parameter_ranges) {
  for (bool arc_length : {false, true}) {
    const double scale = arc_length ? 1.0 : 20.0;
    const GeometryParamPoly3 geometry(0.0, 20.0, -0.4,
        {1.0f, 2.0f, 3.0f}, 0.0, 0.8 * scale, 0.0, 0.0,
        0.0, -0.6 * scale, 0.0, 0.0, arc_length);
    for (double s : {-0.5, 0.0, 0.5, 7.5, 19.5, 20.0, 20.5}) {
      const auto point = geometry.PosFromDist(s);
      const double tangent = -0.4 + std::atan2(-0.6, 0.8);
      EXPECT_NEAR(point.location.x, 1.0 + s * std::cos(tangent), 2e-6);
      EXPECT_NEAR(point.location.y, 2.0 + s * std::sin(tangent), 2e-6);
      EXPECT_FLOAT_EQ(point.location.z, 3.0f);
      EXPECT_NEAR(point.tangent, tangent, 1e-12);
    }
  }
}

TEST(geometry_spline, poly3_nonlinear_reference) {
  const GeometryPoly3 geometry(0.0, 20.0, -0.4, {1.0f, 2.0f, 3.0f},
      0.3, -0.2, 0.03, 0.002);
  // Integrate sqrt(1 + v'(u)^2), invert arc length, then evaluate the
  // polynomial and its tangent. References use 30-digit quadrature;
  // tolerances allow the existing 0.3 m chord approximation.
  const std::array<std::array<double, 4>, 6> cases{{
    {{0.0, 1.116825502692595, 2.276318298200866, -0.597395559849881}},
    {{0.1, 1.199665720882760, 2.220305166152639, -0.591673153970052}},
    {{1.25, 2.176537399880072, 1.614008782694711, -0.516298178103811}},
    {{4.5, 5.189246775909745, 0.428733721579907, -0.214039207695496}},
    {{10.0, 10.607899142488896, 0.817348835463612, 0.320897942060292}},
    {{20.0, 19.021320130776103, 6.105910871805508, 0.716943930473695}}
  }};
  for (const auto &reference : cases) {
    const auto point = geometry.PosFromDist(reference[0]);
    EXPECT_NEAR(point.location.x, reference[1], 3e-3);
    EXPECT_NEAR(point.location.y, reference[2], 3e-3);
    EXPECT_FLOAT_EQ(point.location.z, 3.0f);
    EXPECT_NEAR(point.tangent, reference[3], 3e-4);
  }
}

TEST(geometry_spline, param_poly3_nonlinear_reference) {
  constexpr double length = 20.8045763886910174;
  const GeometryParamPoly3 geometry(0.0, length, 0.6,
      {1.0f, 2.0f, 3.0f}, 0.0, 20.0, 0.0, 0.0,
      0.0, 0.0, 5.0, 0.0, false);
  // Independent arc-length quadrature for u(p)=20p, v(p)=5p^2.
  // Tolerances retain the existing sampled-polynomial approximation.
  const std::array<std::array<double, 4>, 5> cases{{
    {{0.0, 1.0, 2.0, 0.6}},
    {{0.1, 1.082462895356724, 2.056567355259752, 0.602499992187546}},
    {{4.0, 4.183320866234991, 4.419350756004036, 0.699504696326026}},
    {{13.0, 10.398564505941871, 10.905703142133522, 0.909374269415791}},
    {{length, 14.683499931218389, 17.419527542449099, 1.063647609000806}}
  }};
  for (const auto &reference : cases) {
    const auto point = geometry.PosFromDist(reference[0]);
    EXPECT_NEAR(point.location.x, reference[1], 1e-3);
    EXPECT_NEAR(point.location.y, reference[2], 1e-3);
    EXPECT_FLOAT_EQ(point.location.z, 3.0f);
    EXPECT_NEAR(point.tangent, reference[3], 2e-5);
  }
}
