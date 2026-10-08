// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "BaseCarlaMovementComponent.h"
#include "Carla/Vehicle/VehicleControl.h"

#ifdef WITH_CHRONO
#include <util/disable-ue4-macros.h>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wshadow"
#endif

#include "chrono/physics/ChSystemNSC.h"
#include "chrono_vehicle/ChVehicleDataPath.h"
#include "chrono_vehicle/ChTerrain.h"
#include "chrono_vehicle/driver/ChDataDriver.h"
#include "chrono_vehicle/wheeled_vehicle/vehicle/WheeledVehicle.h"

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#include <util/enable-ue4-macros.h>
#endif

#include "ChronoMovementComponent.generated.h"

#ifdef WITH_CHRONO
class UERayCastTerrain : public chrono::vehicle::ChTerrain
{
  // Used where the trace finds no physical material: ChTerrain's own default.
  static constexpr float DefaultFriction = 0.8f;
  ACarlaWheeledVehicle* CarlaVehicle;
  chrono::vehicle::ChVehicle* ChronoVehicle;
public:
  UERayCastTerrain(ACarlaWheeledVehicle* UEVehicle, chrono::vehicle::ChVehicle* ChrVehicle);

  std::pair<bool, FHitResult> GetTerrainProperties(const FVector &Location) const;
  virtual double GetHeight(const chrono::ChVector3d& loc) const override;
  virtual chrono::ChVector3d GetPoint(const chrono::ChVector3d& loc) const override;
  virtual chrono::ChVector3d GetNormal(const chrono::ChVector3d& loc) const override;
  virtual float GetCoefficientFriction(const chrono::ChVector3d& loc) const override;
  virtual void GetProperties(const chrono::ChVector3d& loc,
                             chrono::ChVector3d& point,
                             double& height,
                             chrono::ChVector3d& normal,
                             float& friction) const override;
};
#endif

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent) )
class CARLA_API UChronoMovementComponent : public UBaseCarlaMovementComponent
{
  GENERATED_BODY()

#ifdef WITH_CHRONO
  chrono::ChSystemNSC Sys;
  std::shared_ptr<chrono::vehicle::WheeledVehicle> Vehicle;
  std::shared_ptr<UERayCastTerrain> Terrain;
#endif

  uint64_t MaxSubsteps = 10;
  float MaxSubstepDeltaTime = 0.01;
  FVehicleControl VehicleControl;
  // Defaults: the sedan templates under Co-Simulation/Chrono/Vehicles/, which
  // is also where an empty base path points (see CreateChronoMovementComponent).
  FString VehicleJSON =    "sedan/vehicle/Sedan_Vehicle.json";
  FString PowertrainJSON = "sedan/powertrain/Sedan_SimpleMapPowertrain.json";
  FString TireJSON =       "sedan/tire/Sedan_TMeasyTire.json";
  FString BaseJSONPath = "";
  // Resolved from the powertrain template by CreateChronoMovementComponent.
  FString EngineJSON = "";
  FString TransmissionJSON = "";

public:


  // Replaces the vehicle's movement component with a Chrono one. Returns an
  // empty string on success, or why Chrono could not be enabled, in which
  // case the vehicle keeps the physics it had.
  static FString CreateChronoMovementComponent(
      ACarlaWheeledVehicle* Vehicle,
      uint64_t MaxSubsteps,
      float MaxSubstepDeltaTime,
      FString VehicleJSON = "",
      FString PowertrainJSON = "",
      FString TireJSON = "",
      FString BaseJSONPath = "");

  #ifdef WITH_CHRONO
  virtual void BeginPlay() override;

  // Builds the Chrono system and vehicle from the templates. Throws
  // std::exception on a template Chrono cannot use.
  void InitializeChronoVehicle();

  void ProcessControl(FVehicleControl &Control) override;

  void TickComponent(float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  void AdvanceChronoSimulation(float StepSize);

  virtual FVector GetVelocity() const override;

  virtual int32 GetVehicleCurrentGear() const override;

  virtual float GetVehicleForwardSpeed() const override;

  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  #endif

  virtual void DisableSpecialPhysics() override;

private:

  // Hands the vehicle back to Chaos. It keeps the velocity Chrono gave it
  // unless bResetVelocity, for a Chrono state that cannot be trusted.
  void DisableChronoPhysics(bool bResetVelocity = false);

  UFUNCTION()
  void OnVehicleHit(AActor *Actor,
      AActor *OtherActor,
      FVector NormalImpulse,
      const FHitResult &Hit);

  // On car mesh overlap, only works when carsim is enabled
  // (this event triggers when overlapping with static environment)
  UFUNCTION()
  void OnVehicleOverlap(UPrimitiveComponent* OverlappedComponent,
      AActor* OtherActor,
      UPrimitiveComponent* OtherComp,
      int32 OtherBodyIndex,
      bool bFromSweep,
      const FHitResult & SweepResult);
};
