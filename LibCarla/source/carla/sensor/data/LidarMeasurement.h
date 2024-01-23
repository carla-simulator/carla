// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/rpc/Location.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/LidarSerializer.h"

namespace carla {
namespace sensor {
namespace data {

  /// Measurement produced by a Lidar. Consists of an array of 3D points plus
  /// some extra meta-information about the Lidar.
  class LidarMeasurement : public Array<data::LidarDetection>  {
    static_assert(sizeof(data::LidarDetection) == 4u * sizeof(float), "Location size missmatch");
    using Super = Array<data::LidarDetection>;

  protected:
    using Serializer = s11n::LidarSerializer;

    friend Serializer;

    explicit LidarMeasurement(RawData &&data)
      : Super(std::move(data), [](const RawData &d) {
      return Serializer::GetHeaderOffset(d);
    }) {}

  private:

    auto GetHeader() const {
      return Serializer::DeserializeHeader(Super::GetRawData());
    }

  public:
    /// Horizontal angle of the Lidar at the time of the measurement.
    auto GetHorizontalAngle() const {
      return GetHeader().GetHorizontalAngle();
    }

    /// Number of channels of the Lidar.
    auto GetChannelCount() const {
      return GetHeader().GetChannelCount();
    }

    /// Retrieve the number of points that @a channel generated. Points are
    /// sorted by channel, so this method allows to identify the channel that
    /// generated each point.
    auto GetPointCount(size_t channel) const {
      return GetHeader().GetPointCount(channel);
    }
  };

} // namespace data
} // namespace sensor
} // namespace carla
