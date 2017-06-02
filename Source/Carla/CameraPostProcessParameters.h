// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CameraPostProcessParameters.generated.h"

/**
  * Some post-process parameters that can be overriden per camera depending on
  * the weather and lighting conditions.
  */
USTRUCT()
struct FCameraPostProcessParameters
{
  GENERATED_USTRUCT_BODY()

  /** Will override FTextureRenderTarget2DResource::GetDisplayGamma if > 0. */
  UPROPERTY(Category = "Camera Parameters", EditAnywhere, BlueprintReadWrite)
  float TargetGamma = 0.0f;

  /** Luminance computation method */
  UPROPERTY(Category = "Camera Parameters", EditAnywhere, BlueprintReadWrite)
  TEnumAsByte<enum EAutoExposureMethod> AutoExposureMethod = AEM_Histogram;

  /**
   * A good value should be positive near 0. This is the minimum brightness the auto exposure can adapt to.
   * It should be tweaked in a dark lighting situation (too small: image appears too bright, too large: image appears too dark).
   * Note: Tweaking emissive materials and lights or tweaking auto exposure can look the same. Tweaking auto exposure has global
   * effect and defined the HDR range - you don't want to change that late in the project development.
   * Eye Adaptation is disabled if MinBrightness = MaxBrightness
   */
  UPROPERTY(Category = "Camera Parameters", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", UIMax = "10.0"))
  float AutoExposureMinBrightness = 0.03f;

  /**
   * A good value should be positive (2 is a good value). This is the maximum brightness the auto exposure can adapt to.
   * It should be tweaked in a bright lighting situation (too small: image appears too bright, too large: image appears too dark).
   * Note: Tweaking emissive materials and lights or tweaking auto exposure can look the same. Tweaking auto exposure has global
   * effect and defined the HDR range - you don't want to change that late in the project development.
   * Eye Adaptation is disabled if MinBrightness = MaxBrightness
   */
  UPROPERTY(Category = "Camera Parameters", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", UIMax = "10.0"))
  float AutoExposureMaxBrightness = 2.0f;

  /**
   * Logarithmic adjustment for the exposure. Only used if a tonemapper is specified.
   * 0: no adjustment, -1:2x darker, -2:4x darker, 1:2x brighter, 2:4x brighter, ...
   */
  UPROPERTY(Category = "Camera Parameters", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "-8.0", UIMax = "8.0"))
  float AutoExposureBias = 0.0f;
};
