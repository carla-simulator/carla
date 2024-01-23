// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Vehicle/AckermannControllerSettings.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class AckermannControllerSettings {
  public:

    AckermannControllerSettings() = default;

    AckermannControllerSettings(
        float speed_kp,
        float speed_ki,
        float speed_kd,
        float accel_kp,
        float accel_ki,
        float accel_kd)
      : speed_kp(speed_kp),
        speed_ki(speed_ki),
        speed_kd(speed_kd),
        accel_kp(accel_kp),
        accel_ki(accel_ki),
        accel_kd(accel_kd) {}

    float speed_kp = 0.0f;
    float speed_ki = 0.0f;
    float speed_kd = 0.0f;
    float accel_kp = 0.0f;
    float accel_ki = 0.0f;
    float accel_kd = 0.0f;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    AckermannControllerSettings(const FAckermannControllerSettings &Settings)
      : speed_kp(Settings.SpeedKp),
        speed_ki(Settings.SpeedKi),
        speed_kd(Settings.SpeedKd),
        accel_kp(Settings.AccelKp),
        accel_ki(Settings.AccelKi),
        accel_kd(Settings.AccelKd) {}

    operator FAckermannControllerSettings() const {
      FAckermannControllerSettings Settings;
      Settings.SpeedKp = speed_kp;
      Settings.SpeedKi = speed_ki;
      Settings.SpeedKd = speed_kd;
      Settings.AccelKp = accel_kp;
      Settings.AccelKi = accel_ki;
      Settings.AccelKd = accel_kd;
      return Settings;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    bool operator!=(const AckermannControllerSettings &rhs) const {
      return
          speed_kp != rhs.speed_kp ||
          speed_ki != rhs.speed_ki ||
          speed_kd != rhs.speed_kd ||
          accel_kp != rhs.accel_kp ||
          accel_ki != rhs.accel_ki ||
          accel_kd != rhs.accel_kd;
    }

    bool operator==(const AckermannControllerSettings &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(
        speed_kp,
        speed_ki,
        speed_kd,
        accel_kp,
        accel_ki,
        accel_kd
    );
  };

} // namespace rpc
} // namespace carla
