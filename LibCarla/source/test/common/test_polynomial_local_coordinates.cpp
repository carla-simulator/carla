// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/geom/CubicPolynomial.h>

#include <array>
#include <cmath>
#include <limits>

#include "test.h"

namespace {
using carla::geom::CubicPolynomial;

double Value(const std::array<double, 4> &v, double origin, double x) {
  const long double u = static_cast<long double>(x) - origin;
  return static_cast<double>(v[0] + u * (v[1] + u * (v[2] + u * v[3])));
}
double Derivative(const std::array<double, 4> &v, double origin, double x) {
  const long double u = static_cast<long double>(x) - origin;
  return static_cast<double>(v[1] + u * (2.0L * v[2] + 3.0L * u * v[3]));
}

TEST(polynomial_local_coordinates, small_local_values_survive_large_station_offsets) {
  const std::array<double, 4> v{{3.5, .01, .0001, .001}};
  for (double origin : {10000., 1000000., 100000000., -1000000.}) {
    const CubicPolynomial polynomial(v[0], v[1], v[2], v[3], origin);
    for (double offset : {0., .001, .1, 2.0, 10.0}) {
      SCOPED_TRACE(::testing::Message() << "origin " << origin << " offset " << offset);
      const double x = origin + offset;
      EXPECT_NEAR(polynomial.Evaluate(x), Value(v, origin, x), 1e-11);
    }
  }
}

TEST(polynomial_local_coordinates, small_local_gradients_survive_large_station_offsets) {
  const std::array<double, 4> v{{3.5, .01, .0001, .001}};
  for (double origin : {10000., 1000000., 100000000., -1000000.}) {
    const CubicPolynomial polynomial(v[0], v[1], v[2], v[3], origin);
    for (double offset : {0., .001, .1, 2.0, 10.0}) {
      SCOPED_TRACE(::testing::Message() << "origin " << origin << " offset " << offset);
      const double x = origin + offset;
      EXPECT_NEAR(polynomial.Tangent(x), Derivative(v, origin, x), 1e-11);
    }
  }
}

TEST(polynomial_local_coordinates, shifted_setter_preserves_value_and_derivative) {
  CubicPolynomial polynomial(1., 2., 3., 4.);
  const std::array<double, 4> v{{3.5, .01, .0001, .001}};
  polynomial.Set(v[0], v[1], v[2], v[3], 1000000.);
  EXPECT_NEAR(polynomial.Evaluate(1000000.1), Value(v, 1000000., 1000000.1), 1e-11);
  EXPECT_NEAR(polynomial.Tangent(1000000.1), Derivative(v, 1000000., 1000000.1), 1e-11);
  polynomial.Set(1., 2., 3., 4.);
  EXPECT_EQ(polynomial.GetS(), 0.);
  EXPECT_DOUBLE_EQ(polynomial.Evaluate(2.), 49.);
}

TEST(polynomial_local_coordinates, arithmetic_keeps_nearby_shifted_polynomials_stable) {
  const std::array<double, 4> a{{3.5, .01, .0001, .001}}, b{{2., -.02, .0002, -.0003}};
  const double first = 1000000., second = 1000005., x = 1000000.1;
  const CubicPolynomial left(a[0], a[1], a[2], a[3], first), right(b[0], b[1], b[2], b[3], second);
  const auto sum = left + right;
  EXPECT_NEAR(sum.Evaluate(x), Value(a, first, x) + Value(b, second, x), 1e-11);
  EXPECT_NEAR(sum.Tangent(x), Derivative(a, first, x) + Derivative(b, second, x), 1e-11);
  const auto scaled = left * 2.5;
  EXPECT_NEAR(scaled.Evaluate(x), 2.5 * Value(a, first, x), 1e-11);
  auto doubled = left;
  doubled += doubled;
  EXPECT_NEAR(doubled.Evaluate(x), 2. * Value(a, first, x), 1e-11);
}

TEST(polynomial_local_coordinates, global_coefficient_getters_and_ordinary_controls) {
  const CubicPolynomial shifted(1., 2., 3., 4., 5.);
  EXPECT_DOUBLE_EQ(shifted.GetA(), -434.);
  EXPECT_DOUBLE_EQ(shifted.GetB(), 272.);
  EXPECT_DOUBLE_EQ(shifted.GetC(), -57.);
  EXPECT_DOUBLE_EQ(shifted.GetD(), 4.);
  EXPECT_DOUBLE_EQ(shifted.GetS(), 5.);
  const CubicPolynomial local(1., 2., 3., 4.);
  EXPECT_DOUBLE_EQ(local.Evaluate(2.), 49.);
  EXPECT_DOUBLE_EQ(local.Tangent(2.), 62.);
  EXPECT_DOUBLE_EQ(shifted.Evaluate(7.), 49.);
  EXPECT_DOUBLE_EQ(shifted.Tangent(7.), 62.);
}

TEST(polynomial_local_coordinates, zero_accumulator_adopts_a_stable_evaluation_basis) {
  const std::array<double, 4> v{{3.5, .01, .0001, .001}};
  for (double origin : {1000000., -1000000.}) {
    CubicPolynomial accumulator(0., 0., 0., 0.);
    const CubicPolynomial profile(v[0], v[1], v[2], v[3], origin);
    accumulator += profile;
    const double x = origin + .1;
    EXPECT_NEAR(accumulator.Evaluate(x), Value(v, origin, x), 1e-11);
    EXPECT_NEAR(accumulator.Tangent(x), Derivative(v, origin, x), 1e-11);
    EXPECT_DOUBLE_EQ(accumulator.GetS(), 0.);
    EXPECT_DOUBLE_EQ(profile.GetS(), origin);
  }
}

TEST(polynomial_local_coordinates, constant_accumulator_does_not_expand_the_shifted_cubic) {
  const std::array<double, 4> v{{3.5, .01, .0001, .001}};
  for (double origin : {1000000., -1000000.}) {
    CubicPolynomial accumulator(2., 0., 0., 0.);
    const CubicPolynomial profile(v[0], v[1], v[2], v[3], origin);
    accumulator += profile;
    const double x = origin + .1;
    EXPECT_NEAR(accumulator.Evaluate(x), 2. + Value(v, origin, x), 1e-11);
    EXPECT_NEAR(accumulator.Tangent(x), Derivative(v, origin, x), 1e-11);
    EXPECT_DOUBLE_EQ(accumulator.GetS(), 0.);
  }
}
}  // namespace

TEST(polynomial_local_coordinates, default_zero_accumulator_control) {
  carla::geom::CubicPolynomial accumulator;
  const carla::geom::CubicPolynomial profile(3.5, .01, .0001, .001, 1000000.);
  EXPECT_DOUBLE_EQ(accumulator.Evaluate(1000000.), 0.);
  accumulator += profile;
  EXPECT_NEAR(accumulator.Evaluate(1000000.1), profile.Evaluate(1000000.1), 1e-11);
  EXPECT_NEAR(accumulator.Tangent(1000000.1), profile.Tangent(1000000.1), 1e-11);
}

TEST(polynomial_local_coordinates, unshifted_large_coefficients_keep_finite_getters) {
  const double large = std::numeric_limits<double>::max() / 2.;
  const carla::geom::CubicPolynomial polynomial(1., 2., 3., large);
  EXPECT_DOUBLE_EQ(polynomial.GetA(), 1.);
  EXPECT_DOUBLE_EQ(polynomial.GetB(), 2.);
  EXPECT_DOUBLE_EQ(polynomial.GetC(), 3.);
  EXPECT_DOUBLE_EQ(polynomial.GetD(), large);
  const auto doubled = polynomial + polynomial;
  EXPECT_DOUBLE_EQ(doubled.GetA(), 2.);
  EXPECT_DOUBLE_EQ(doubled.GetB(), 4.);
  EXPECT_DOUBLE_EQ(doubled.GetC(), 6.);
  EXPECT_DOUBLE_EQ(doubled.GetD(), large * 2.);
}
