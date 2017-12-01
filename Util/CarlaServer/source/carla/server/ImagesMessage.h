// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <memory>

#include "carla/ArrayView.h"
#include "carla/NonCopyable.h"
#include "carla/server/CarlaServerAPI.h"
#include "carla/server/ServerTraits.h"

namespace carla {
namespace server {

  /// Encodes the given images as binary array to be sent to the client.
  ///
  /// The message consists of an array of uint32's in the following layout
  ///
  ///    {
  ///      total size,
  ///      width, height, type, FOV, color[0], color[1],...,     <- first image
  ///      width, height, type, FOV, color[0], color[1],...,     <- second image
  ///      ...
  ///    }
  ///
  class ImagesMessage : private NonCopyable {
  public:

    /// Allocates a new buffer if the capacity is not enough to hold the images,
    /// but it does not allocate a smaller one if the capacity is greater than
    /// the size of the images.
    ///
    /// @note The expected usage of this class is to mantain a constant size
    /// buffer of images, so memory allocation occurs only once.
    void Write(const_array_view<carla_image> images);

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
