// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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

  struct Vector2D {

    static constexpr auto Dim = 2;

    // =========================================================================
    // -- Public data members --------------------------------------------------
    // =========================================================================

    float x = 0.0f;

    float y = 0.0f;

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    Vector2D() = default;

    Vector2D(float ix, float iy)
      : x(ix),
        y(iy) {}

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    float SquaredLength() const {
      return x * x + y * y;
    }

    float Length() const {
       return std::sqrt(SquaredLength());
    }

    Vector2D MakeUnitVector() const {
      const float len = Length();
      DEVELOPMENT_ASSERT(len > 2.0f * std::numeric_limits<float>::epsilon());
      const float k = 1.0f / len;
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

    Vector2D &operator*=(float rhs) {
      x *= rhs;
      y *= rhs;
      return *this;
    }

    friend Vector2D operator*(Vector2D lhs, float rhs) {
      lhs *= rhs;
      return lhs;
    }

    friend Vector2D operator*(float lhs, Vector2D rhs) {
      rhs *= lhs;
      return rhs;
    }

    Vector2D &operator/=(float rhs) {
      x /= rhs;
      y /= rhs;
      return *this;
    }

    friend Vector2D operator/(Vector2D lhs, float rhs) {
      lhs /= rhs;
      return lhs;
    }

    friend Vector2D operator/(float lhs, Vector2D rhs) {
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

    /// Return a Vector2D converted from centimeters to meters.
    Vector2D ToMeters() const {
      return *this * 1e-2f;
    }

    /// Return a Vector2D converted from meters to centimeters.
    Vector2D ToCentimeters() const {
      return *this * 1e2f;
    }

    FVector2D ToFVector2D() const {
      return FVector2D{x, y};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(x, y)
  };

} // namespace geom
} // namespace carla
