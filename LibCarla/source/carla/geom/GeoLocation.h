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

    MSGPACK_DEFINE_ARRAY(latitude, longitude, altitude);
  };

} // namespace geom
} // namespace carla
