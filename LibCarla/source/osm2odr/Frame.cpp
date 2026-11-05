// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Frame.h"

#include <cmath>
#include <sstream>

namespace twinmodel {

  namespace {

    // WGS84 ellipsoid, and the unit scale factor pyproj's +k=1 asks for.
    constexpr double kSemiMajor = 6378137.0;
    constexpr double kFlattening = 1.0 / 298.257223563;
    constexpr double kScale = 1.0;

    constexpr double kDegToRad = M_PI / 180.0;
    constexpr double kRadToDeg = 180.0 / M_PI;

  } // namespace

  std::string LocalFrame::GetProjString() const {
    std::ostringstream out;
    out.precision(12);
    out << "+proj=tmerc +lat_0=" << _origin_lat << " +lon_0=" << _origin_lon
        << " +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs";
    return out.str();
  }

  void LocalFrame::ToLocal(double lon, double lat, double &x, double &y) const {
    // Kruger series, the same transverse Mercator PROJ applies for +proj=tmerc.
    const double e2 = kFlattening * (2.0 - kFlattening);
    const double ep2 = e2 / (1.0 - e2);
    const double phi = lat * kDegToRad;
    const double phi0 = _origin_lat * kDegToRad;
    const double dl = (lon - _origin_lon) * kDegToRad;

    const double sin_phi = std::sin(phi);
    const double cos_phi = std::cos(phi);
    const double tan_phi = std::tan(phi);
    const double nu = kSemiMajor / std::sqrt(1.0 - e2 * sin_phi * sin_phi);
    const double t = tan_phi * tan_phi;
    const double c = ep2 * cos_phi * cos_phi;
    const double a = dl * cos_phi;

    auto meridian = [&](double p) {
      return kSemiMajor * ((1.0 - e2 / 4.0 - 3.0 * e2 * e2 / 64.0 - 5.0 * e2 * e2 * e2 / 256.0) * p
           - (3.0 * e2 / 8.0 + 3.0 * e2 * e2 / 32.0 + 45.0 * e2 * e2 * e2 / 1024.0) * std::sin(2.0 * p)
           + (15.0 * e2 * e2 / 256.0 + 45.0 * e2 * e2 * e2 / 1024.0) * std::sin(4.0 * p)
           - (35.0 * e2 * e2 * e2 / 3072.0) * std::sin(6.0 * p));
    };

    x = kScale * nu * (a + (1.0 - t + c) * a * a * a / 6.0
      + (5.0 - 18.0 * t + t * t + 72.0 * c - 58.0 * ep2) * std::pow(a, 5) / 120.0);
    y = kScale * (meridian(phi) - meridian(phi0) + nu * tan_phi *
        (a * a / 2.0 + (5.0 - t + 9.0 * c + 4.0 * c * c) * std::pow(a, 4) / 24.0
       + (61.0 - 58.0 * t + t * t + 600.0 * c - 330.0 * ep2) * std::pow(a, 6) / 720.0));
  }

  void LocalFrame::ToWGS84(double x, double y, double &lon, double &lat) const {
    const double e2 = kFlattening * (2.0 - kFlattening);
    const double ep2 = e2 / (1.0 - e2);
    const double e1 = (1.0 - std::sqrt(1.0 - e2)) / (1.0 + std::sqrt(1.0 - e2));
    const double phi0 = _origin_lat * kDegToRad;

    auto meridian = [&](double p) {
      return kSemiMajor * ((1.0 - e2 / 4.0 - 3.0 * e2 * e2 / 64.0 - 5.0 * e2 * e2 * e2 / 256.0) * p
           - (3.0 * e2 / 8.0 + 3.0 * e2 * e2 / 32.0 + 45.0 * e2 * e2 * e2 / 1024.0) * std::sin(2.0 * p)
           + (15.0 * e2 * e2 / 256.0 + 45.0 * e2 * e2 * e2 / 1024.0) * std::sin(4.0 * p)
           - (35.0 * e2 * e2 * e2 / 3072.0) * std::sin(6.0 * p));
    };

    const double m = meridian(phi0) + y / kScale;
    const double mu = m / (kSemiMajor * (1.0 - e2 / 4.0 - 3.0 * e2 * e2 / 64.0
                                             - 5.0 * e2 * e2 * e2 / 256.0));
    const double phi1 = mu
      + (3.0 * e1 / 2.0 - 27.0 * e1 * e1 * e1 / 32.0) * std::sin(2.0 * mu)
      + (21.0 * e1 * e1 / 16.0 - 55.0 * e1 * e1 * e1 * e1 / 32.0) * std::sin(4.0 * mu)
      + (151.0 * e1 * e1 * e1 / 96.0) * std::sin(6.0 * mu);

    const double sin_phi1 = std::sin(phi1);
    const double cos_phi1 = std::cos(phi1);
    const double tan_phi1 = std::tan(phi1);
    const double c1 = ep2 * cos_phi1 * cos_phi1;
    const double t1 = tan_phi1 * tan_phi1;
    const double n1 = kSemiMajor / std::sqrt(1.0 - e2 * sin_phi1 * sin_phi1);
    const double r1 = kSemiMajor * (1.0 - e2) / std::pow(1.0 - e2 * sin_phi1 * sin_phi1, 1.5);
    const double d = x / (n1 * kScale);

    const double phi = phi1 - (n1 * tan_phi1 / r1) *
        (d * d / 2.0 - (5.0 + 3.0 * t1 + 10.0 * c1 - 4.0 * c1 * c1 - 9.0 * ep2) * std::pow(d, 4) / 24.0
       + (61.0 + 90.0 * t1 + 298.0 * c1 + 45.0 * t1 * t1 - 252.0 * ep2 - 3.0 * c1 * c1)
         * std::pow(d, 6) / 720.0);
    const double dl = (d - (1.0 + 2.0 * t1 + c1) * d * d * d / 6.0
       + (5.0 - 2.0 * c1 + 28.0 * t1 - 3.0 * c1 * c1 + 8.0 * ep2 + 24.0 * t1 * t1)
         * std::pow(d, 5) / 120.0) / cos_phi1;

    lat = phi * kRadToDeg;
    lon = _origin_lon + dl * kRadToDeg;
  }

} // namespace twinmodel
