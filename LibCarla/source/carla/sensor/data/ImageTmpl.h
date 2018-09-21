// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/ImageSerializer.h"

namespace carla {
namespace sensor {
namespace data {

  template <typename T>
  class ImageTmpl : public Array<s11n::ImageSerializer::header_offset, T>  {
    using Super = Array<s11n::ImageSerializer::header_offset, T>;
  protected:

    using Serializer = s11n::ImageSerializer;

    friend Serializer;

    explicit ImageTmpl(DataMessage message)
      : Super(std::move(message)) {
      DEBUG_ASSERT(GetWidth() * GetHeight() == Super::size());
    }

  private:

   const auto &GetHeader() const {
     return Serializer::DeserializeHeader(Super::GetMessage());
   }

  public:

    auto GetWidth() const {
     return GetHeader().width;
    }

    auto GetHeight() const {
     return GetHeader().height;
    }

    uint64_t GetFOVAngle() const {
     return GetHeader().fov_angle;
    }
  };

} // namespace data
} // namespace sensor
} // namespace carla
