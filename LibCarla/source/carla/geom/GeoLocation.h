// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <ostream>

namespace carla {
namespace geom {

  class Location;
  class Transform;

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

  inline std::ostream &operator<<(std::ostream &out, const GeoLocation &geo_location) {
    out << "GeoLocation(latitude=" << std::to_string(geo_location.latitude)
        << ", longitude=" << std::to_string(geo_location.longitude)
        << ", altitude=" << std::to_string(geo_location.altitude) << ')';
    return out;
  }

} // namespace geom
} // namespace carla
