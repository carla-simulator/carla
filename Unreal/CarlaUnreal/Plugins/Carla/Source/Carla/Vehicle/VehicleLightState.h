// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "VehicleLightState.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FVehicleLightState
{
  GENERATED_BODY()

  UPROPERTY(Category = "Vehicle Lights", EditAnywhere, BlueprintReadWrite)
  bool Position = false;

  UPROPERTY(Category = "Vehicle Lights", EditAnywhere, BlueprintReadWrite)
  bool LowBeam = false;

  UPROPERTY(Category = "Vehicle Lights", EditAnywhere, BlueprintReadWrite)
  bool HighBeam = false;

  UPROPERTY(Category = "Vehicle Lights", EditAnywhere, BlueprintReadWrite)
  bool Brake = false;

  UPROPERTY(Category = "Vehicle Lights", EditAnywhere, BlueprintReadWrite)
  bool RightBlinker = false;

  UPROPERTY(Category = "Vehicle Lights", EditAnywhere, BlueprintReadWrite)
  bool LeftBlinker = false;

  UPROPERTY(Category = "Vehicle Lights", EditAnywhere, BlueprintReadWrite)
  bool Reverse = false;

  UPROPERTY(Category = "Vehicle Lights", EditAnywhere, BlueprintReadWrite)
  bool Fog = false;

  UPROPERTY(Category = "Vehicle Lights", EditAnywhere, BlueprintReadWrite)
  bool Interior = false;

  UPROPERTY(Category = "Vehicle Lights", EditAnywhere, BlueprintReadWrite)
  bool Special1 = false;

  UPROPERTY(Category = "Vehicle Lights", EditAnywhere, BlueprintReadWrite)
  bool Special2 = false;

};
