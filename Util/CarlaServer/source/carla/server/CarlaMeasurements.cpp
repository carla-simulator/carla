// CARLA, Copyright (C) 2017 Computer Vision Center (CVC) Project Settings.

#include "carla/server/CarlaMeasurements.h"

#include "carla/Logging.h"

namespace carla {
namespace server {

  void CarlaMeasurements::Write(const carla_measurements &measurements) {
    _measurements = measurements;
    const auto size = measurements.number_of_non_player_agents * sizeof(carla_agent);
    if (_agents_buffer_size < size) {
      log_info("allocating agents buffer of", size, "bytes");
      _agents_buffer = std::make_unique<unsigned char[]>(size);
      _agents_buffer_size = size;
    }
    std::memcpy(_agents_buffer.get(), measurements.non_player_agents, size);
    _measurements.non_player_agents =
        reinterpret_cast<const carla_agent *>(_agents_buffer.get());
  }

} // namespace server
} // namespace carla
