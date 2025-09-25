// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WheeledVehicleMovementComponentNW.h"
#include "PhysicsPublic.h"
#include "PhysXPublic.h"
#include "PhysXVehicleManager.h"
#include "Components/PrimitiveComponent.h"
#include "Logging/MessageLog.h"

UWheeledVehicleMovementComponentNW::UWheeledVehicleMovementComponentNW(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// grab default values from physx
	PxVehicleEngineData DefEngineData;
	EngineSetup.MOI = DefEngineData.mMOI;
	EngineSetup.MaxRPM = OmegaToRPM(DefEngineData.mMaxOmega);
	EngineSetup.DampingRateFullThrottle = DefEngineData.mDampingRateFullThrottle;
	EngineSetup.DampingRateZeroThrottleClutchEngaged = DefEngineData.mDampingRateZeroThrottleClutchEngaged;
	EngineSetup.DampingRateZeroThrottleClutchDisengaged = DefEngineData.mDampingRateZeroThrottleClutchDisengaged;

	// Convert from PhysX curve to ours
	FRichCurve* TorqueCurveData = EngineSetup.TorqueCurve.GetRichCurve();
	for (PxU32 KeyIdx = 0; KeyIdx < DefEngineData.mTorqueCurve.getNbDataPairs(); ++KeyIdx)
	{
		float Input = DefEngineData.mTorqueCurve.getX(KeyIdx) * EngineSetup.MaxRPM;
		float Output = DefEngineData.mTorqueCurve.getY(KeyIdx) * DefEngineData.mPeakTorque;
		TorqueCurveData->AddKey(Input, Output);
	}

	PxVehicleClutchData DefClutchData;
	TransmissionSetup.ClutchStrength = DefClutchData.mStrength;

	PxVehicleGearsData DefGearSetup;
	TransmissionSetup.GearSwitchTime = DefGearSetup.mSwitchTime;
	TransmissionSetup.ReverseGearRatio = DefGearSetup.mRatios[PxVehicleGearsData::eREVERSE];
	TransmissionSetup.FinalRatio = DefGearSetup.mFinalRatio;

	PxVehicleAutoBoxData DefAutoBoxSetup;
	TransmissionSetup.NeutralGearUpRatio = DefAutoBoxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL];
	TransmissionSetup.GearAutoBoxLatency = DefAutoBoxSetup.getLatency();
	TransmissionSetup.bUseGearAutoBox = true;

	for (uint32 i = PxVehicleGearsData::eFIRST; i < DefGearSetup.mNbRatios; ++i)
	{
		FVehicleNWGearData GearData;
		GearData.DownRatio = DefAutoBoxSetup.mDownRatios[i];
		GearData.UpRatio = DefAutoBoxSetup.mUpRatios[i];
		GearData.Ratio = DefGearSetup.mRatios[i];
		TransmissionSetup.ForwardGears.Add(GearData);
	}

	// Init steering speed curve
	FRichCurve* SteeringCurveData = SteeringCurve.GetRichCurve();
	SteeringCurveData->AddKey(0.0f, 1.0f);
	SteeringCurveData->AddKey(20.0f, 0.9f);
	SteeringCurveData->AddKey(60.0f, 0.8f);
	SteeringCurveData->AddKey(120.0f, 0.7f);

	// Initialize WheelSetups array with 4 wheels, this can be modified via editor later
	const int32 NbrWheels = 4;
	WheelSetups.SetNum(NbrWheels);
	DifferentialSetup.SetNum(NbrWheels);

	IdleBrakeInput = 10;
}

#if WITH_EDITOR
void UWheeledVehicleMovementComponentNW::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == TEXT("DownRatio"))
	{
		for (int32 GearIdx = 0; GearIdx < TransmissionSetup.ForwardGears.Num(); ++GearIdx)
		{
			FVehicleNWGearData& GearData = TransmissionSetup.ForwardGears[GearIdx];
			GearData.DownRatio = FMath::Min(GearData.DownRatio, GearData.UpRatio);
		}
	}
	else if (PropertyName == TEXT("UpRatio"))
	{
		for (int32 GearIdx = 0; GearIdx < TransmissionSetup.ForwardGears.Num(); ++GearIdx)
		{
			FVehicleNWGearData& GearData = TransmissionSetup.ForwardGears[GearIdx];
			GearData.UpRatio = FMath::Max(GearData.DownRatio, GearData.UpRatio);
		}
	}
	else if (PropertyName == TEXT("SteeringCurve"))
	{
		//make sure values are capped between 0 and 1
		TArray<FRichCurveKey> SteerKeys = SteeringCurve.GetRichCurve()->GetCopyOfKeys();
		for (int32 KeyIdx = 0; KeyIdx < SteerKeys.Num(); ++KeyIdx)
		{
			float NewValue = FMath::Clamp(SteerKeys[KeyIdx].Value, 0.0f, 1.0f);
			SteeringCurve.GetRichCurve()->UpdateOrAddKey(SteerKeys[KeyIdx].Time, NewValue);
		}
	}
}
#endif

static void GetVehicleDifferentialNWSetup(const TArray<FVehicleNWWheelDifferentialData>& Setup, PxVehicleDifferentialNWData& PxSetup)
{
	for (int32 i = 0; i < Setup.Num(); ++i)
	{
		PxSetup.setDrivenWheel(i, Setup[i].bDriven);
	}
}

float FVehicleNWEngineData::FindPeakTorque() const
{
	// Find max torque
	float PeakTorque = 0.0f;
	TArray<FRichCurveKey> TorqueKeys = TorqueCurve.GetRichCurveConst()->GetCopyOfKeys();
	for (int32 KeyIdx = 0; KeyIdx < TorqueKeys.Num(); ++KeyIdx)
	{
		FRichCurveKey& Key = TorqueKeys[KeyIdx];
		PeakTorque = FMath::Max(PeakTorque, Key.Value);
	}
	return PeakTorque;
}

static void GetVehicleEngineSetup(const FVehicleNWEngineData& Setup, PxVehicleEngineData& PxSetup)
{
	PxSetup.mMOI = M2ToCm2(Setup.MOI);
	PxSetup.mMaxOmega = RPMToOmega(Setup.MaxRPM);
	PxSetup.mDampingRateFullThrottle = M2ToCm2(Setup.DampingRateFullThrottle);
	PxSetup.mDampingRateZeroThrottleClutchEngaged = M2ToCm2(Setup.DampingRateZeroThrottleClutchEngaged);
	PxSetup.mDampingRateZeroThrottleClutchDisengaged = M2ToCm2(Setup.DampingRateZeroThrottleClutchDisengaged);

	float PeakTorque = Setup.FindPeakTorque(); // In Nm
	PxSetup.mPeakTorque = M2ToCm2(PeakTorque);	// convert Nm to (kg cm^2/s^2)

	// Convert from our curve to PhysX
	PxSetup.mTorqueCurve.clear();
	TArray<FRichCurveKey> TorqueKeys = Setup.TorqueCurve.GetRichCurveConst()->GetCopyOfKeys();
	int32 NumTorqueCurveKeys = FMath::Min<int32>(TorqueKeys.Num(), PxVehicleEngineData::eMAX_NB_ENGINE_TORQUE_CURVE_ENTRIES);
	for (int32 KeyIdx = 0; KeyIdx < NumTorqueCurveKeys; ++KeyIdx)
	{
		FRichCurveKey& Key = TorqueKeys[KeyIdx];
		PxSetup.mTorqueCurve.addPair(FMath::Clamp(Key.Time / Setup.MaxRPM, 0.0f, 1.0f), Key.Value / PeakTorque); // Normalize torque to 0-1 range
	}
}

static void GetVehicleGearSetup(const FVehicleNWTransmissionData& Setup, PxVehicleGearsData& PxSetup)
{
	PxSetup.mSwitchTime = Setup.GearSwitchTime;
	PxSetup.mRatios[PxVehicleGearsData::eREVERSE] = Setup.ReverseGearRatio;
	for (int32 i = 0; i < Setup.ForwardGears.Num(); i++)
	{
		PxSetup.mRatios[i + PxVehicleGearsData::eFIRST] = Setup.ForwardGears[i].Ratio;
	}
	PxSetup.mFinalRatio = Setup.FinalRatio;
	PxSetup.mNbRatios = Setup.ForwardGears.Num() + PxVehicleGearsData::eFIRST;
}

static void GetVehicleAutoBoxSetup(const FVehicleNWTransmissionData& Setup, PxVehicleAutoBoxData& PxSetup)
{
	for (int32 i = 0; i < Setup.ForwardGears.Num(); ++i)
	{
		const FVehicleNWGearData& GearData = Setup.ForwardGears[i];
		PxSetup.mUpRatios[i + PxVehicleGearsData::eFIRST] = GearData.UpRatio;
		PxSetup.mDownRatios[i + PxVehicleGearsData::eFIRST] = GearData.DownRatio;
	}
	PxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL] = Setup.NeutralGearUpRatio;
	PxSetup.setLatency(Setup.GearAutoBoxLatency);

}

int32 UWheeledVehicleMovementComponentNW::GetCustomGearBoxNumForwardGears() const
{
	return TransmissionSetup.ForwardGears.Num();
}

void SetupDriveHelper(const UWheeledVehicleMovementComponentNW* VehicleData, const PxVehicleWheelsSimData* PWheelsSimData, PxVehicleDriveSimDataNW& DriveData)
{
	PxVehicleDifferentialNWData DifferentialSetup;
	GetVehicleDifferentialNWSetup(VehicleData->DifferentialSetup, DifferentialSetup);

	DriveData.setDiffData(DifferentialSetup);

	PxVehicleEngineData EngineSetup;
	GetVehicleEngineSetup(VehicleData->EngineSetup, EngineSetup);
	DriveData.setEngineData(EngineSetup);

	PxVehicleClutchData ClutchSetup;
	ClutchSetup.mStrength = M2ToCm2(VehicleData->TransmissionSetup.ClutchStrength);
	DriveData.setClutchData(ClutchSetup);

	PxVehicleGearsData GearSetup;
	GetVehicleGearSetup(VehicleData->TransmissionSetup, GearSetup);
	DriveData.setGearsData(GearSetup);

	PxVehicleAutoBoxData AutoBoxSetup;
	GetVehicleAutoBoxSetup(VehicleData->TransmissionSetup, AutoBoxSetup);
	DriveData.setAutoBoxData(AutoBoxSetup);
}

void UWheeledVehicleMovementComponentNW::SetupVehicle()
{
	if (!UpdatedPrimitive)
	{
		return;
	}

	if (WheelSetups.Num() < 2)
	{
		PVehicle = nullptr;
		PVehicleDrive = nullptr;
		return;
	}

	for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx)
	{
		const FWheelSetup& WheelSetup = WheelSetups[WheelIdx];
		if (WheelSetup.BoneName == NAME_None)
		{
			return;
		}
	}

	// Setup the chassis and wheel shapes
	SetupVehicleShapes();

	// Setup mass properties
	SetupVehicleMass();

	// Setup the wheels
	PxVehicleWheelsSimData* PWheelsSimData = PxVehicleWheelsSimData::allocate(WheelSetups.Num());
	SetupWheels(PWheelsSimData);

	// Setup drive data
	PxVehicleDriveSimDataNW DriveData;
	SetupDriveHelper(this, PWheelsSimData, DriveData);

	// Create the vehicle
	PxVehicleDriveNW* PVehicleDriveNW = PxVehicleDriveNW::allocate(WheelSetups.Num());
	check(PVehicleDriveNW);

	FBodyInstance* TargetInstance = UpdatedPrimitive->GetBodyInstance();

	FPhysicsCommand::ExecuteWrite(TargetInstance->ActorHandle, [&](const FPhysicsActorHandle& Actor)
		{
			PxRigidActor* PActor = FPhysicsInterface::GetPxRigidActor_AssumesLocked(Actor);
			if (!PActor)
			{
				return;
			}

			if (PxRigidDynamic* PVehicleActor = PActor->is<PxRigidDynamic>())
			{
				PVehicleDriveNW->setup(GPhysXSDK, PVehicleActor, *PWheelsSimData, DriveData, 0);
				PVehicleDriveNW->setToRestState();

				// cleanup
				PWheelsSimData->free();
			}
		});

	PWheelsSimData = nullptr;

	// cache values
	PVehicle = PVehicleDriveNW;
	PVehicleDrive = PVehicleDriveNW;

	SetUseAutoGears(TransmissionSetup.bUseGearAutoBox);

}

void UWheeledVehicleMovementComponentNW::UpdateSimulation(float DeltaTime)
{
	if (PVehicleDrive == nullptr)
		return;

	FBodyInstance* TargetInstance = UpdatedPrimitive->GetBodyInstance();

	FPhysicsCommand::ExecuteWrite(TargetInstance->ActorHandle, [&](const FPhysicsActorHandle& Actor)
	{
		PxVehicleDriveNWRawInputData RawInputData;
		RawInputData.setAnalogAccel(ThrottleInput);
		RawInputData.setAnalogSteer(SteeringInput);
		RawInputData.setAnalogBrake(BrakeInput);
		RawInputData.setAnalogHandbrake(HandbrakeInput);

		if (!PVehicleDrive->mDriveDynData.getUseAutoGears())
		{
			RawInputData.setGearUp(bRawGearUpInput);
			RawInputData.setGearDown(bRawGearDownInput);
		}

		// Convert from our curve to PxFixedSizeLookupTable
		PxFixedSizeLookupTable<8> SpeedSteerLookup;
		TArray<FRichCurveKey> SteerKeys = SteeringCurve.GetRichCurve()->GetCopyOfKeys();
		const int32 MaxSteeringSamples = FMath::Min(8, SteerKeys.Num());
		for (int32 KeyIdx = 0; KeyIdx < MaxSteeringSamples; KeyIdx++)
		{
			FRichCurveKey& Key = SteerKeys[KeyIdx];
			SpeedSteerLookup.addPair(KmHToCmS(Key.Time), FMath::Clamp(Key.Value, 0.0f, 1.0f));
		}

		PxVehiclePadSmoothingData SmoothData = {
			{ ThrottleInputRate.RiseRate, BrakeInputRate.RiseRate, HandbrakeInputRate.RiseRate, SteeringInputRate.RiseRate, SteeringInputRate.RiseRate },
			{ ThrottleInputRate.FallRate, BrakeInputRate.FallRate, HandbrakeInputRate.FallRate, SteeringInputRate.FallRate, SteeringInputRate.FallRate }
		};

		PxVehicleDriveNW* PVehicleDriveNW = (PxVehicleDriveNW*)PVehicleDrive;
		PxVehicleDriveNWSmoothAnalogRawInputsAndSetAnalogInputs(SmoothData, SpeedSteerLookup, RawInputData, DeltaTime, false, *PVehicleDriveNW);
	});
}

void UWheeledVehicleMovementComponentNW::UpdateEngineSetup(const FVehicleNWEngineData& NewEngineSetup)
{
	if (PVehicleDrive)
	{
		PxVehicleEngineData EngineData;
		GetVehicleEngineSetup(NewEngineSetup, EngineData);

		PxVehicleDriveNW* PVehicleDriveNW = (PxVehicleDriveNW*)PVehicleDrive;
		PVehicleDriveNW->mDriveSimData.setEngineData(EngineData);
	}
}

void UWheeledVehicleMovementComponentNW::UpdateDifferentialSetup(const TArray<FVehicleNWWheelDifferentialData>& NewDifferentialSetup)
{
	if (PVehicleDrive)
	{
		PxVehicleDifferentialNWData DifferentialData;
		GetVehicleDifferentialNWSetup(NewDifferentialSetup, DifferentialData);

		PxVehicleDriveNW* PVehicleDriveNW = (PxVehicleDriveNW*)PVehicleDrive;
		PVehicleDriveNW->mDriveSimData.setDiffData(DifferentialData);
	}
}

void UWheeledVehicleMovementComponentNW::UpdateTransmissionSetup(const FVehicleNWTransmissionData& NewTransmissionSetup)
{
	if (PVehicleDrive)
	{
		PxVehicleGearsData GearData;
		GetVehicleGearSetup(NewTransmissionSetup, GearData);

		PxVehicleAutoBoxData AutoBoxData;
		GetVehicleAutoBoxSetup(NewTransmissionSetup, AutoBoxData);

		PxVehicleDriveNW* PVehicleDriveNW = (PxVehicleDriveNW*)PVehicleDrive;
		PVehicleDriveNW->mDriveSimData.setGearsData(GearData);
		PVehicleDriveNW->mDriveSimData.setAutoBoxData(AutoBoxData);
	}
}

void BackwardsConvertCm2ToM2NW(float& val, float defaultValue)
{
	if (val != defaultValue)
	{
		val = Cm2ToM2(val);
	}
}

void UWheeledVehicleMovementComponentNW::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_VEHICLES_UNIT_CHANGE)
	{
		PxVehicleEngineData DefEngineData;
		const float DefaultRPM = OmegaToRPM(DefEngineData.mMaxOmega);

		// We need to convert from old units to new. This backwards compatible code fails in the rare case that they were using very strange values that are the new defaults in the correct units.
		EngineSetup.MaxRPM = EngineSetup.MaxRPM != DefaultRPM ? OmegaToRPM(EngineSetup.MaxRPM) : DefaultRPM;	//need to convert from rad/s to RPM
	}

	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_VEHICLES_UNIT_CHANGE2)
	{
		PxVehicleEngineData DefEngineData;
		PxVehicleClutchData DefClutchData;

		// We need to convert from old units to new. This backwards compatable code fails in the rare case that they were using very strange values that are the new defaults in the correct units.
		BackwardsConvertCm2ToM2NW(EngineSetup.DampingRateFullThrottle, DefEngineData.mDampingRateFullThrottle);
		BackwardsConvertCm2ToM2NW(EngineSetup.DampingRateZeroThrottleClutchDisengaged, DefEngineData.mDampingRateZeroThrottleClutchDisengaged);
		BackwardsConvertCm2ToM2NW(EngineSetup.DampingRateZeroThrottleClutchEngaged, DefEngineData.mDampingRateZeroThrottleClutchEngaged);
		BackwardsConvertCm2ToM2NW(EngineSetup.MOI, DefEngineData.mMOI);
		BackwardsConvertCm2ToM2NW(TransmissionSetup.ClutchStrength, DefClutchData.mStrength);
	}
}

void UWheeledVehicleMovementComponentNW::ComputeConstants()
{
	Super::ComputeConstants();
	MaxEngineRPM = EngineSetup.MaxRPM;
}

const void* UWheeledVehicleMovementComponentNW::GetTireData(physx::PxVehicleWheels* InWheels, UVehicleWheel* InWheel)
{
	const void* realShaderData = &InWheels->mWheelsSimData.getTireData((PxU32)InWheel->WheelIndex);
	return realShaderData;
}

const int32 UWheeledVehicleMovementComponentNW::GetWheelShapeMapping(physx::PxVehicleWheels* InWheels, uint32 InWheel)
{
	const physx::PxI32 ShapeIndex = InWheels->mWheelsSimData.getWheelShapeMapping((PxU32)InWheel);
	return ShapeIndex;
}

const physx::PxVehicleWheelData UWheeledVehicleMovementComponentNW::GetWheelData(physx::PxVehicleWheels* InWheels, uint32 InWheel)
{
	const physx::PxVehicleWheelData WheelData = InWheels->mWheelsSimData.getWheelData((physx::PxU32)InWheel);
	return WheelData;
}
