// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include <util/ue-header-guard-end.h>

#include "VehicleTelemetryData.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FWheelTelemetryData
{
  GENERATED_BODY()

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LatSlip = 0.0f;

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float LongSlip = 0.0f;

  UPROPERTY(Category = "Wheel Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Omega = 0.0f;
};

USTRUCT(BlueprintType)
struct CARLA_API FVehicleTelemetryData
{
  GENERATED_BODY()

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Speed = 0.0f;

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Steer = 0.0f;

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Throttle = 0.0f;

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float Brake = 0.0f;

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  float EngineRPM = 0.0f;

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  int32 Gear = 0;

  UPROPERTY(Category = "Vehicle Telemetry Data", EditAnywhere, BlueprintReadWrite)
  TArray<FWheelTelemetryData> Wheels;
};