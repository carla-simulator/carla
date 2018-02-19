
#pragma once

#include "RouteCommand.generated.h"

UENUM(BlueprintType)
enum class ERouteCommand : uint8 {
  GoalReached  UMETA(DisplayName = "GoalReached"),
  Unused_1     UMETA(DisplayName = "Unused_1"),
  LaneFollow   UMETA(DisplayName = "LaneFollow"),
  TurnLeft     UMETA(DisplayName = "TurnLeft"),
  TurnRight    UMETA(DisplayName = "TurnRight"),
  GoStraight   UMETA(DisplayName = "GoStraight")
};
