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


    const std::vector<geom::Location> &GetTorqueCurve() const {
      return torque_curve;
    }

    #ifdef LIBCARLA_INCLUDED_FROM_UE4

    VehiclePhysicsControl(const FVehiclePhysicsControl &Control) {      
      TArray<FRichCurveKey> CurveKeys = Control.TorqueCurve.GetCopyOfKeys();
      for(int32 KeyIdx = 0; KeyIdx < CurveKeys.Num(); KeyIdx++)
      {
          geom::Location point(CurveKeys[KeyIdx].Time, CurveKeys[KeyIdx].Value, 0.0f);
          torque_curve.push_back(point);
      }
    }

    operator FVehiclePhysicsControl() const {
      FVehiclePhysicsControl Control;
      FRichCurve TorqueCurve;
      for (auto location : torque_curve) {
          TorqueCurve.AddKey (location.x, location.y);
      }
      Control.TorqueCurve = TorqueCurve;
      return Control;
    }

    #endif

    MSGPACK_DEFINE_ARRAY(torque_curve);
  };

} // namespace rpc
} // namespace carla
