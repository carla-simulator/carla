// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cmath>
#include <limits>

namespace carla {
namespace geom {

  class Vector2D {
  public:

    // =========================================================================
    // -- Public data members --------------------------------------------------
    // =========================================================================

    float x = 0.0f;

    float y = 0.0f;

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    Vector2D() = default;

    Vector2D(const Vector2D &) = default;

    Vector2D(float ix, float iy)
      : x(ix),
        y(iy) {}

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    double SquaredLength() const {
      return x * x + y * y;
    }

    double Length() const {
       return std::sqrt(SquaredLength());
    }

    Vector2D MakeUnitVector() const {
      const double len = Length();
      DEBUG_ASSERT(len > std::numeric_limits<double>::epsilon());
      double k = 1.0 / len;
      return Vector2D(x * k, y * k);
    }

    // =========================================================================
    // -- Arithmetic operators -------------------------------------------------
    // =========================================================================

    Vector2D &operator+=(const Vector2D &rhs) {
      x += rhs.x;
      y += rhs.y;
      return *this;
    }

    friend Vector2D operator+(Vector2D lhs, const Vector2D &rhs) {
      lhs += rhs;
      return lhs;
    }

    Vector2D &operator-=(const Vector2D &rhs) {
      x -= rhs.x;
      y -= rhs.y;
      return *this;
    }

    friend Vector2D operator-(Vector2D lhs, const Vector2D &rhs) {
      lhs -= rhs;
      return lhs;
    }

    Vector2D &operator*=(const double &rhs) {
      x *= rhs;
      y *= rhs;
      return *this;
    }

    friend Vector2D operator*(Vector2D lhs, const double &rhs) {
      lhs *= rhs;
      return lhs;
    }

    friend Vector2D operator*(const double &lhs, Vector2D rhs) {
      rhs *= lhs;
      return rhs;
    }

    Vector2D &operator/=(const double &rhs) {
      x /= rhs;
      y /= rhs;
      return *this;
    }

    friend Vector2D operator/(Vector2D lhs, const double &rhs) {
      lhs /= rhs;
      return lhs;
    }

    friend Vector2D operator/(const double &lhs, Vector2D rhs) {
      rhs /= lhs;
      return rhs;
    }

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const Vector2D &rhs) const {
      return (x == rhs.x) && (y == rhs.y);
    }

    bool operator!=(const Vector2D &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Vector2D(const FVector2D &vector)
      : Vector2D(vector.X, vector.Y) {}

    Vector2D &ToMeters(void) { // from centimeters to meters.
       x *= 0.001f;
       y *= 0.001f;
       return *this;
    }

    Vector2D &ToCentimeters(void) { // from meters to centimeters.
       x *= 100.0f;
       y *= 100.0f;
       return *this;
    }

    operator FVector2D() const {
      return FVector2D{x, y};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4
  
    MSGPACK_DEFINE_ARRAY(x, y)
  };

} // namespace geom
} // namespace carla
