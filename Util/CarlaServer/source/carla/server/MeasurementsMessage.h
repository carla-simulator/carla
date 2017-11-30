// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/server/CarlaMeasurements.h"
#include "carla/server/CarlaServerAPI.h"
#include "carla/server/ImagesMessage.h"
#include "carla/server/LidarMeasurementsMessage.h"

namespace carla {
namespace server {

  class MeasurementsMessage : private NonCopyable {
  public:

    void Write(
        const carla_measurements &measurements,
        const_array_view<carla_image> images,
        const_array_view<carla_lidar_measurement> lidar_measurements) {
      _measurements.Write(measurements);
      _images.Write(images);
      _lidar_measurements.Write(lidar_measurements);
    }

    const carla_measurements &measurements() const {
      return _measurements.measurements();
    }

    const_buffer images() const {
      return _images.buffer();
    }

    const_buffer lidar_measurements() const {
      return _lidar_measurements.buffer();
    }

  private:

    CarlaMeasurements _measurements;

    ImagesMessage _images;

    LidarMeasurementsMessage _lidar_measurements;
  };

} // namespace server
} // namespace carla
