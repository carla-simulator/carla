// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h"
#include "carla/geom/Vector3D.h"
#include "carla/rpc/ActorId.h"
#include "carla/rpc/TrafficLightState.h"
#include "carla/rpc/VehicleControl.h"

#include <cstdint>

namespace carla {
namespace sensor {
namespace data {

namespace detail {

#pragma pack(push, 1)
  class PackedVehicleControl {
  public:

    PackedVehicleControl() = default;

    PackedVehicleControl(const rpc::VehicleControl &control)
      : throttle(control.throttle),
        steer(control.steer),
        brake(control.brake),
        hand_brake(control.hand_brake),
        reverse(control.reverse),
        manual_gear_shift(control.manual_gear_shift),
        gear(control.gear) {}

    operator rpc::VehicleControl() const {
      return {throttle, steer, brake, hand_brake, reverse, manual_gear_shift, gear};
    }

  private:

    float throttle;
    float steer;
    float brake;
    bool hand_brake;
    bool reverse;
    bool manual_gear_shift;
    int32_t gear;
  };
#pragma pack(pop)

} // namespace detail

#pragma pack(push, 1)

  /// Dynamic state of an actor at a certain frame.
  struct ActorDynamicState {

    actor_id_type id;

    geom::Transform transform;

    geom::Vector3D velocity;

    union TypeDependentState {
      rpc::TrafficLightState traffic_light_state;
      detail::PackedVehicleControl vehicle_control;
    } state;
  };

#pragma pack(pop)

  static_assert(
      sizeof(ActorDynamicState) == 10u * sizeof(uint32_t) + sizeof(detail::PackedVehicleControl),
      "Invalid ActorDynamicState size!");

} // namespace data
} // namespace sensor
} // namespace carla
