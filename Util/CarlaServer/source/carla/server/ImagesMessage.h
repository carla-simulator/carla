// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
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

    size_t Write(
      const_array_view<carla_image> images,
      unsigned char *buffer
    );

    size_t GetSize(const_array_view<carla_image> images);
  };

} // namespace server
} // namespace carla
