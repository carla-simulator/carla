// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/rpc/FloatColor.h"
#include "carla/sensor/data/Color.h"

#include <vector>

namespace carla {
namespace rpc {

  template<typename T>
  class Texture {
  public:

    Texture() = default;

    Texture(uint32_t width, uint32_t height)
      : _width(width), _height(height) {
      _texture_data.resize(_width*_height);
    }

    uint32_t GetWidth() const {
      return _width;
    }

    uint32_t GetHeight() const {
      return _height;
    }

    void SetDimensions(uint32_t width, uint32_t height) {
      _width = width;
      _height = height;
      _texture_data.resize(_width*_height);
    }

    T& At (uint32_t x, uint32_t y) {
      return _texture_data[y*_width + x];
    }

    const T& At (uint32_t x, uint32_t y) const {
      return _texture_data[y*_width + x];
    }

    const T* GetDataPtr() const {
      return _texture_data.data();
    }

  private:

    uint32_t _width = 0;
    uint32_t _height = 0;
    std::vector<T> _texture_data;

  public:

    MSGPACK_DEFINE_ARRAY(_width, _height, _texture_data);
  };

  using TextureColor = Texture<sensor::data::Color>;
  using TextureFloatColor = Texture<FloatColor>;

}
}
