// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Vehicle.h"

#include "carla/client/detail/Simulator.h"

namespace carla {
namespace client {

  void Vehicle::ApplyControl(const Control &control) {
    if (control != _control) {
      GetEpisode()->ApplyControlToVehicle(*this, control);
      _control = control;
    }
  }

  void Vehicle::SetAutopilot(bool enabled) {
    GetEpisode()->SetVehicleAutopilot(*this, enabled);
  }

} // namespace client
} // namespace carla
