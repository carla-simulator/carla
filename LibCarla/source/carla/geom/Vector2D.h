// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
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

    Vector2D(float ix, float iy)
      : x(ix),
        y(iy) {}

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    double ExactLength() const {
       return std::sqrt(double(x) * x + double(y) * y);
    }

    float SquaredLength() const {
      return x * x + y * y;
    }

    float Length() const {
       return std::sqrt(SquaredLength());
    }

    Vector2D MakeUnitVectorLengthInput(const double length, const float epsilon = 2.0f * std::numeric_limits<float>::epsilon()) const {
      if (length < epsilon) {
        return *this;
      }
      const double k = 1.0 / length;
      Vector2D result(float(x * k), float(y * k));
      return result;
    }

    Vector2D MakeUnitVector(const float epsilon = 2.0f * std::numeric_limits<float>::epsilon()) const  {
      const double length = ExactLength();
      return MakeUnitVectorLengthInput(length, epsilon);
    }

    std::pair<Vector2D, double> GetUnitVectorAndLength() const {
      const auto length = ExactLength();
      const auto unit_vector = MakeUnitVectorLengthInput(length);
      return std::make_pair(unit_vector, length);
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
