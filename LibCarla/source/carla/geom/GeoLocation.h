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

    // Get Location in Transverse Mercator projection
    // using as base the already defined latitude, longitude and altitude in the object
    Location GetTransversemercProjection(double lat, double lon, double alt) const;

    // Get GeoLocation Inversing Traverse Mercator projection
    // using as base the already defined latitude, longitude and altitude in the object
    GeoLocation InverseTransversemercProjection(double x, double y, double alt) const;

    // =========================================================================
    // -- Transform locations --------------------------------------------------
    // =========================================================================

    /// Transform the given @a location to a GeoLocation using this as
    /// geo-reference.
    GeoLocation Transform(const Location &location) const;

    // Transform the given @a location to a GeoLocation using this as
    // geo-reference.
    Location GeoLocationToTransform(double lat, double lon, double altitude) const;

    Location GeoLocationToTransform(const GeoLocation other) const;

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
