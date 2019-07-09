// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "EpisodeSettings.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FEpisodeSettings
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bSynchronousMode = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bNoRenderingMode = false;

  TOptional<double> FixedDeltaSeconds;
};
