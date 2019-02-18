// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

namespace carla {
namespace rpc {

  class WheelPhysicsControl {
  public:
    explicit WheelPhysicsControl() = default;

    explicit WheelPhysicsControl(
      float in_tire_friction,
      float in_torque,
      float in_mass,
      bool in_disable_steering
    ) {
      tire_friction = in_tire_friction;
      torque = in_torque;
      mass = in_mass;
      disable_steering = in_disable_steering;
    }

    float tire_friction;
    float torque;
    float mass;
    bool disable_steering;
    
    bool operator!=(const WheelPhysicsControl &rhs) const {
      return
          tire_friction != rhs.tire_friction ||
          torque != rhs.torque ||
          mass != rhs.mass ||
          disable_steering != rhs.disable_steering;
    }

    bool operator==(const WheelPhysicsControl &rhs) const {
      return !(*this != rhs);
    }
    #ifdef LIBCARLA_INCLUDED_FROM_UE4
      WheelPhysicsControl(const FWheelPhysicsControl &Wheel) {      
        tire_friction = Wheel.TireFriction;
        torque = Wheel.Torque;
        mass = Wheel.Mass;
        disable_steering = Wheel.bDisableSteering;
      }

      operator FWheelPhysicsControl() const {
        FWheelPhysicsControl Wheel;
        Wheel.TireFriction = tire_friction;
        Wheel.Torque = torque;
        Wheel.Mass = mass;
        Wheel.bDisableSteering = disable_steering;
        return Wheel;
      }
    #endif

    MSGPACK_DEFINE_ARRAY(tire_friction,
                        torque,
                        mass,
                        disable_steering)
  };
}
}