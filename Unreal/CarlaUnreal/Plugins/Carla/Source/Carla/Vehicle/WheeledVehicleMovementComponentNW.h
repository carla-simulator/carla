// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Curves/CurveFloat.h"
#include <util/ue-header-guard-end.h>

#include "WheeledVehicleMovementComponentNW.generated.h"

USTRUCT()
struct FVehicleNWWheelDifferentialData
{
  GENERATED_USTRUCT_BODY()

    /** If True, torque is applied to this wheel */
    UPROPERTY(EditAnywhere, Category = Setup)
    bool bDriven;

  FVehicleNWWheelDifferentialData()
    : bDriven(true)
  { }
};

USTRUCT()
struct FVehicleNWEngineData
{
  GENERATED_USTRUCT_BODY()

  /** Torque (Nm) at a given RPM*/
  UPROPERTY(EditAnywhere, Category = Setup)
  FRuntimeFloatCurve TorqueCurve;

  /** Maximum revolutions per minute of the engine */
  UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
  float MaxRPM = 0.0F;

  /** Moment of inertia of the engine around the axis of rotation (Kgm^2). */
  UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
  float MOI = 0.0F;

  /** Damping rate of engine when full throttle is applied (Kgm^2/s) */
  UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
  float DampingRateFullThrottle = 0.0F;

  /** Damping rate of engine in at zero throttle when the clutch is engaged (Kgm^2/s)*/
  UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
  float DampingRateZeroThrottleClutchEngaged = 0.0F;

  /** Damping rate of engine in at zero throttle when the clutch is disengaged (in neutral gear) (Kgm^2/s)*/
  UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
  float DampingRateZeroThrottleClutchDisengaged = 0.0F;

  /** Find the peak torque produced by the TorqueCurve */
  float FindPeakTorque() const;
};

USTRUCT()
struct FVehicleNWGearData
{
  GENERATED_USTRUCT_BODY()

  /** Determines the amount of torque multiplication*/
  UPROPERTY(EditAnywhere, Category = Setup)
  float Ratio = 0.0F;

  /** Value of engineRevs/maxEngineRevs that is low enough to gear down*/
  UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
  float DownRatio = 0.0F;

  /** Value of engineRevs/maxEngineRevs that is high enough to gear up*/
  UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
  float UpRatio = 0.0F;
};

USTRUCT()
struct FVehicleNWTransmissionData
{
  GENERATED_USTRUCT_BODY()
  /** Whether to use automatic transmission */
  UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (DisplayName = "Automatic Transmission"))
  bool bUseAutomaticGears = false;

  /** Time it takes to switch gears (seconds) */
  UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0"))
  float GearChangeTime = 0.0F;

  /** Minimum time it takes the automatic transmission to initiate a gear change (seconds)*/
  UPROPERTY(EditAnywhere, Category = Setup, meta = (editcondition = "bUseAutomaticGears", ClampMin = "0.0", UIMin = "0.0"))
  float GearAutoBoxLatency = 0.0F;

  /** The final gear ratio multiplies the transmission gear ratios.*/
  UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
  float FinalRatio = 0.0F;

  /** Forward gear ratios (up to 30) */
  UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay)
  TArray<FVehicleNWGearData> ForwardGearRatios;

  /** Reverse gear ratio */
  UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
  float ReverseGearRatio = 0.0F;

  /** Value of engineRevs/maxEngineRevs that is high enough to increment gear*/
  UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
  float NeutralGearUpRatio = 0.0F;

  /** Strength of clutch (Kgm^2/s)*/
  UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
  float ClutchStrength = 0.0F;
};

UCLASS(ClassGroup = (Physics), meta = (BlueprintSpawnableComponent), hidecategories = (PlanarMovement, "Components|Movement|Planar", Activation, "Components|Activation"))
class CARLA_API UWheeledVehicleMovementComponentNW :
  public UChaosWheeledVehicleMovementComponent
{
  GENERATED_UCLASS_BODY()

    /** Engine */
  //UPROPERTY(EditAnywhere, Category = MechanicalSetup)
  //FVehicleNWEngineData EngineSetupEXT;

  /** Differential */
  //UPROPERTY(EditAnywhere, Category = MechanicalSetup)
  //TArray<FVehicleNWWheelDifferentialData> DifferentialSetupEXT;

  /** Transmission data */
  //UPROPERTY(EditAnywhere, Category = MechanicalSetup)
  //FVehicleNWTransmissionData TransmissionSetupEXT;

  /** Maximum steering versus forward speed (km/h) */
  //UPROPERTY(EditAnywhere, Category = SteeringSetup)
  //FRuntimeFloatCurve SteeringCurve;

#if WITH_EDITOR
  virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

};
