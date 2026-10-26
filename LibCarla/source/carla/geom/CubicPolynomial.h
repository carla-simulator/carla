// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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
        _s(0.0),
        _origin(0.0) {}

    CubicPolynomial(
        const value_type &a,
        const value_type &b,
        const value_type &c,
        const value_type &d,
        const value_type &s) // lateral offset
      : _v{ {a, b, c, d} },
        _s(s),
        _origin(s) {}

    // =========================================================================
    // -- Getters --------------------------------------------------------------
    // =========================================================================

    // Preserve the expanded global-coordinate coefficient interface.
    value_type GetA() const {
      if (_origin == 0.0) {
        return _v[0];
      }
      return _v[0] - _v[1] * _origin + _v[2] * _origin * _origin -
          _v[3] * _origin * _origin * _origin;
    }

    value_type GetB() const {
      if (_origin == 0.0) {
        return _v[1];
      }
      return _v[1] - 2 * _v[2] * _origin + 3 * _v[3] * _origin * _origin;
    }

    value_type GetC() const {
      if (_origin == 0.0) {
        return _v[2];
      }
      return _v[2] - 3 * _v[3] * _origin;
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
      _v = {a, b, c, d};
      _s = s;
      _origin = s;
    }

    void Set(
        const value_type &a,
        const value_type &b,
        const value_type &c,
        const value_type &d) {
      _v = {a, b, c, d};
      _s = 0.0;
      _origin = 0.0;
    }

    // =========================================================================
    // -- Evaluate methods -----------------------------------------------------
    // =========================================================================

    /// Evaluates f(x) = a + bx + cx^2 + dx^3
    value_type Evaluate(const value_type &x) const {
      // Retain the authored local basis: expanded global coefficients lose
      // small widths/elevations when large station terms cancel near the
      // evaluation origin.
      const auto local_x = x - _origin;
      return _v[0] + local_x * (_v[1] + local_x * (_v[2] + local_x * _v[3]));
    }

    /// Evaluates the tangent using df/dx = b + 2cx + 3dx^2
    value_type Tangent(const value_type &x) const {
      const auto local_x = x - _origin;
      return _v[1] + local_x * (2 * _v[2] + local_x * 3 * _v[3]);
    }

    // =========================================================================
    // -- Arithmetic operators -------------------------------------------------
    // =========================================================================

    CubicPolynomial &operator+=(const CubicPolynomial &rhs) {
      // Use a nonconstant operand's local basis. A zero or constant accumulator
      // must not expand a shifted road profile back into global coordinates.
      // For two active profiles, the later station keeps their shared domain
      // close to the evaluation origin. GetS() retains the left operand's
      // authored station, independently of this numerical choice.
      const bool lhs_constant = _v[1] == 0.0 && _v[2] == 0.0 && _v[3] == 0.0;
      const bool rhs_constant = rhs._v[1] == 0.0 && rhs._v[2] == 0.0 && rhs._v[3] == 0.0;
      const auto origin = lhs_constant || (!rhs_constant && rhs._origin > _origin)
          ? rhs._origin : _origin;
      const auto lhs = Rebase(_v, origin - _origin);
      const auto other = Rebase(rhs._v, origin - rhs._origin);
      for (auto i = 0u; i < _v.size(); ++i) {
        _v[i] = lhs[i] + other[i];
      }
      _origin = origin;
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

    static std::array<value_type, 4> Rebase(
        const std::array<value_type, 4> &v, value_type shift) {
      if (shift == 0.0) {
        return v;
      }
      return {{v[0] + shift * (v[1] + shift * (v[2] + shift * v[3])),
               v[1] + shift * (2 * v[2] + shift * 3 * v[3]),
               v[2] + 3 * shift * v[3], v[3]}};
    }

    // Coefficients are stored in the local coordinate x - _origin.
    // a - elevation
    // b - slope
    // c - vertical curvature
    // d - curvature change
    std::array<value_type, 4> _v = {0.0, 0.0, 0.0, 0.0};

    // s - distance
    value_type _s;

    // Evaluation origin may change during arithmetic without changing GetS().
    value_type _origin = 0.0;
  };

} // namespace geom
} // namespace carla
