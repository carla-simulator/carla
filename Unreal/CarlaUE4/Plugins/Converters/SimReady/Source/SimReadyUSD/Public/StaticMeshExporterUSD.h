// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//


#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

#include "StaticMeshExporterUSD.generated.h"

UCLASS()
class UStaticMeshExporterUsd : public UObject
{
    GENERATED_BODY()

public:
    // Declare this so that Engine/Plugins/Importers/USDImporter/Content/Python/init_unreal.py doesn't throw errors
    UFUNCTION(BlueprintCallable, Category = "USD")
    static bool IsUsdAvailable();

public:
    UStaticMeshExporterUsd();
};
