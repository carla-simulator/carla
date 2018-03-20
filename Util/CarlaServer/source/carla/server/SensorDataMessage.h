// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/server/ServerTraits.h"

#include <memory>

struct carla_sensor_data;
struct carla_sensor_definition;

namespace carla {
namespace server {

  class SensorDataMessage : private NonCopyable {
  public:

    void Write(const carla_sensor_data &data);

    const_buffer buffer() const {
      return boost::asio::buffer(_buffer.get(), _size);
    }

  private:

    void Reset(uint32_t count);

    std::unique_ptr<unsigned char[]> _buffer = nullptr;

    uint32_t _size = 0u;

    uint32_t _capacity = 0u;
  };

} // namespace server
} // namespace carla
