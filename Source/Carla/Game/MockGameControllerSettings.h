// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "MockGameControllerSettings.generated.h"

USTRUCT(BlueprintType)
struct FMockGameControllerSettings
{
  GENERATED_USTRUCT_BODY()

  /** If true, weather will be changed every time we start the level. */
  UPROPERTY(EditAnywhere, Category = "Mock CARLA Controller")
  bool bChangeWeatherOnBeginPlay = true;

  /** If true, a random player start position will be chosen every time we start the level. */
  UPROPERTY(EditAnywhere, Category = "Mock CARLA Controller")
  bool bRandomPlayerStart = true;

  /** If true, semantic segmentation will be always enabled even if no camera needs it. */
  UPROPERTY(EditAnywhere, Category = "Mock CARLA Controller")
  bool bForceEnableSemanticSegmentation = false;
};
