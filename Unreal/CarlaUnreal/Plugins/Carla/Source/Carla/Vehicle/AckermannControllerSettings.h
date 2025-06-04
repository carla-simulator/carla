// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "AckermannControllerSettings.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FAckermannControllerSettings
{
  GENERATED_BODY()

  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float SpeedKp = 0.0f;

  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float SpeedKi = 0.0f;

  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float SpeedKd = 0.0f;

  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float AccelKp = 0.0f;

  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float AccelKi = 0.0f;

  UPROPERTY(Category = "Ackermann Controller Settings", EditAnywhere, BlueprintReadWrite)
  float AccelKd = 0.0f;
};
