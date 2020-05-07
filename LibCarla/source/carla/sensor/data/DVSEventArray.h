// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/data/DVSEvent.h"
#include "carla/sensor/data/Color.h"
#include "carla/sensor/s11n/DVSEventArraySerializer.h"

namespace carla {
namespace sensor {
namespace data {

  /// An array of DVS Events in an image structure HxW
  class DVSEventArray : public Array<DVSEvent> {
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

    ///  Get an event "frame" image for visualization
    std::vector<Color> ToImage() const {
      std::vector<Color> img(GetHeight() * GetWidth());
      for (const auto &event : *this) {
        size_t index = (GetWidth() * event.y) + event.x;
        if (event.pol == true) {
          // Blue is positive
          img[index].b = 255u;
        } else {
          // Red is negative
          img[index].r = 255u;
        }
      }
      return img;
    }

    /// Get the array of events in pure vector format
    std::vector<std::vector<std::int64_t>> ToArray() const {
      std::vector<std::vector<std::int64_t>> array;
      for (const auto &event : *this) {
        array.push_back({static_cast<std::int64_t>(event.x), static_cast<std::int64_t>(event.y), static_cast<std::int64_t>(event.t), (2*static_cast<std::int64_t>(event.pol)) - 1});
      }
      return array;
    }

    /// Get all events' x coordinate for convenience
    std::vector<std::uint16_t> ToArrayX() const {
      std::vector<std::uint16_t> array;
      for (const auto &event : *this) {
        array.push_back(event.x);
      }
      return array;
    }

    /// Get all events' y coordinate for convenience
    std::vector<std::uint16_t> ToArrayY() const {
      std::vector<std::uint16_t> array;
      for (const auto &event : *this) {
        array.push_back(event.y);
      }
      return array;
    }

    /// Get all events' timestamp for convenience
    std::vector<std::int64_t> ToArrayT() const {
      std::vector<std::int64_t> array;
      for (const auto &event : *this) {
        array.push_back(event.t);
      }
      return array;
    }

    /// Get all events' polarity for convenience
    std::vector<short> ToArrayPol() const {
      std::vector<short> array;
      for (const auto &event : *this) {
        array.push_back(2*static_cast<short>(event.pol) - 1);
      }
      return array;
    }

  };

} // namespace data
} // namespace sensor
} // namespace carla
