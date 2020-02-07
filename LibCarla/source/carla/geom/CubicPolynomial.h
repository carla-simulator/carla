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

    using value_type = double;

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    CubicPolynomial() = default;

    CubicPolynomial(const CubicPolynomial &) = default;

    CubicPolynomial(
        const value_type &a,
        const value_type &b,
        const value_type &c,
        const value_type &d)
      : _v{ {a, b, c, d} },
        _s(0.0) {}

    CubicPolynomial(
        const value_type &a,
        const value_type &b,
        const value_type &c,
        const value_type &d,
        const value_type &s) // lateral offset
      : _v{ {a - b * s + c * s * s - d * s * s * s,
             b - 2 * c * s + 3 * d * s * s,
             c - 3 * d * s,
             d} },
        _s(s) {}

    // =========================================================================
    // -- Getters --------------------------------------------------------------
    // =========================================================================

    value_type GetA() const {
      return _v[0];
    }

    value_type GetB() const {
      return _v[1];
    }

    value_type GetC() const {
      return _v[2];
    }

    value_type GetD() const {
      return _v[3];
    }

    value_type GetS() const {
      return _s;
    }

    // =========================================================================
    // -- Set ------------------------------------------------------------------
    // =========================================================================

    void Set(
        const value_type &a,
        const value_type &b,
        const value_type &c,
        const value_type &d,
        const value_type &s) { // lateral offset
      _v = { a - b * s + c * s * s - d * s * s * s,
             b - 2 * c * s + 3 * d * s * s,
             c - 3 * d * s,
             d };
      _s = s;
    }

    void Set(
        const value_type &a,
        const value_type &b,
        const value_type &c,
        const value_type &d) {
      _v = {a, b, c, d};
      _s = 0.0;
    }

    // =========================================================================
    // -- Evaluate methods -----------------------------------------------------
    // =========================================================================

    /// Evaluates f(x) = a + bx + cx^2 + dx^3
    value_type Evaluate(const value_type &x) const {
      // return _v[0] + _v[1] * (x) + _v[2] * (x * x) + _v[3] * (x * x * x);
      return _v[0] + x * (_v[1] + x * (_v[2] + x * _v[3]));
    }

    /// Evaluates the tangent using df/dx = b + 2cx + 3dx^2
    value_type Tangent(const value_type &x) const {
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

    CubicPolynomial &operator*=(const value_type &rhs) {
      for (auto i = 0u; i < _v.size(); ++i) {
        _v[i] *= rhs;
      }
      return *this;
    }

    friend CubicPolynomial operator*(CubicPolynomial lhs, const value_type &rhs) {
      lhs *= rhs;
      return lhs;
    }

    friend CubicPolynomial operator*(const value_type &lhs, CubicPolynomial rhs) {
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
    std::array<value_type, 4> _v = {0.0, 0.0, 0.0, 0.0};

    // s - distance
    value_type _s;
  };

} // namespace geom
} // namespace carla
