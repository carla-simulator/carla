// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "TrafficLightState.generated.h"

UENUM(BlueprintType)
enum class ETrafficLightState : uint8 {
  Red       UMETA(DisplayName = "Red"),
  Yellow    UMETA(DisplayName = "Yellow"),
  Green     UMETA(DisplayName = "Green")
};
