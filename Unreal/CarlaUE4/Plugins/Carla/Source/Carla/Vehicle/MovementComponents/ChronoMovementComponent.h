// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "BaseCarlaMovementComponent.h"
#include "Carla/Vehicle/VehicleControl.h"

#ifdef WITH_CHRONO
#include "compiler/disable-ue4-macros.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"

#include "chrono/physics/ChSystemNSC.h"
#include "chrono_vehicle/ChVehicleModelData.h"
#include "chrono_vehicle/terrain/RigidTerrain.h"
#include "chrono_vehicle/driver/ChDataDriver.h"
#include "chrono_models/vehicle/hmmwv/HMMWV.h"

#pragma clang diagnostic pop
#include "compiler/enable-ue4-macros.h"
#endif

#include "ChronoMovementComponent.generated.h"

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent) )
class CARLA_API UChronoMovementComponent : public UBaseCarlaMovementComponent
{
  GENERATED_BODY()

#ifdef WITH_CHRONO
  chrono::ChSystemNSC sys;
  chrono::vehicle::hmmwv::HMMWV_Full my_hmmwv;
  std::shared_ptr<chrono::vehicle::RigidTerrain> terrain;
#endif

public:


  static void CreateChronoMovementComponent(ACarlaWheeledVehicle* Vehicle);

  #ifdef WITH_CHRONO
  virtual void BeginPlay() override;

  void ProcessControl(FVehicleControl &Control) override;

  void TickComponent(float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  void AdvanceChronoSimulation(float StepSize);
  #endif
};
