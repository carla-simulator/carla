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

  UPROPERTY(Category = "Wheel Steer Angle", EditAnywhere, BlueprintReadWrite)
  float SteerAngle = 0.0f;

  UPROPERTY(Category = "Wheel Disable Steering", EditAnywhere, BlueprintReadWrite)
  bool bDisableSteering = 0.0f;
};
