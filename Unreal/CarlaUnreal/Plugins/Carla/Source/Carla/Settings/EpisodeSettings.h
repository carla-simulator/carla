// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bSubstepping = true;

  TOptional<double> FixedDeltaSeconds;

  double MaxSubstepDeltaTime = 0.01;

  int MaxSubsteps = 10;

  float MaxCullingDistance = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bDeterministicRagdolls = true;

  float TileStreamingDistance = 300000.f; // 3km

  float ActorActiveDistance = 200000.f; // 2km

  bool SpectatorAsEgo = true;

};
