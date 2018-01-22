// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/server/CarlaMeasurements.h"
#include "carla/server/CarlaServerAPI.h"
#include "carla/server/ImagesMessage.h"

namespace carla {
namespace server {

  class MeasurementsMessage : private NonCopyable {
  public:

    void Write(
        const carla_measurements &measurements,
        const_array_view<carla_image> images) {
      _measurements.Write(measurements);
      _images.Write(images);
    }

    const carla_measurements &measurements() const {
      return _measurements.measurements();
    }

    const_buffer images() const {
      return _images.buffer();
    }

  private:

    CarlaMeasurements _measurements;

    ImagesMessage _images;
  };

} // namespace server
} // namespace carla
