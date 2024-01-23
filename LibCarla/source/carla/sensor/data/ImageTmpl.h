// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/ImageSerializer.h"
#include "carla/sensor/s11n/OpticalFlowImageSerializer.h"
#include "carla/sensor/s11n/GBufferUint8Serializer.h"
#include "carla/sensor/s11n/GBufferFloatSerializer.h"
#include "carla/sensor/s11n/NormalsImageSerializer.h"
#include "carla/ros2/ROS2.h"

namespace carla {
namespace sensor {
namespace data {

  /// Templated image for any type of pixel.
  template <typename PixelT>
  class ImageTmpl : public Array<PixelT> {
    using Super = Array<PixelT>;
    friend class carla::ros2::ROS2;
  protected:

    using Serializer = s11n::ImageSerializer;
    using SerializerOpticalFlow = s11n::OpticalFlowImageSerializer;
    using SerializerNormals = s11n::NormalsImageSerializer;

    friend Serializer;
    friend SerializerOpticalFlow;
    friend s11n::GBufferUint8Serializer;
    friend s11n::GBufferFloatSerializer;
    friend SerializerNormals;

    explicit ImageTmpl(RawData &&data)
      : Super(Serializer::header_offset, std::move(data)) {
      DEBUG_ASSERT(GetWidth() * GetHeight() == Super::size());
    }

  private:

    const auto &GetHeader() const {
      return Serializer::DeserializeHeader(Super::GetRawData());
    }

  public:

    using pixel_type = PixelT;

    /// Get image width in pixels.
    auto GetWidth() const {
      return GetHeader().width;
    }

    /// Get image height in pixels.
    auto GetHeight() const {
      return GetHeader().height;
    }

    /// Get horizontal field of view of the image in degrees.
    auto GetFOVAngle() const {
      return GetHeader().fov_angle;
    }
  };

} // namespace data
} // namespace sensor
} // namespace carla
