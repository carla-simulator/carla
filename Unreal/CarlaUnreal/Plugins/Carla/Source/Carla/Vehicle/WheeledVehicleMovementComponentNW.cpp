// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WheeledVehicleMovementComponentNW.h"

#include <util/ue-header-guard-begin.h>
#include "PhysicsPublic.h"
#include "Components/PrimitiveComponent.h"
#include "Logging/MessageLog.h"
#include <util/ue-header-guard-end.h>

UWheeledVehicleMovementComponentNW::UWheeledVehicleMovementComponentNW(const FObjectInitializer& ObjectInitializer) : 
  Super(ObjectInitializer)
{
  /*
  FVehicleEngineConfig DefEngineData;
  DefEngineData.InitDefaults();
  EngineSetup.MOI = DefEngineData.EngineRevUpMOI;
  EngineSetup.MaxRPM = DefEngineData.MaxRPM;
  
  //
  -- We need to investigate about this --
  EngineSetup.DampingRateFullThrottle = DefEngineData.mDampingRateFullThrottle;
  EngineSetup.DampingRateZeroThrottleClutchEngaged = DefEngineData.mDampingRateZeroThrottleClutchEngaged;
  EngineSetup.DampingRateZeroThrottleClutchDisengaged = DefEngineData.mDampingRateZeroThrottleClutchDisengaged;

  // Convert from Chaos curve to ours
  FRichCurve* TorqueCurveData = EngineSetup.TorqueCurve.GetRichCurve();
  for (uint32 KeyIdx = 0; KeyIdx < DefEngineData.TorqueCurve.getNbDataPairs(); ++KeyIdx)
  {
    float Input = DefEngineData.TorqueCurve.getX(KeyIdx) * EngineSetup.MaxRPM;
    float Output = DefEngineData.TorqueCurve.getY(KeyIdx) * DefEngineData.MaxTorque;
    TorqueCurveData->AddKey(Input, Output);
  }

  FVehicleTransmissionConfig TransmissionData;
  TransmissionSetup.GearChangeTime = DefGearSetup.mSwitchTime;
  TransmissionSetup.ReverseGearRatio = DefGearSetup.mRatios[PxVehicleGearsData::eREVERSE];
  TransmissionSetup.FinalRatio = DefGearSetup.mFinalRatio;

  PxVehicleAutoBoxData DefAutoBoxSetup;
  TransmissionSetup.NeutralGearUpRatio = DefAutoBoxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL];
  TransmissionSetup.GearAutoBoxLatency = DefAutoBoxSetup.getLatency();
  TransmissionSetup.bUseAutomaticGears = true;

  for (uint32 i = PxVehicleGearsData::eFIRST; i < DefGearSetup.mNbRatios; ++i)
  {
    FVehicleNWGearData GearData;
    GearData.DownRatio = DefAutoBoxSetup.mDownRatios[i];
    GearData.UpRatio = DefAutoBoxSetup.mUpRatios[i];
    GearData.Ratio = DefGearSetup.mRatios[i];
    TransmissionSetup.ForwardGearRatios.Add(GearData);
  }

  */

  // Init steering speed curve
  //FRichCurve* SteeringCurveData = SteeringCurve.GetRichCurve();
  //SteeringCurveData->AddKey(0.0f, 1.0f);
  //SteeringCurveData->AddKey(20.0f, 0.9f);
  //SteeringCurveData->AddKey(60.0f, 0.8f);
  //SteeringCurveData->AddKey(120.0f, 0.7f);

  // Initialize WheelSetups array with 4 wheels, this can be modified via editor later
  const int32 NbrWheels = 4;
  WheelSetups.SetNum(NbrWheels);
  //DifferentialSetup.SetNum(NbrWheels);

  IdleBrakeInput = 10;
}

#if WITH_EDITOR
void UWheeledVehicleMovementComponentNW::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  /*
  const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

  if (PropertyName == TEXT("DownRatio"))
  {
    for (int32 GearIdx = 0; GearIdx < TransmissionSetup.ForwardGearRatios.Num(); ++GearIdx)
    {
      FVehicleNWGearData& GearData = TransmissionSetup.ForwardGearRatios[GearIdx];
      GearData.DownRatio = FMath::Min(GearData.DownRatio, GearData.UpRatio);
    }
  }
  else if (PropertyName == TEXT("UpRatio"))
  {
    for (int32 GearIdx = 0; GearIdx < TransmissionSetup.ForwardGearRatios.Num(); ++GearIdx)
    {
      FVehicleNWGearData& GearData = TransmissionSetup.ForwardGearRatios[GearIdx];
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
  */
}
#endif

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
