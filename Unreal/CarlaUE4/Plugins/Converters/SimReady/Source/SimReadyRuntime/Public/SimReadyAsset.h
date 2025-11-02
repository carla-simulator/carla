// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "UObject/Object.h"
#include "AssetRegistryModule.h"
#include "UObject/UObjectHash.h"
#include "SimReadyAsset.generated.h"

UCLASS(Abstract, BlueprintType)
class SIMREADYRUNTIME_API USimReadyAsset: public UObject
{
    GENERATED_BODY()

public:
    USimReadyAsset();
    ~USimReadyAsset();

    static USimReadyAsset* GetSimReadyAsset(UObject& Object);
    template<typename T>
    static T* GetSimReadyAsset(UObject& Object) { return Cast<T>(GetSimReadyAsset(Object)); }

    static void ShowMaxPathNotification(const FString& InText);
    static void HandleMaxPathHyperlinkNavigate();

    static bool bSavingCache;	// Indicates whether it's a user saving or a Omniverse saving.
};
