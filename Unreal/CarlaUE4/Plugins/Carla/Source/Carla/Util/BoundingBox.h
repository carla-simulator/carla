// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "BoundingBox.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FBoundingBox
{
  GENERATED_BODY()

  /// Origin of the bounding box relative to its owner.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FVector Origin = {0.0f, 0.0f, 0.0f};

  /// Radii extent of the bounding box.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FVector Extent = {0.0f, 0.0f, 0.0f};

  /// Rotation of the bounding box.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FRotator Rotation = {0.0f, 0.0f, 0.0f};
};
