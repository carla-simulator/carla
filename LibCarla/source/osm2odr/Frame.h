// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace twinmodel {

  /// WGS84 <-> local ENU metres, a transverse Mercator centred on the origin.
  /// Ported from twinmodel/frame.py, which gets the same projection from pyproj.
  class LocalFrame {
  public:

    LocalFrame() = default;

    LocalFrame(double origin_lat, double origin_lon)
      : _origin_lat(origin_lat), _origin_lon(origin_lon) {}

    static LocalFrame FromBoundingBox(double south, double west, double north, double east) {
      return LocalFrame((south + north) * 0.5, (west + east) * 0.5);
    }

    double GetOriginLatitude() const { return _origin_lat; }
    double GetOriginLongitude() const { return _origin_lon; }

    /// The +proj string written as the OpenDRIVE <geoReference>.
    std::string GetProjString() const;

    void ToLocal(double lon, double lat, double &x, double &y) const;
    void ToWGS84(double x, double y, double &lon, double &lat) const;

  private:

    double _origin_lat = 0.0;
    double _origin_lon = 0.0;
  };

} // namespace twinmodel
