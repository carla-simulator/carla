// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Vehicle/WheelPhysicsControl.h"
#include "Curves/RichCurve.h"
#include "VehiclePhysicsControl.generated.h"


USTRUCT(BlueprintType)
struct CARLA_API FVehiclePhysicsControl
{
  GENERATED_BODY()

  // MECHANICAL SETUP

  // Engine Setup
  FRichCurve TorqueCurve;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float MaxTorque = 300.0f;;
  
  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float MaxRPM = 5000.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float MOI = 1.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float RevDownRate = 600.0f;

  // Differential Setup
  // ToDo: Convert to an enum, see EVehicleDifferential.
  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  uint8 DifferentialType = 0;
  
  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float FrontRearSplit = 0.5f;

  // Transmission Setup
  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  bool bUseGearAutoBox = true;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float GearSwitchTime = 0.5f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float FinalRatio = 4.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  TArray<float> ForwardGears = {2.85, 2.02, 1.35, 1.0, 2.85, 2.02, 1.35, 1.0};

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  TArray<float> ReverseGears = {2.86, 2.86};

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float ChangeUpRPM = 4500.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float ChangeDownRPM = 2000.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float TransmissionEfficiency = 0.9f;

  // Vehicle Setup
  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float Mass = 1000.0f;

  UPROPERTY(Category = "Vehicle Engine Physics Control", EditAnywhere, BlueprintReadWrite)
  float DragCoefficient = 0.3f;

  // Steering Setup
  FRichCurve SteeringCurve;

  // Center Of Mass
  UPROPERTY(Category = "Vehicle Center Of Mass", EditAnywhere, BlueprintReadWrite)
  FVector CenterOfMass = FVector::ZeroVector;

  // Wheels Setup
  TArray<FWheelPhysicsControl> Wheels;

  UPROPERTY(Category = "Vehicle Wheels Configuration", EditAnywhere, BlueprintReadWrite)
  bool UseSweepWheelCollision = false;
};
