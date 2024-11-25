// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Util/BoundingBox.h"

#include <util/disable-ue4-macros.h>
#include <carla/rpc/ObjectLabel.h>
#include <util/enable-ue4-macros.h>

#include "EnvironmentObject.generated.h"

namespace crp = carla::rpc;

enum EnvironmentObjectType
{
  Invalid,
  Vehicle,
  Character,
  TrafficLight,
  ISMComp,
  SMComp,
  SKMComp
};

// Name is under discussion
USTRUCT(BlueprintType)
struct CARLA_API FEnvironmentObject
{
  GENERATED_BODY()

  AActor* Actor = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Name;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString IdStr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FTransform Transform;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FBoundingBox BoundingBox;

  uint64 Id = 0;

  EnvironmentObjectType Type = EnvironmentObjectType::Invalid;

  crp::CityObjectLabel ObjectLabel;

  bool CanTick = false;
};
