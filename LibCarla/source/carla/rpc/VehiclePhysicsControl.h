// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/geom/Location.h"

#include <string>
#include <vector>

namespace carla {
namespace rpc {

  class VehiclePhysicsControl {
  public:

    
    explicit VehiclePhysicsControl() = default;
    
    std::vector<geom::Location> torque_curve;
    float max_rpm = 0.0f;
    float moi = 0.0f;
    float damping_rate_full_throttle = 0.0f;
    float damping_rate_zero_throttle_clutch_engaged = 0.0f;
    float damping_rate_zero_throttle_clutch_disengaged = 0.0f;

    #ifdef LIBCARLA_INCLUDED_FROM_UE4

    VehiclePhysicsControl(const FVehiclePhysicsControl &Control) {      
      // Engine Setup
      TArray<FRichCurveKey> CurveKeys = Control.TorqueCurve.GetCopyOfKeys();
      for(int32 KeyIdx = 0; KeyIdx < CurveKeys.Num(); KeyIdx++)
      {
          geom::Location point(CurveKeys[KeyIdx].Time, CurveKeys[KeyIdx].Value, 0.0f);
          torque_curve.push_back(point);
      }
      max_rpm = Control.MaxRPM;
      moi = Control.MOI;
      damping_rate_full_throttle = Control.DampingRateFullThrottle;
      damping_rate_zero_throttle_clutch_engaged = Control.DampingRateZeroThrottleClutchEngaged;
      damping_rate_zero_throttle_clutch_disengaged = Control.DampingRateZeroThrottleClutchDisengaged;
    }

    operator FVehiclePhysicsControl() const {
      FVehiclePhysicsControl Control;

      // Engine Setup
      FRichCurve TorqueCurve;
      for (auto location : torque_curve) {
          TorqueCurve.AddKey (location.x, location.y);
      }
      Control.TorqueCurve = TorqueCurve;
      Control.MaxRPM = max_rpm;
      Control.MOI = moi;
      Control.DampingRateFullThrottle = damping_rate_full_throttle;
      Control.DampingRateZeroThrottleClutchEngaged= damping_rate_zero_throttle_clutch_engaged;
      Control.DampingRateZeroThrottleClutchDisengaged = damping_rate_zero_throttle_clutch_disengaged;

      return Control;
    }

    #endif

    MSGPACK_DEFINE_ARRAY(torque_curve, 
                        max_rpm, 
                        moi, 
                        damping_rate_full_throttle, 
                        damping_rate_zero_throttle_clutch_engaged, 
                        damping_rate_zero_throttle_clutch_disengaged);
  };

} // namespace rpc
} // namespace carla
