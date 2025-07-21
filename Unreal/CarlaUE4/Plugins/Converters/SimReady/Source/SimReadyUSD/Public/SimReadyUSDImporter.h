// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "SimReadyPxr.h"
#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "SimReadyImportSettings.h"

class SIMREADYUSD_API FSimReadyUSDImporter
{
public:
    static bool LoadUSD(const FString& Path, const FString& Dest, const FSimReadyImportSettings& ImportSettings = FSimReadyImportSettings());
};