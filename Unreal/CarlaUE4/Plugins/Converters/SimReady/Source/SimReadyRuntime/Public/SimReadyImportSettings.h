// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SimReadyImportSettings.generated.h"

USTRUCT(BlueprintType)
struct SIMREADYRUNTIME_API FSimReadyImportSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    bool bImportUnusedReferences = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    bool bImportAsBlueprint = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    bool bImportLights = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    bool bSimReadyVehicle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    TArray<FString> PathSubstringsToIgnore;
};