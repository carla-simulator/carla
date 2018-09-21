// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/sensor/s11n/SensorHeaderSerializer.h"

#include <cstdint>
#include <iterator>

namespace carla {
namespace sensor {

  class DataMessage {
   using HeaderSerializer = s11n::SensorHeaderSerializer;
  private:

    const auto &GetHeader() const {
     return HeaderSerializer::Deserialize(_buffer);
    }

  public:

    uint64_t GetSensorTypeId() const {
     return GetHeader().sensor_type;
    }

    uint64_t GetFrameNumber() const {
     return GetHeader().frame_number;
    }

    const rpc::Transform &GetSensorTransform() const {
     return GetHeader().sensor_transform;
    }

    auto begin() {
     return _buffer.begin() + HeaderSerializer::header_offset;
    }

    auto begin() const {
     return _buffer.begin() + HeaderSerializer::header_offset;
    }

    auto end() {
     return _buffer.end();
    }

    auto end() const {
     return _buffer.end();
    }

    size_t size() const {
     return std::distance(begin(), end());
    }

  private:

    template <typename... Items>
    friend class CompositeSerializer;

    DataMessage(Buffer buffer) : _buffer(std::move(buffer)) {}

    Buffer _buffer;
  };

} // namespace sensor
} // namespace carla
