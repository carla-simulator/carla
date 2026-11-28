// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaTrafficLightStatusPublisher.h"

namespace carla {
namespace ros2 {

bool CarlaTrafficLightStatusPublisher::Write(
    const std::vector<TrafficLightState> &states) {
  if (_has_last_states && StatesEqual(states, _last_states)) {
    return false;
  }
  auto message = _impl->GetMessage();

  message->traffic_lights.clear();
  message->traffic_lights.reserve(states.size());
  for (const auto &state : states) {
    msg::CarlaTrafficLightStatus status;
    status.id = state.id;
    status.state = state.state;
    message->traffic_lights.push_back(status);
  }

  _last_states = states;
  _has_last_states = true;
  return true;
}

}  // namespace ros2
}  // namespace carla
