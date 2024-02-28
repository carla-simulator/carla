// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/EpisodeStateSerializer.h"

#include "carla/sensor/data/RawEpisodeState.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> EpisodeStateSerializer::Deserialize(RawData DESERIALIZE_DECL_DATA(data)) {
    return SharedPtr<data::RawEpisodeState>(new data::RawEpisodeState{DESERIALIZE_MOVE_DATA(data)});
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
