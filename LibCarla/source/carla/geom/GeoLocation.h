// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

namespace carla {
namespace geom {

  class Location;

  class GeoLocation {
  public:

    // =========================================================================
    // -- Public data members --------------------------------------------------
    // =========================================================================

    double latitude = 0.0;

    double longitude = 0.0;

    double altitude = 0.0;

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    GeoLocation() = default;

    GeoLocation(double latitude, double longitude, double altitude)
      : latitude(latitude),
        longitude(longitude),
        altitude(altitude) {}

    // =========================================================================
    // -- Transform locations --------------------------------------------------
    // =========================================================================

    /// Transform the given @a location to a GeoLocation using this as
    /// geo-reference.
    GeoLocation Transform(const Location &location) const;

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const GeoLocation &rhs) const {
      return (latitude == rhs.latitude) && (longitude == rhs.longitude) && (altitude == rhs.altitude);
    }

    bool operator!=(const GeoLocation &rhs) const {
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
        clmdep_msgpack::type::make_define_array(latitude, longitude, altitude).msgpack_pack(pk);
    }
    void msgpack_unpack(clmdep_msgpack::object const& o)
    {
        clmdep_msgpack::type::make_define_array(latitude, longitude, altitude).msgpack_unpack(o);
    }
    template <typename MSGPACK_OBJECT>
    void msgpack_object(MSGPACK_OBJECT* o, clmdep_msgpack::zone& sneaky_variable_that_shadows_z) const
    {
        clmdep_msgpack::type::make_define_array(latitude, longitude, altitude).msgpack_object(o, sneaky_variable_that_shadows_z);
    }
    // =========================================================================

  };

} // namespace geom
} // namespace carla
