// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "WalkerBoneControl.generated.h"

USTRUCT(BlueprintType)
struct CARLA_API FWalkerBoneControl
{
  GENERATED_BODY()

  UPROPERTY(Category = "Walker Bone Control", EditAnywhere, BlueprintReadWrite)
  bool bModifyBones = false;

  UPROPERTY(Category = "Walker Bone Control", EditAnywhere, BlueprintReadWrite)
  FVector WorldPosition = {0.0f, 0.0f, 0.0f};

  UPROPERTY(Category = "Walker Bone Control", EditAnywhere, BlueprintReadWrite)
  FVector WorldRotation = {0.0f, 0.0f, 0.0f};

  UPROPERTY(Category = "Walker Bone Control", EditAnywhere, BlueprintReadWrite)
  TMap<FString, FTransform> BoneTransforms;

};
