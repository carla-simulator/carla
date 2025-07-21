// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SimReadySettingsCommon.generated.h"


UENUM(BlueprintType)
enum class EAnimationSource : uint8
{
    EAS_USD		UMETA(DisplayName = "USD Animation"),
    ERC_A2F		UMETA(DisplayName = "Audio2Face MetaHuman Curves"),
};
