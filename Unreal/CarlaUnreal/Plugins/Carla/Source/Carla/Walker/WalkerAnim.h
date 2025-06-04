// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "Animation/AnimInstance.h"
#include <util/ue-header-guard-end.h>

#include "WalkerAnim.generated.h"

UCLASS()
class CARLA_API UWalkerAnim : public UAnimInstance
{

  GENERATED_BODY()

public:

  UPROPERTY(Category="Walker Anim", BlueprintReadWrite, EditAnywhere)
  float Blend = 0.0;

  UPROPERTY(Category="Walker Anim", BlueprintReadWrite, EditAnywhere)
  FPoseSnapshot Snap;
};
