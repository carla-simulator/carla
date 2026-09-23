// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "HAL/Platform.h"
#include "UObject/ObjectMacros.h"

#include "TLHeadAttachment.generated.h"

UENUM(BlueprintType)
enum class ETLHeadAttachment : uint8
{
	Ring UMETA(DisplayName = "Ring"),
	SideArm UMETA(DisplayName = "Side Arm"),
	StandardBracket UMETA(DisplayName = "Standard Bracket"),
	StandardClamp UMETA(DisplayName = "Standard Clamp"),
};
