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

  UPROPERTY(Category = "Wheel Torque", EditAnywhere, BlueprintReadWrite)
  float Torque = 0.0f;

  UPROPERTY(Category = "Wheel Mass", EditAnywhere, BlueprintReadWrite)
  float Mass = 0.0f;

  UPROPERTY(Category = "Wheel Disable Steering", EditAnywhere, BlueprintReadWrite)
  bool bDisableSteering = 0.0f;

  UPROPERTY(Category = "Wheel Contact Surface Friction", EditAnywhere, BlueprintReadWrite)
  float ContactSurfaceFriction = 0.0f;

};