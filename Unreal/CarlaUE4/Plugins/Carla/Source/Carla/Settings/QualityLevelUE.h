// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/QualityLevel.h>
#include <compiler/enable-ue4-macros.h>

#include "QualityLevelUE.generated.h"

#define CARLA_ENUM_FROM_RPC(e) static_cast<uint8>(carla::rpc::QualityLevel:: e)

UENUM(BlueprintType)
enum class EQualityLevel : uint8
{
  Null = 0, // Workarround for UE4.24 issue with enums
  Low    = CARLA_ENUM_FROM_RPC(Low)    UMETA(DisplayName = "Low"),
  // Medium = CARLA_ENUM_FROM_RPC(Medium) UMETA(DisplayName = "Medium"),
  // High   = CARLA_ENUM_FROM_RPC(High)   UMETA(DisplayName = "High"),
  Epic   = CARLA_ENUM_FROM_RPC(Epic)   UMETA(DisplayName = "Epic"),

  SIZE      UMETA(Hidden),
  INVALID   UMETA(Hidden)
};

static_assert(
    static_cast<uint8>(EQualityLevel::SIZE) == static_cast<uint8>(carla::rpc::QualityLevel::SIZE),
    "Please keep these two enums in sync.");

#undef CARLA_ENUM_FROM_RPC
