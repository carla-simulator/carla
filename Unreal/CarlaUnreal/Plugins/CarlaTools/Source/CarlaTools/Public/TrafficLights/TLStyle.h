// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "HAL/Platform.h"
#include "UObject/ObjectMacros.h"

#include "TLStyle.generated.h"

UENUM(BlueprintType)
enum class ETLStyle : uint8
{
	NorthAmerican UMETA(DisplayName = "North American Standard"),
	European UMETA(DisplayName = "European Standard"),
	Asian UMETA(DisplayName = "Asian Standard"),
	Custom UMETA(DisplayName = "Custom"),
};
