// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/sensor/s11n/SensorHeaderSerializer.h"
#include "carla/sensor/Deserializer.h"

#include <cstdint>
#include <iterator>

namespace carla {
namespace sensor {

  /// Wrapper around the raw data generated by a sensor plus some useful
  /// meta-information.
  class RawData {
   using HeaderSerializer = s11n::SensorHeaderSerializer;
  private:

    const auto &GetHeader() const {
     return HeaderSerializer::Deserialize(_buffer);
    }

  public:

    /// Type-id of the sensor that generated the data.
    uint64_t GetSensorTypeId() const {
     return GetHeader().sensor_type;
    }

    /// Frame count when the data was generated.
    uint64_t GetFrame() const {
     return GetHeader().frame;
    }

    /// Timestamp when the data was generated.
    double GetTimestamp() const {
     return GetHeader().timestamp;
    }

    /// Sensor's transform when the data was generated.
    const rpc::Transform &GetSensorTransform() const {
     return GetHeader().sensor_transform;
    }

    /// Begin iterator to the data generated by the sensor.
    auto begin() noexcept {
     return _buffer.begin() + HeaderSerializer::header_offset;
    }

    /// @copydoc begin()
    auto begin() const noexcept {
     return _buffer.begin() + HeaderSerializer::header_offset;
    }

    /// Past-the-end iterator to the data generated by the sensor.
    auto end() noexcept {
     return _buffer.end();
    }

    /// @copydoc end()
    auto end() const noexcept {
     return _buffer.end();
    }

    /// Retrieve a pointer to the memory containing the data generated by the
    /// sensor.
    auto data() noexcept {
      return begin();
    }

    /// @copydoc data()
    auto data() const noexcept {
      return begin();
    }

    /// Size in bytes of the data generated by the sensor.
    size_t size() const {
      DEBUG_ASSERT(std::distance(begin(), end()) >= 0);
      return static_cast<size_t>(std::distance(begin(), end()));
    }

  private:

    template <typename... Items>
    friend class CompositeSerializer;

    RawData(Buffer DESERIALIZE_DECL_DATA(buffer)) : _buffer(DESERIALIZE_MOVE_DATA(buffer)) {}

#if defined(CARLA_SERVER_BUILD)
    Buffer const &_buffer;
#else
    Buffer _buffer;
#endif
  };

} // namespace sensor
} // namespace carla
