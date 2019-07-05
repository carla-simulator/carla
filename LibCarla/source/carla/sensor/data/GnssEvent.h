// Copyright (c) 2019 Intel Labs.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/GeoLocation.h"
#include "carla/sensor/SensorData.h"

namespace carla {
namespace sensor {
namespace data {

  /// A change of GNSS data.
  class GnssEvent : public SensorData {
  public:

    explicit GnssEvent(
        size_t frame,
        double timestamp,
        const rpc::Transform &sensor_transform,
        const geom::GeoLocation &geo_location)
      : SensorData(frame, timestamp, sensor_transform),
        _geo_location(geo_location) {}

    const geom::GeoLocation &GetGeoLocation() const {
      return _geo_location;
    }

    double GetLongitude() const {
      return _geo_location.longitude;
    }

    double GetLatitude() const {
      return _geo_location.latitude;
    }

    double GetAltitude() const {
      return _geo_location.altitude;
    }

  private:

    geom::GeoLocation _geo_location;
  };

} // namespace data
} // namespace sensor
} // namespace carla
