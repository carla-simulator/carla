// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "WheeledVehicleMovementComponent.h"
#include "Curves/CurveFloat.h"
#include "WheeledVehicleMovementComponentNW.generated.h"

namespace physx
{
	class PxVehicleWheelData;
}

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
	float MaxRPM;

	/** Moment of inertia of the engine around the axis of rotation (Kgm^2). */
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.01", UIMin = "0.01"))
	float MOI;

	/** Damping rate of engine when full throttle is applied (Kgm^2/s) */
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DampingRateFullThrottle;

	/** Damping rate of engine in at zero throttle when the clutch is engaged (Kgm^2/s)*/
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DampingRateZeroThrottleClutchEngaged;

	/** Damping rate of engine in at zero throttle when the clutch is disengaged (in neutral gear) (Kgm^2/s)*/
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DampingRateZeroThrottleClutchDisengaged;

	/** Find the peak torque produced by the TorqueCurve */
	float FindPeakTorque() const;
};

USTRUCT()
struct FVehicleNWGearData
{
	GENERATED_USTRUCT_BODY()

	/** Determines the amount of torque multiplication*/
	UPROPERTY(EditAnywhere, Category = Setup)
	float Ratio;

	/** Value of engineRevs/maxEngineRevs that is low enough to gear down*/
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
	float DownRatio;

	/** Value of engineRevs/maxEngineRevs that is high enough to gear up*/
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"), Category = Setup)
	float UpRatio;
};

USTRUCT()
struct FVehicleNWTransmissionData
{
	GENERATED_USTRUCT_BODY()
	/** Whether to use automatic transmission */
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (DisplayName = "Automatic Transmission"))
	bool bUseGearAutoBox;

	/** Time it takes to switch gears (seconds) */
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float GearSwitchTime;

	/** Minimum time it takes the automatic transmission to initiate a gear change (seconds)*/
	UPROPERTY(EditAnywhere, Category = Setup, meta = (editcondition = "bUseGearAutoBox", ClampMin = "0.0", UIMin = "0.0"))
	float GearAutoBoxLatency;

	/** The final gear ratio multiplies the transmission gear ratios.*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
	float FinalRatio;

	/** Forward gear ratios (up to 30) */
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay)
	TArray<FVehicleNWGearData> ForwardGears;

	/** Reverse gear ratio */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup)
	float ReverseGearRatio;

	/** Value of engineRevs/maxEngineRevs that is high enough to increment gear*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Setup, meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float NeutralGearUpRatio;

	/** Strength of clutch (Kgm^2/s)*/
	UPROPERTY(EditAnywhere, Category = Setup, AdvancedDisplay, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ClutchStrength;
};

UCLASS(ClassGroup = (Physics), meta = (BlueprintSpawnableComponent), hidecategories = (PlanarMovement, "Components|Movement|Planar", Activation, "Components|Activation"))
class CARLA_API UWheeledVehicleMovementComponentNW : public UWheeledVehicleMovementComponent
{
	GENERATED_UCLASS_BODY()

		/** Engine */
	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
	FVehicleNWEngineData EngineSetup;

	/** Differential */
	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
	TArray<FVehicleNWWheelDifferentialData> DifferentialSetup;

	/** Transmission data */
	UPROPERTY(EditAnywhere, Category = MechanicalSetup)
	FVehicleNWTransmissionData TransmissionSetup;

	/** Maximum steering versus forward speed (km/h) */
	UPROPERTY(EditAnywhere, Category = SteeringSetup)
	FRuntimeFloatCurve SteeringCurve;

	virtual void Serialize(FArchive& Ar) override;
	virtual void ComputeConstants() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

	virtual const void* GetTireData(physx::PxVehicleWheels* Wheels, UVehicleWheel* Wheel);
	virtual const int32 GetWheelShapeMapping(physx::PxVehicleWheels* Wheels, uint32 Wheel);
	virtual const physx::PxVehicleWheelData GetWheelData(physx::PxVehicleWheels* Wheels, uint32 Wheel);

	/** Allocate and setup the PhysX vehicle */
	virtual void SetupVehicle() override;

	virtual int32 GetCustomGearBoxNumForwardGears() const;

	virtual void UpdateSimulation(float DeltaTime) override;

	/** update simulation data: engine */
	virtual void UpdateEngineSetup(const FVehicleNWEngineData& NewEngineSetup);

	/** update simulation data: differential */
	virtual void UpdateDifferentialSetup(const TArray<FVehicleNWWheelDifferentialData>& NewDifferentialSetup);

	/** update simulation data: transmission */
	virtual void UpdateTransmissionSetup(const FVehicleNWTransmissionData& NewGearSetup);
};
