// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/data/DVSEvent.h"
#include "carla/sensor/s11n/DVSEventArraySerializer.h"

namespace carla {
namespace sensor {
namespace data {

  /// An array of DVS Events in an image structure HxW
  class DVSEventArray : public Array<DVSEvent>  {
    using Super = Array<DVSEvent>;
  protected:

    using Serializer = s11n::DVSEventArraySerializer;

    friend Serializer;

    explicit DVSEventArray(RawData data)
      : Super(Serializer::header_offset, std::move(data)) {
    }

  private:

    const auto &GetHeader() const {
      return Serializer::DeserializeHeader(Super::GetRawData());
    }
  public:

    using event_type = DVSEvent;

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
