// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace rpc {

  enum class VehicleControlType: uint8_t {
    VehicleControl,
    AckermannControl
  } ;

} // namespace rpc
} // namespace carla


