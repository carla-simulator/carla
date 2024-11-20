// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <util/enable-ue4-macros.h>
#include "Carla/Vehicle/VehicleLightState.h"
#include <util/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  #define SET_FLAG(flag, value) flag |= static_cast<flag_type>(value)

  #define FLAG_ENABLED(flag, value) flag & static_cast<flag_type>(value)

  /// Defines the physical appearance of a vehicle whitch is obtained
  /// by the sensors.
  class VehicleLightState {
  public:

    using flag_type = uint32_t;

    /// Can be used as flags
    enum class LightState : flag_type {
      None         = 0,
      Position     = 0x1,
      LowBeam      = 0x1 << 1,
      HighBeam     = 0x1 << 2,
      Brake        = 0x1 << 3,
      RightBlinker = 0x1 << 4,
      LeftBlinker  = 0x1 << 5,
      Reverse      = 0x1 << 6,
      Fog          = 0x1 << 7,
      Interior     = 0x1 << 8,
      Special1     = 0x1 << 9,  // E.g: sirens
      Special2     = 0x1 << 10,
      All          = 0xFFFFFFFF
    };

    VehicleLightState() = default;

    VehicleLightState(LightState light_state)
      : light_state(static_cast<flag_type>(light_state)) {}

    VehicleLightState(flag_type light_state)
      : light_state(light_state) {}

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    VehicleLightState(const FVehicleLightState &InLightState) {
      light_state = static_cast<flag_type>(LightState::None);
      if (InLightState.Position)     SET_FLAG(light_state, LightState::Position);
      if (InLightState.LowBeam)      SET_FLAG(light_state, LightState::LowBeam);
      if (InLightState.HighBeam)     SET_FLAG(light_state, LightState::HighBeam);
      if (InLightState.Brake)        SET_FLAG(light_state, LightState::Brake);
      if (InLightState.RightBlinker) SET_FLAG(light_state, LightState::RightBlinker);
      if (InLightState.LeftBlinker)  SET_FLAG(light_state, LightState::LeftBlinker);
      if (InLightState.Reverse)      SET_FLAG(light_state, LightState::Reverse);
      if (InLightState.Fog)          SET_FLAG(light_state, LightState::Fog);
      if (InLightState.Interior)     SET_FLAG(light_state, LightState::Interior);
      if (InLightState.Special1)     SET_FLAG(light_state, LightState::Special1);
      if (InLightState.Special2)     SET_FLAG(light_state, LightState::Special2);
    }

    operator FVehicleLightState() const {
      FVehicleLightState Lights; // By default all False
      if (FLAG_ENABLED(light_state, LightState::Position))     Lights.Position = true;
      if (FLAG_ENABLED(light_state, LightState::LowBeam))      Lights.LowBeam = true;
      if (FLAG_ENABLED(light_state, LightState::HighBeam))     Lights.HighBeam = true;
      if (FLAG_ENABLED(light_state, LightState::Brake))        Lights.Brake = true;
      if (FLAG_ENABLED(light_state, LightState::RightBlinker)) Lights.RightBlinker = true;
      if (FLAG_ENABLED(light_state, LightState::LeftBlinker))  Lights.LeftBlinker = true;
      if (FLAG_ENABLED(light_state, LightState::Reverse))      Lights.Reverse = true;
      if (FLAG_ENABLED(light_state, LightState::Fog))          Lights.Fog = true;
      if (FLAG_ENABLED(light_state, LightState::Interior))     Lights.Interior = true;
      if (FLAG_ENABLED(light_state, LightState::Special1))     Lights.Special1 = true;
      if (FLAG_ENABLED(light_state, LightState::Special2))     Lights.Special2 = true;
      return Lights;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    /// Returns the current light state as an enum type
    LightState GetLightStateEnum() const {
      return static_cast<LightState>(light_state);
    }

    /// Returns the current light state as a value type
    flag_type GetLightStateAsValue() const {
      return light_state;
    }

    /// Lights state flag, all turned off by default
    flag_type light_state = static_cast<flag_type>(LightState::None);

    MSGPACK_DEFINE_ARRAY(light_state)

  };

} // namespace rpc
} // namespace carla
