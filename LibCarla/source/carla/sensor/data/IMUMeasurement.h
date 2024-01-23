// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"
#include "carla/sensor/s11n/IMUSerializer.h"
#include "carla/sensor/SensorData.h"

namespace carla {
namespace sensor {
namespace data {

  class IMUMeasurement : public SensorData {
  protected:

    using Super = SensorData;

    using Serializer = s11n::IMUSerializer;

    friend Serializer;

    explicit IMUMeasurement(const RawData &data)
      : Super(data),
        _accelerometer(Serializer::DeserializeRawData(data).accelerometer),
        _gyroscope(Serializer::DeserializeRawData(data).gyroscope),
        _compass(Serializer::DeserializeRawData(data).compass) {}

  public:

    geom::Vector3D GetAccelerometer() const {
      return _accelerometer;
    }

    geom::Vector3D GetGyroscope() const {
      return _gyroscope;
    }

    float GetCompass() const {
      return _compass;
    }

  private:

    geom::Vector3D _accelerometer;
    geom::Vector3D _gyroscope;
    float _compass;

  };

} // namespace data
} // namespace sensor
} // namespace carla
