// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "PostProcessEffect.h"
#include "CapturedImage.generated.h"

/// Bitmap and meta info of a scene capture.
///
/// The bitmap may be empty if the capture failed.
USTRUCT()
struct FCapturedImage
{
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere)
  uint32 SizeX = 0u;

  UPROPERTY(VisibleAnywhere)
  uint32 SizeY = 0u;

  UPROPERTY(VisibleAnywhere)
  EPostProcessEffect PostProcessEffect = EPostProcessEffect::INVALID;

  UPROPERTY(VisibleAnywhere)
  TArray<FColor> BitMap;
};
