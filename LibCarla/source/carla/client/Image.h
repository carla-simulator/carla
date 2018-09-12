// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/NonCopyable.h"
#include "carla/client/Memory.h"

#include <memory>

namespace carla {
namespace client {

  class Image
    : public EnableSharedFromThis<Image>,
      private NonCopyable  {
  public:

    using byte_type = unsigned char;

    static SharedPtr<Image> FromBuffer(Buffer buffer);

    Image();

    Image(Image &&rhs);
    Image &operator=(Image &&rhs);

    void Clear();

    size_t GetFrameNumber() const {
      return _frame_number;
    }

    size_t GetWidth() const {
      return _width;
    }

    size_t GetHeight() const {
      return _height;
    }

    const std::string &GetType() const {
      return _type;
    }

    float GetFOV() const {
      return _fov;
    }

    byte_type *GetData() {
      return _raw_data.get();
    }

    const byte_type *GetData() const {
      return _raw_data.get();
    }

    size_t GetSize() const {
      return 4u * _width * _height;
    }

  private:

    size_t _frame_number;

    size_t _width;

    size_t _height;

    std::string _type;

    float _fov;

    std::unique_ptr<byte_type[]> _raw_data;
  };

} // namespace client
} // namespace carla
