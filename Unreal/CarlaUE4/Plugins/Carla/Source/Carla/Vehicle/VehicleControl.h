// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "VehicleControl.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FVehicleControl
{
  GENERATED_BODY()

  UPROPERTY(Category = "Vehicle Control", EditAnywhere)
  float Throttle = 0.0f;

  UPROPERTY(Category = "Vehicle Control", EditAnywhere)
  float Steer = 0.0f;

  UPROPERTY(Category = "Vehicle Control", EditAnywhere)
  float Brake = 0.0f;

  UPROPERTY(Category = "Vehicle Control", EditAnywhere)
  bool bHandBrake = false;

  UPROPERTY(Category = "Vehicle Control", EditAnywhere)
  bool bReverse = false;
};
