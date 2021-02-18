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

#include "chrono/physics/ChSystemNSC.h"
#include "chrono_vehicle/ChVehicleModelData.h"
#include "chrono_vehicle/ChTerrain.h"
#include "chrono_vehicle/driver/ChDataDriver.h"
#include "chrono_models/vehicle/hmmwv/HMMWV.h"

#include "compiler/enable-ue4-macros.h"
#endif

#include "ChronoMovementComponent.generated.h"

#ifdef WITH_CHRONO
class UERayCastTerrain : public chrono::vehicle::ChTerrain
{
  ACarlaWheeledVehicle* CarlaVehicle;
  chrono::vehicle::ChVehicle* ChronoVehicle;
public:
  UERayCastTerrain(ACarlaWheeledVehicle* UEVehicle, chrono::vehicle::ChVehicle* ChrVehicle);

  std::pair<bool, FHitResult> GetTerrainProperties(const FVector &Location) const;
  virtual double GetHeight(const chrono::ChVector<>& loc) const override;
  virtual chrono::ChVector<> GetNormal(const chrono::ChVector<>& loc) const override;
  virtual float GetCoefficientFriction(const chrono::ChVector<>& loc) const override;
};
#endif

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent) )
class CARLA_API UChronoMovementComponent : public UBaseCarlaMovementComponent
{
  GENERATED_BODY()

#ifdef WITH_CHRONO
  chrono::ChSystemNSC sys;
  chrono::vehicle::hmmwv::HMMWV_Full my_hmmwv;
  std::shared_ptr<UERayCastTerrain> terrain;
#endif

  uint64_t MaxSubsteps = 10;
  float MaxSubstepDeltaTime = 0.01;
  FVehicleControl VehicleControl;

public:


  static void CreateChronoMovementComponent(ACarlaWheeledVehicle* Vehicle, uint64_t MaxSubsteps, float MaxSubstepDeltaTime);

  #ifdef WITH_CHRONO
  virtual void BeginPlay() override;

  void ProcessControl(FVehicleControl &Control) override;

  void TickComponent(float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  void AdvanceChronoSimulation(float StepSize);
  #endif
};
