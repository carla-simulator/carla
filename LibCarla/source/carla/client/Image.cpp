// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Debug.h"
#include "carla/client/Image.h"

#include <cstdint>
#include <cstring>

namespace carla {
namespace client {

  /// @todo This object should be shared and packed.
  struct FImageHeaderData
  {
    uint64_t FrameNumber;
    uint32_t Width;
    uint32_t Height;
    uint32_t Type;
    float FOV;
  };

  static std::string GetTypeString(uint32_t type) {
    switch (type) {
      case 0u: return "None";
      case 1u: return "SceneFinal";
      case 2u: return "Depth";
      case 3u: return "SemanticSegmentation";
      default: return "Invalid";
    }
  }

  SharedPtr<Image> Image::FromBuffer(Buffer buffer) {
    /// @todo We can avoid making another copy of the buffer here.
    if (buffer.size() < sizeof(FImageHeaderData)) {
      throw std::invalid_argument("buffer too small to be an image");
    }
    auto begin = reinterpret_cast<const byte_type *>(buffer.data());
    auto image = MakeShared<Image>();
    FImageHeaderData data;
    std::memcpy(&data, begin, sizeof(data));
    image->_frame_number = data.FrameNumber;
    image->_width = data.Width;
    image->_height = data.Height;
    image->_type = GetTypeString(data.Type);
    image->_fov = data.FOV;
    const auto size = image->GetSize();
    DEBUG_ASSERT((size + sizeof(FImageHeaderData)) == buffer.size());
    auto raw_data = std::make_unique<byte_type[]>(size);
    std::memcpy(raw_data.get(), begin + sizeof(FImageHeaderData), size);
    image->_raw_data = std::move(raw_data);
    return image;
  }

  Image::Image() {
    Clear();
  }

  Image::Image(Image &&rhs) {
    (*this) = std::move(rhs);
  }

  Image &Image::operator=(Image &&rhs) {
    _width = rhs._width;
    _height = rhs._height;
    _type = rhs._type;
    _fov = rhs._fov;
    _raw_data = std::move(rhs._raw_data);
    rhs.Clear();
    return *this;
  }

  void Image::Clear() {
    _frame_number = 0u;
    _width = 0u;
    _height = 0u;
    _type = "Invalid";
    _fov = 0.0f;
    _raw_data = nullptr;
  }

} // namespace client
} // namespace carla
