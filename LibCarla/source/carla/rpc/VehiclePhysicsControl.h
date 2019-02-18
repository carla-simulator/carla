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
    
    explicit VehiclePhysicsControl(
      const std::vector<geom::Location>& in_torque_curve,
      float in_max_rpm,
      float in_moi,
      float in_damping_rate_full_throttle,
      float in_damping_rate_zero_throttle_clutch_engaged,
      float in_damping_rate_zero_throttle_clutch_disengaged,

      bool  in_use_gear_autobox,
      float in_gear_switch_time,
      float in_clutch_strength,

      float in_mass,
      float in_drag_coefficient,
      geom::Vector3D in_inertia_tensor_scale
    ) {

      torque_curve = in_torque_curve;
      max_rpm = in_max_rpm;
      moi = in_moi;
      damping_rate_full_throttle = in_damping_rate_full_throttle;
      damping_rate_zero_throttle_clutch_engaged = in_damping_rate_zero_throttle_clutch_engaged;
      damping_rate_zero_throttle_clutch_disengaged = in_damping_rate_zero_throttle_clutch_disengaged;

      use_gear_autobox = in_use_gear_autobox;
      gear_switch_time = in_gear_switch_time;
      clutch_strength = in_clutch_strength;

      mass = in_mass;
      drag_coefficient = in_drag_coefficient;
      inertia_tensor_scale = in_inertia_tensor_scale;
    }

    const std::vector<geom::Location> GetTorqueCurve() const {
      return torque_curve;
    }

    void SetTorqueCurve(std::vector<geom::Location> &in_torque_curve) {
      torque_curve = in_torque_curve;
    }

    std::vector<geom::Location> torque_curve;
    float max_rpm = 0.0f;
    float moi = 0.0f;
    float damping_rate_full_throttle = 0.0f;
    float damping_rate_zero_throttle_clutch_engaged = 0.0f;
    float damping_rate_zero_throttle_clutch_disengaged = 0.0f;

    bool use_gear_autobox = true;
    float gear_switch_time = 0.0f;
    float clutch_strength = 0.0f;

    float mass = 0.0f;
    float drag_coefficient = 0.0f;
    geom::Vector3D inertia_tensor_scale;
   
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

      // Transmission Setup
      use_gear_autobox = Control.bUseGearAutoBox;
      gear_switch_time = Control.GearSwitchTime;
      clutch_strength = Control.ClutchStrength;

      // Vehicle Setup
      mass = Control.Mass;
      drag_coefficient = Control.DragCoefficient;
      inertia_tensor_scale = Control.InertiaTensorScale;
      
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

      // Transmission Setup
      Control.bUseGearAutoBox = use_gear_autobox;
      Control.GearSwitchTime = gear_switch_time;
      Control.ClutchStrength = clutch_strength;

      // Vehicle Setup
      Control.Mass = mass;
      Control.DragCoefficient = drag_coefficient;
      Control.InertiaTensorScale = inertia_tensor_scale;

      return Control;
    }

    #endif

    MSGPACK_DEFINE_ARRAY(torque_curve, 
                        max_rpm, 
                        moi, 
                        damping_rate_full_throttle, 
                        damping_rate_zero_throttle_clutch_engaged, 
                        damping_rate_zero_throttle_clutch_disengaged,
                        use_gear_autobox,
                        gear_switch_time,
                        clutch_strength,
                        mass,
                        drag_coefficient,
                        inertia_tensor_scale);
  };

} // namespace rpc
} // namespace carla
