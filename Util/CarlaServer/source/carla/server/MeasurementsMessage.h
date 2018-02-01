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
#include "carla/server/LidarMeasurementsMessage.h"

namespace carla {
namespace server {

  class MeasurementsMessage : private NonCopyable {
  public:

    /// Allocates a new buffer if the capacity is not enough to hold the images and
    /// lidar measurements, but it does not allocate a smaller one if the capacity is
    /// greater than the size of the images.
    ///
    /// @note The expected usage of this class is to mantain a constant size
    /// buffer of images, so memory allocation occurs only once.
    void Write(
        const carla_measurements &measurements,
        const_array_view<carla_image> images,
        const_array_view<carla_lidar_measurement> lidar_measurements) {

      _measurements.Write(measurements);

      uint32_t buffer_size = _images.GetSize(images) + _lidar_measurements.GetSize(lidar_measurements);
      Reset(sizeof(uint32_t) + buffer_size); // header + buffer
      auto begin = _buffer.get();
      std::memcpy(begin, &buffer_size, sizeof(uint32_t));
      begin += sizeof(uint32_t);

      begin += _images.Write(images, begin);
      _lidar_measurements.Write(lidar_measurements, begin);
    }

    const carla_measurements &measurements() const {
      return _measurements.measurements();
    }

    const_buffer buffer() const {
      return boost::asio::buffer(_buffer.get(), _size);
    }

  protected:

    void Reset(uint32_t count);

  private:

    CarlaMeasurements _measurements;

    ImagesMessage _images;

    LidarMeasurementsMessage _lidar_measurements;

    std::unique_ptr<unsigned char[]> _buffer = nullptr;

    uint32_t _size = 0u;

    uint32_t _capacity = 0u;
  };

} // namespace server
} // namespace carla
