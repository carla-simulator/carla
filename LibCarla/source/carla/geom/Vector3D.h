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

  struct Vector3D {

    static constexpr auto Dim = 3;

    // =========================================================================
    // -- Public data members --------------------------------------------------
    // =========================================================================

    float x = 0.0f;

    float y = 0.0f;

    float z = 0.0f;

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    Vector3D() = default;

    constexpr Vector3D(float ix, float iy, float iz)
      : x(ix),
        y(iy),
        z(iz) {
    }

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    constexpr float SquaredLength() const {
      return x * x + y * y + z * z;
    }

    inline float Length() const {
       return std::sqrt(SquaredLength());
    }

    constexpr float SquaredLength2D() const {
      return x * x + y * y;
    }

    inline float Length2D() const {
      return std::sqrt(SquaredLength2D());
    }

    inline Vector3D Abs() const {
       return Vector3D(abs(x), abs(y), abs(z));
    }

    inline Vector3D MakeUnitVector() const {
      const float length = Length();
      DEVELOPMENT_ASSERT(length > 2.0f * std::numeric_limits<float>::epsilon());
      const float k = 1.0f / length;
      return Vector3D(x * k, y * k, z * k);
    }

    inline Vector3D MakeSafeUnitVector(const float epsilon) const {
      const float length = Length();
      const float k = (length > std::max(epsilon, 0.0f)) ? (1.0f / length) : 1.0f;
      return Vector3D(x * k, y * k, z * k);
    }

    // =========================================================================
    // -- Arithmetic operators -------------------------------------------------
    // =========================================================================

    constexpr Vector3D &operator+=(const Vector3D &rhs) {
      x += rhs.x;
      y += rhs.y;
      z += rhs.z;
      return *this;
    }

    constexpr friend Vector3D operator+(Vector3D lhs, const Vector3D &rhs) {
      lhs += rhs;
      return lhs;
    }

    constexpr Vector3D &operator-=(const Vector3D &rhs) {
      x -= rhs.x;
      y -= rhs.y;
      z -= rhs.z;
      return *this;
    }

    constexpr friend Vector3D operator-(Vector3D lhs, const Vector3D &rhs) {
      lhs -= rhs;
      return lhs;
    }

    constexpr Vector3D& operator-=(const float f) {
      x -= f;
      y -= f;
      z -= f;
      return *this;
    }

    constexpr Vector3D &operator*=(float rhs) {
      x *= rhs;
      y *= rhs;
      z *= rhs;
      return *this;
    }

    constexpr friend Vector3D operator*(Vector3D lhs, float rhs) {
      lhs *= rhs;
      return lhs;
    }

    constexpr friend Vector3D operator*(float lhs, Vector3D rhs) {
      rhs *= lhs;
      return rhs;
    }

    constexpr Vector3D &operator/=(float rhs) {
      x /= rhs;
      y /= rhs;
      z /= rhs;
      return *this;
    }

    constexpr friend Vector3D operator/(Vector3D lhs, float rhs) {
      lhs /= rhs;
      return lhs;
    }

    constexpr friend Vector3D operator/(float lhs, Vector3D rhs) {
      rhs /= lhs;
      return rhs;
    }

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    constexpr bool operator==(const Vector3D &rhs) const {
      return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
    }

    constexpr bool operator!=(const Vector3D &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    /// These 2 methods are explicitly deleted to avoid creating them by other users,
    /// unlike locations, some vectors have units and some don't, by removing
    /// these methods we found several places were the conversion from cm to m was missing
    Vector3D(const FVector &v) = delete;
    Vector3D& operator=(const FVector &rhs) = delete;

    /// Return a Vector3D converted from centimeters to meters.
    inline Vector3D ToMeters() const {
      return *this * 1e-2f;
    }

    /// Return a Vector3D converted from meters to centimeters.
    inline Vector3D ToCentimeters() const {
      return *this * 1e2f;
    }

    inline FVector ToFVector() const {
      return FVector{x, y, z};
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    // =========================================================================
    /// @todo The following is copy-pasted from MSGPACK_DEFINE_ARRAY.
    /// This is a workaround for an issue in msgpack library. The
    /// MSGPACK_DEFINE_ARRAY macro is shadowing our `z` variable.
    /// https://github.com/msgpack/msgpack-c/issues/709
    // =========================================================================
    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_pack(pk);
    }
    void msgpack_unpack(clmdep_msgpack::object const& o)
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_unpack(o);
    }
    template <typename MSGPACK_OBJECT>
    void msgpack_object(MSGPACK_OBJECT* o, clmdep_msgpack::zone& sneaky_variable_that_shadows_z) const
    {
        clmdep_msgpack::type::make_define_array(x, y, z).msgpack_object(o, sneaky_variable_that_shadows_z);
    }
    // =========================================================================
  };

} // namespace geom
} // namespace carla
