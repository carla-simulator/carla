// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
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

  class LidarMeasurement : public Array<rpc::Location>  {
    static_assert(sizeof(rpc::Location) == 3u * sizeof(float), "Location size missmatch");
    using Super = Array<rpc::Location>;
  protected:

    using Serializer = s11n::LidarSerializer;

    friend Serializer;

    explicit LidarMeasurement(DataMessage message)
      : Super(std::move(message)) {
      Super::SetOffset(Serializer::GetHeaderOffset(Super::GetMessage()));
    }

  private:

    auto GetHeader() const {
      return Serializer::DeserializeHeader(Super::GetMessage());
    }

  public:

    auto GetHorizontalAngle() const {
      return GetHeader().GetHorizontalAngle();
    }

    auto GetChannelCount() const {
      return GetHeader().GetChannelCount();
    }

    auto GetPointCount(size_t channel) const {
      return GetHeader().GetPointCount(channel);
    }
  };

} // namespace data
} // namespace sensor
} // namespace carla
