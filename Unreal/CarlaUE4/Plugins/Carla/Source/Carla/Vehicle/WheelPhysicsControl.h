// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "WheelPhysicsControl.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FWheelPhysicsControl
{
  GENERATED_BODY()

  UPROPERTY(Category = "Wheel Tire Friction", EditAnywhere, BlueprintReadWrite)
  float TireFriction = 0.0f;

  UPROPERTY(Category = "Wheel Damping Rate", EditAnywhere, BlueprintReadWrite)
  float DampingRate = 0.0f;

  UPROPERTY(Category = "Wheel Max Steer Angle", EditAnywhere, BlueprintReadWrite)
  float MaxSteerAngle = 0.0f;

  UPROPERTY(Category = "Wheel Is Steerable", EditAnywhere, BlueprintReadWrite)
  bool IsSteerable = true;
};
