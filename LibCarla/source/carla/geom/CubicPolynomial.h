// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"

#include <array>

namespace carla {
namespace geom {

  /// Describes a Cubic Polynomial so:
  /// f(x) = a + bx + cx^2 + dx^3
  class CubicPolynomial {
  public:

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    CubicPolynomial() = default;

    CubicPolynomial(const CubicPolynomial &) = default;

    CubicPolynomial(
        const double &a,
        const double &b,
        const double &c,
        const double &d)
      : _v{ {a, b, c, d} } {}

    CubicPolynomial(
        const double &a,
        const double &b,
        const double &c,
        const double &d,
        const double &s) // lateral offset
      : _v{ {a - b * s + c * s * s - d * s * s * s,
             b - 2 * c * s + 3 * d * s * s,
             c - 3 * d * s,
             d} } {}

    // =========================================================================
    // -- Getters --------------------------------------------------------------
    // =========================================================================

    double GetA() const {
      return _v[0];
    }

    double GetB() const {
      return _v[1];
    }

    double GetC() const {
      return _v[2];
    }

    double GetD() const {
      return _v[3];
    }

    // =========================================================================
    // -- Evaluate methods -----------------------------------------------------
    // =========================================================================

    /// Evaluates f(x) = a + bx + cx^2 + dx^3
    double Evaluate(const double &x) const {
      // return _v[0] + _v[1] * (x) + _v[2] * (x * x) + _v[3] * (x * x * x);
      return _v[0] + x * (_v[1] + x * (_v[2] + x * _v[3]));
    }

    /// Evaluates the tangent using df/dx = b + 2cx + 3dx^2
    double Tangent(const double &x) const {
      return _v[1] + x * (2 * _v[2] + x * 3 * _v[3]);
    }

    // =========================================================================
    // -- Arithmetic operators -------------------------------------------------
    // =========================================================================

    CubicPolynomial &operator+=(const CubicPolynomial &rhs) {
      for (auto i = 0u; i < _v.size(); ++i) {
        _v[i] += rhs._v[i];
      }
      return *this;
    }

    friend CubicPolynomial operator+(CubicPolynomial lhs, const CubicPolynomial &rhs) {
      lhs += rhs;
      return lhs;
    }

    CubicPolynomial &operator*=(const double &rhs) {
      for (auto i = 0u; i < _v.size(); ++i) {
        _v[i] *= rhs;
      }
      return *this;
    }

    friend CubicPolynomial operator*(CubicPolynomial lhs, const double &rhs) {
      lhs *= rhs;
      return lhs;
    }

    friend CubicPolynomial operator*(const double &lhs, CubicPolynomial rhs) {
      rhs *= lhs;
      return rhs;
    }

  private:

    // =========================================================================
    // -- Private data members -------------------------------------------------
    // =========================================================================

    // a - elevation
    // b - slope
    // c - vertical curvature
    // d - curvature change
    std::array<double, 4> _v = {0.0, 0.0, 0.0, 0.0};

  };

} // namespace geom
} // namespace carla
