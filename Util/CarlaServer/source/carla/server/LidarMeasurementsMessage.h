
#pragma once

#include <cstdint>
#include <memory>

#include "carla/ArrayView.h"
#include "carla/NonCopyable.h"
#include "carla/server/CarlaServerAPI.h"
#include "carla/server/ServerTraits.h"

namespace carla {
namespace server {

  class LidarMeasurementsMessage : private NonCopyable {
  public:

    size_t Write(
      const_array_view<carla_lidar_measurement> lidar_measurements,
      unsigned char *buffer
    );

    size_t GetSize(const_array_view<carla_lidar_measurement> lidar_measurements);
  };

} // namespace server
} // namespace carla
