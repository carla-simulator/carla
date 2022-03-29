// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/rpc/ObjectLabel.h"
#include "carla/rpc/LabelledPoint.h"
#include <compiler/enable-ue4-macros.h>

#include <vector>
#include <utility>

#include "RayTracer.generated.h"

class AActor;

UCLASS()
class CARLA_API URayTracer : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  static std::vector<carla::rpc::LabelledPoint> CastRay(
      FVector StartLocation, FVector EndLocation, UWorld * World);

  static std::pair<bool, carla::rpc::LabelledPoint> ProjectPoint(
      FVector StartLocation, FVector Direction, float MaxDistance, UWorld * World);

};
