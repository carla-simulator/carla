// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "SimReadySettingsCommon.h"
#include "SimReadyImportSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimReadyUSDBlueprintLibrary.generated.h"

UCLASS()
class SIMREADYUSD_API USimReadyUSDBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Import usd animation to the animation sequence. */
    UFUNCTION(BlueprintCallable, Category = "SimReady USD Import")
    static bool LoadUSDAnimation(const FString& Path, const FString& Dest, class USkeleton* Skeleton, FName Name = NAME_None, EAnimationSource AnimationSource = EAnimationSource::EAS_USD);

    UFUNCTION(BlueprintCallable, Category="SimReady USD Import")
    static bool LoadUSD(const FString& Path, const FString& Dest, const FSimReadyImportSettings& ImportSettings);
};