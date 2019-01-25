// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Vehicle.h"

#include "carla/client/detail/Simulator.h"

namespace carla {
namespace client {

  void Vehicle::SetAutopilot(bool enabled) {
    GetEpisode().Lock()->SetVehicleAutopilot(*this, enabled);
  }

  void Vehicle::ApplyControl(const Control &control) {
    if (control != _control) {
      GetEpisode().Lock()->ApplyControlToVehicle(*this, control);
      _control = control;
    }
  }

  Vehicle::Control Vehicle::GetControl() const {
    return GetEpisode().Lock()->GetActorDynamicState(*this).state.vehicle_control;
  }

} // namespace client
} // namespace carla
