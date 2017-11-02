// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/server/ImagesMessage.h"

#include <cstring>

#include "carla/Debug.h"
#include "carla/Logging.h"

namespace carla {
namespace server {

  static size_t GetSizeOfBuffer(const_array_view<carla_image> images) {
    size_t total = 0u;
    for (const auto &image : images) {
      total += 3u; // width, height, type.
      total += image.width * image.height;
    }
    return total * sizeof(uint32_t);
  }

  static size_t WriteSizeToBuffer(unsigned char *buffer, uint32_t size) {
    std::memcpy(buffer, &size, sizeof(uint32_t));
    return sizeof(uint32_t);
  }

  static size_t WriteImageToBuffer(unsigned char *buffer, const carla_image &image) {
    const auto size = sizeof(uint32_t) * image.width * image.height;
    DEBUG_ASSERT(image.data != nullptr);
    std::memcpy(buffer, image.data, size);
    return size;
  }

  void ImagesMessage::Write(const_array_view<carla_image> images) {
    const size_t buffer_size = GetSizeOfBuffer(images);
    Reset(sizeof(uint32_t) + buffer_size); // header + buffer.

    auto begin = _buffer.get();
    begin += WriteSizeToBuffer(begin, buffer_size);
    for (const auto &image : images) {
      begin += WriteSizeToBuffer(begin, image.width);
      begin += WriteSizeToBuffer(begin, image.height);
      begin += WriteSizeToBuffer(begin, image.type);
      begin += WriteImageToBuffer(begin, image);
    }
    DEBUG_ASSERT(std::distance(_buffer.get(), begin) == _size);
  }

  void ImagesMessage::Reset(const uint32_t count) {
    if (_capacity < count) {
      log_info("allocating image buffer of", count, "bytes");
      _buffer = std::make_unique<unsigned char[]>(count);
      _capacity = count;
    }
    _size = count;
  }

} // namespace server
} // namespace carla
