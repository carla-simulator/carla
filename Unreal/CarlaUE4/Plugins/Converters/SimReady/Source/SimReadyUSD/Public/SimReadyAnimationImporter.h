// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "SimReadyPxr.h"
#include "SimReadySettingsCommon.h"
#include "CoreMinimal.h"
#include "Logging/LogMacros.h"


class SIMREADYUSD_API FSimReadyAnimationImporter
{
public:
    static TArray<FString> LoadUSDAnimation(const TArray<FString>& Paths, const FString& Dest, class USkeleton* Skeleton, const FName Name = NAME_None, const EAnimationSource AnimationSource = EAnimationSource::EAS_USD);
    static bool LoadUSDAnimation(const FString& Path, const FString& Dest, class USkeleton* Skeleton, const FName Name = NAME_None, const EAnimationSource AnimationSource = EAnimationSource::EAS_USD);
};
