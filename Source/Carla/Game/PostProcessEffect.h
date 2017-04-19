// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "PostProcessEffect.generated.h"

UENUM(BlueprintType)
enum class EPostProcessEffect : uint8
{
  None                  UMETA(DisplayName = "RGB without any post-processing"),
  SceneFinal            UMETA(DisplayName = "RGB with post-processing present at the scene"),
  Depth                 UMETA(DisplayName = "Depth Map"),
  SemanticSegmentation  UMETA(DisplayName = "Semantic Segmentation"),

  SIZE                  UMETA(Hidden),
  INVALID               UMETA(Hidden),
};

/// Helper class for working with EPostProcessEffect.
class CARLA_API PostProcessEffect {
public:

  static FString ToString(EPostProcessEffect PostProcessEffect);
};
