// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CameraControl.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FCameraControl
{
  GENERATED_BODY()

  UPROPERTY(Category = "Camera Control", EditAnywhere, BlueprintReadWrite)
  float Pan = 0.0f;

  UPROPERTY(Category = "Camera Control", EditAnywhere, BlueprintReadWrite)
  float Tilt = 0.0f;

  UPROPERTY(Category = "Camera Control", EditAnywhere, BlueprintReadWrite)
  float Zoom = 0.0f;
};
