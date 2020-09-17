// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/TrafficLightState.h>
#include <compiler/enable-ue4-macros.h>

#include "TrafficLightState.generated.h"


UENUM(BlueprintType)
enum class ETrafficLightState : uint8 {
  Red       UMETA(DisplayName = "Red"),
  Yellow    UMETA(DisplayName = "Yellow"),
  Green     UMETA(DisplayName = "Green"),
  Off       UMETA(DisplayName = "Off")
};
