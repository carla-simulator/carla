// Copyright (c) 2019 Intel Labs.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/LaneMarking.h"
#include "carla/sensor/SensorData.h"

#include <vector>

namespace carla {
namespace sensor {
namespace data {

  /// A change of gnss data
  class GnssEvent : public SensorData {
  public:

    explicit GnssEvent(
        size_t frame_number,
        const rpc::Transform &sensor_transform,
        double lat,
        double lon,
        double alt)
      : SensorData(frame_number, sensor_transform),
        _lat(std::move(lat)),
        _lon(std::move(lon)),
        _alt(std::move(alt)) {}

    const double &GetLatitude() const {
      return _lat;
    }

    const double &GetLongitude() const {
      return _lon;
    }

    const double &GetAltitude() const {
      return _alt;
    }

  private:

    double _lat;

    double _lon;

    double _alt;
  };

} // namespace data
} // namespace sensor
} // namespace carla
