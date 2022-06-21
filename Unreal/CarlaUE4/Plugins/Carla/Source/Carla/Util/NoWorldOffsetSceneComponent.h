// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/SceneComponent.h"
#include "NoWorldOffsetSceneComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API UNoWorldOffsetSceneComponent : public USceneComponent
{
  GENERATED_BODY()

public:
  virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift);

};
