// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Memory.h"
#include "carla/sensor/RawData.h"

namespace carla {
namespace sensor {

  class SensorData;

namespace s11n {

  class IMUSerializer
  {
  public:

    struct Data {

      geom::Vector3D accelerometer;

      geom::Vector3D gyroscope;

      float compass;

      MSGPACK_DEFINE_ARRAY(accelerometer, gyroscope, compass)
    };

    template <typename SensorT>
    static Buffer Serialize(
      const SensorT &sensor,
      const geom::Vector3D &accelerometer,
      const geom::Vector3D &gyroscope,
      const float compass);

    static Data DeserializeRawData(const RawData &message) {
      return MsgPack::UnPack<Data>(message.begin(), message.size());
    }

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

  template <typename SensorT>
  inline Buffer IMUSerializer::Serialize(
      const SensorT &,
      const geom::Vector3D &accelerometer,
      const geom::Vector3D &gyroscope,
      const float compass) {
    return MsgPack::Pack(Data{accelerometer, gyroscope, compass});
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
