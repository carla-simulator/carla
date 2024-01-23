// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/data/LaneInvasionEvent.h"

#include "carla/Exception.h"
#include "carla/client/detail/Simulator.h"

namespace carla {
namespace sensor {
namespace data {

  SharedPtr<client::Actor> LaneInvasionEvent::GetActor() const {
    auto episode = GetEpisode().Lock();
    auto description = episode->GetActorById(_parent);
    if (!description.has_value()) {
      throw_exception(std::runtime_error("LaneInvasionEvent: parent already dead"));
    }
    return episode->MakeActor(*description);
  }

} // namespace data
} // namespace sensor
} // namespace carla
