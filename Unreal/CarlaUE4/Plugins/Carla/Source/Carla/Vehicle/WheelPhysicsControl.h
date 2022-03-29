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
  float TireFriction = 3.5f;

  UPROPERTY(Category = "Wheel Damping Rate", EditAnywhere, BlueprintReadWrite)
  float DampingRate = 1.0f;

  UPROPERTY(Category = "Wheel Max Steer Angle", EditAnywhere, BlueprintReadWrite)
  float MaxSteerAngle = 70.0f;

  UPROPERTY(Category = "Wheel Shape Radius", EditAnywhere, BlueprintReadWrite)
  float Radius = 30.0f;

  UPROPERTY(Category = "Wheel Max Brake Torque (Nm)", EditAnywhere, BlueprintReadWrite)
  float MaxBrakeTorque = 1500.0f;

  UPROPERTY(Category = "Wheel Max Handbrake Torque (Nm)", EditAnywhere, BlueprintReadWrite)
  float MaxHandBrakeTorque = 3000.0f;

  UPROPERTY(Category = "Max normalized tire load at which the tire can deliver no more lateral stiffness no matter how much extra load is applied to the tire", EditAnywhere, BlueprintReadWrite)
  float LatStiffMaxLoad = 2.0f;

  UPROPERTY(Category = "Lateral Stiffness Value", EditAnywhere, BlueprintReadWrite)
  float LatStiffValue = 17.0f;

  UPROPERTY(Category = "Longitudinal Stiffness Value", EditAnywhere, BlueprintReadWrite)
  float LongStiffValue = 1000.0f;

  UPROPERTY(Category = "Wheel Position", EditAnywhere, BlueprintReadWrite)
  FVector Position = FVector::ZeroVector;
};
