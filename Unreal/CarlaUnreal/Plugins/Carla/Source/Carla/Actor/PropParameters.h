// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "Engine/StaticMesh.h"
#include <util/ue-header-guard-end.h>

#include "PropParameters.generated.h"

UENUM(BlueprintType)
enum class EPropSize : uint8
{

  Tiny        UMETA(DisplayName = "Tiny",   ToolTip = "Smaller than a mailbox"),
  Small       UMETA(DisplayName = "Small",  ToolTip = "Size of a mailbox"),
  Medium      UMETA(DisplayName = "Medium", ToolTip = "Size of a human"),
  Big         UMETA(DisplayName = "Big",    ToolTip = "Size of a bus stop"),
  Huge        UMETA(DisplayName = "Huge",   ToolTip = "Size of a house or bigger"),

  SIZE        UMETA(Hidden),
  INVALID     UMETA(DisplayName = "INVALID")
};

USTRUCT(BlueprintType)
struct CARLA_API FPropParameters
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Name;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UStaticMesh* Mesh = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  EPropSize Size = EPropSize::INVALID;
};
