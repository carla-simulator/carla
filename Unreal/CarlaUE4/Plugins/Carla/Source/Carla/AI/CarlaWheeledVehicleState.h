// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CarlaWheeledVehicleState.generated.h"

/// State of a ACarlaWheeledVehicle, to be displayed in editor for debugging
/// purposes.
UENUM(BlueprintType)
enum class ECarlaWheeledVehicleState : uint8 {
  AutopilotOff          UMETA(DisplayName = "Autopilot Off"),
  FreeDriving           UMETA(DisplayName = "Free driving"),
  FollowingFixedRoute   UMETA(DisplayName = "Following fixed route"),
  WaitingForRedLight    UMETA(DisplayName = "Waiting for red light"),
  ObstacleAhead         UMETA(DisplayName = "Obstacle ahead"),
  UNKNOWN               UMETA(DisplayName = "Unknown")
};
