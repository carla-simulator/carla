// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/server/CarlaServerAPI.h"

#include <memory>

namespace carla {
namespace server {

  /// Holds a carla_measurements and keeps its own copy of the dynamically
  /// allocated data.
  class CarlaMeasurements : private NonCopyable {
  public:

    void Write(const carla_measurements &measurements);

    const carla_measurements &measurements() const {
      return _measurements;
    }

  private:

    carla_measurements _measurements;

    std::unique_ptr<unsigned char[]> _agents_buffer = nullptr;

    uint32_t _agents_buffer_size = 0u;
  };

} // namespace server
} // namespace carla
