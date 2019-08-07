// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Util/BoundingBox.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BoundingBoxCalculator.generated.h"

class AActor;

UCLASS()
class CARLA_API UBoundingBoxCalculator : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// Compute the bounding box of the given Carla actor.
  ///
  /// @warning If the actor type is not supported a default initialized bounding
  /// box is returned.
  ///
  /// @warning Traffic signs return its trigger box instead.
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FBoundingBox GetActorBoundingBox(const AActor *Actor);
};
