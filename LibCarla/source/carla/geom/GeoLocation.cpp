// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/GeoLocation.h"

#include "carla/geom/Location.h"
#include "carla/geom/Math.h"

#include <cmath>

#if defined(_WIN32) && !defined(_USE_MATH_DEFINES)
#  define _USE_MATH_DEFINES
#  include <math.h> // cmath is not enough for MSVC
#endif

namespace carla {
namespace geom {

  /// Earth radius at equator [m].
  static constexpr double EARTH_RADIUS_EQUA = 6378137.0;

  /// Convert latitude to scale, which is needed by mercator
  /// transformations
  /// @param lat latitude in degrees (DEG)
  /// @return scale factor
  /// @note when converting from lat/lon -> mercator and back again,
  ///        or vice versa, use the same scale in both transformations!
  static double LatToScale(double lat) {
    return std::cos(Math::ToRadians(lat));
  }

  static double DegreesToRadians(double degrees) {
    return degrees * Math::Pi<double>() / 180.0;
  }

  static double RadiansToDegrees(double radians) {
    return radians * 180.0 / Math::Pi<double>();
  }

  /// Converts lat/lon/scale to mx/my (mx/my in meters if correct scale
  /// is given).
  template <class double_type>
  static void LatLonToMercator(double lat, double lon, double scale, double_type &mx, double_type &my) {
    mx = scale * Math::ToRadians(lon) * EARTH_RADIUS_EQUA;
    my = scale * EARTH_RADIUS_EQUA * std::log(std::tan((90.0 + lat) * Math::Pi<double>() / 360.0));
  }

  /// Converts mx/my/scale to lat/lon (mx/my in meters if correct scale
  /// is given).
  static void MercatorToLatLon(double mx, double my, double scale, double &lat, double &lon) {
    lon = mx * 180.0 / (Math::Pi<double>() * EARTH_RADIUS_EQUA * scale);
    lat = 360.0 * std::atan(std::exp(my / (EARTH_RADIUS_EQUA * scale))) / Math::Pi<double>() - 90.0;
  }

  /// Adds meters dx/dy to given lat/lon and returns new lat/lon.
  static void LatLonAddMeters(
      double lat_start,
      double lon_start,
      double dx,
      double dy,
      double &lat_end,
      double &lon_end) {
    double scale = LatToScale(lat_start);
    double mx, my;
    LatLonToMercator(lat_start, lon_start, scale, mx, my);
    mx += dx;
    my += dy;
    MercatorToLatLon(mx, my, scale, lat_end, lon_end);
  }

  // Forward (spherical TM)
  Location GeoLocation::GetTransversemercProjection(double lat, double lon, double alt) const {
      const double R = EARTH_RADIUS_EQUA;
      const double k0 = 0.9996;

      double phi = DegreesToRadians(lat);
      double lambda = DegreesToRadians(lon);
      double phi0 = DegreesToRadians(latitude);
      double lambda0 = DegreesToRadians(longitude);

      double deltaLambda = lambda - lambda0;
      double B = cos(phi) * sin(deltaLambda);

      double x = 0.5 * R * k0 * log((1 + B) / (1 - B));
      double y = k0 * R * (atan(tan(phi) / cos(deltaLambda)) - phi0);

      return Location(x, y, altitude + alt);
  }

  // Inverse (matches above)
  GeoLocation GeoLocation::InverseTransversemercProjection(double x, double y, double alt) const {
      const double R = EARTH_RADIUS_EQUA;
      const double k0 = 0.9996;

      double phi0 = DegreesToRadians(latitude);
      double lambda0 = DegreesToRadians(longitude);

      double D = x / (k0 * R);
      double E = (y / (k0 * R)) + phi0;

      double phi = asin(sin(E) / cosh(D));
      double deltaLambda = atan2(sinh(D), cos(E));

      double lambda = lambda0 + deltaLambda;

      double latDeg = RadiansToDegrees(phi);
      double lonDeg = RadiansToDegrees(lambda);

      return GeoLocation(latDeg, lonDeg, alt + altitude);
  }

  GeoLocation GeoLocation::Transform(const Location &location) const {
    return InverseTransversemercProjection(
        location.x, location.y, location.z + altitude);
  }


  Location GeoLocation::GeoLocationToTransform(double lat, double lon, double altitude) const {
    return GetTransversemercProjection(lat, lon, altitude);
  }

  Location GeoLocation::GeoLocationToTransform(const GeoLocation other) const
  {
    return GeoLocationToTransform(other.latitude, other.longitude, other.altitude);
  }
} // namespace geom
} // namespace carla
