// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Settings/PostProcessEffect.h"
#include "CapturedImage.generated.h"

/// Bitmap and meta info of a scene capture.
///
/// The bitmap may be empty if the capture failed.
USTRUCT()
struct FCapturedImage
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY(VisibleAnywhere)
  uint32 SizeX = 0u;

  UPROPERTY(VisibleAnywhere)
  uint32 SizeY = 0u;

  UPROPERTY(VisibleAnywhere)
  EPostProcessEffect PostProcessEffect = EPostProcessEffect::INVALID;

  UPROPERTY(VisibleAnywhere)
  float FOVAngle = 0.0f;

  UPROPERTY(VisibleAnywhere)
  TArray<FColor> BitMap;
};
