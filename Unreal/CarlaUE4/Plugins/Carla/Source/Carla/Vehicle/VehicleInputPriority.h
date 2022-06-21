// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <type_traits>

#include "VehicleInputPriority.generated.h"

UENUM(BlueprintType)
enum class EVehicleInputPriority : uint8
{
  Null = 0, // Workarround for UE4.24 issue with enums
  INVALID = 0u  UMETA(Hidden),

  Lowest        UMETA(DisplayName = "Lowest Priority", ToolTip = "Use for debugging purposes only"),
  Relaxation    UMETA(DisplayName = "Control Relaxation Input", ToolTip = "Control relaxation when no other input is provided (non-sticky control)"),
  Autopilot     UMETA(DisplayName = "Autopilot Input", ToolTip = "Input provided by the built-in autopilot"),
  User          UMETA(DisplayName = "User Input", ToolTip = "Input provided by an user playing in the simulator"),
  Client        UMETA(DisplayName = "Client Input", ToolTip = "Input provided by an RPC client connected to the simulator"),
  Highest       UMETA(DisplayName = "Highest Priority", ToolTip = "Use for debugging purposes only")
};

inline static bool operator<=(EVehicleInputPriority Lhs, EVehicleInputPriority Rhs)
{
  auto Cast = [](auto e) { return static_cast<typename std::underlying_type<decltype(e)>::type>(e); };
  return Cast(Lhs) <= Cast(Rhs);
}
