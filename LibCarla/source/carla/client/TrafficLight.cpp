// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/TrafficLight.h"

#include "carla/client/detail/Simulator.h"

namespace carla {
namespace client {

  TrafficLightState TrafficLight::GetState() {
    auto state = GetEpisode().Lock()->GetActorDynamicState(*this).state;
    switch (state) {
      case 1u: return TrafficLightState::Red;
      case 2u: return TrafficLightState::Yellow;
      case 3u: return TrafficLightState::Green;
      default: return TrafficLightState::Unknown;
    }
  }

} // namespace client
} // namespace carla
