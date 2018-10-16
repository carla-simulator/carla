// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

namespace carla {
namespace geom {

  class Vector3D {
  public:

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vector3D() = default;

    Vector3D(float ix, float iy, float iz)
      : x(ix),
        y(iy),
        z(iz) {}

    Vector3D &operator+=(const Vector3D &rhs) {
      x += rhs.x;
      y += rhs.y;
      z += rhs.z;
      return *this;
    }

    friend Vector3D operator+(Vector3D lhs, const Vector3D &rhs) {
      lhs += rhs;
      return lhs;
    }

    Vector3D &operator-=(const Vector3D &rhs) {
      x -= rhs.x;
      y -= rhs.y;
      z -= rhs.z;
      return *this;
    }

    friend Vector3D operator-(Vector3D lhs, const Vector3D &rhs) {
      lhs -= rhs;
      return lhs;
    }

    bool operator==(const Vector3D &rhs) const  {
      return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
    }

    bool operator!=(const Vector3D &rhs) const  {
      return !(*this == rhs);
    }

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
