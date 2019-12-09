// Copyright (c) 2019 Intel Labs.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/GeoLocation.h"
#include "carla/sensor/SensorData.h"

#include "carla/sensor/s11n/GnssSerializer.h"

namespace carla {
namespace sensor {
namespace data {

  /// A change of GNSS Measurement.
  class GnssMeasurement : public SensorData {

    using Super = SensorData;

  protected:

    using Serializer = s11n::GnssSerializer;

    friend Serializer;

    explicit GnssMeasurement(const RawData &&data)
      : Super(data){

      geom::GeoLocation gnss_data = Serializer::DeserializeRawData(data);
      _geo_location = gnss_data;

    }

  public:

    geom::GeoLocation GetGeoLocation() const {
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
