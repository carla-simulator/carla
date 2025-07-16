// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "SimReadySettings.generated.h"

UENUM()
enum class ERenderContext : uint8
{
    ERC_DEFAULT UMETA(DisplayName="Default"),
    ERC_MDL		UMETA(DisplayName="MDL"),
};

UCLASS(Config=Engine, DefaultConfig)
class SIMREADYRUNTIME_API USimReadySettings:public UObject
{
    GENERATED_BODY()

public:
    // Paths should start with "/" and be found in /Content/Omniverse at the root of the Nucleus server mount
    UPROPERTY(Config, EditAnywhere, Category=Settings, meta = (DisplayName="MDL Template Paths"))
    TArray<FString> MDLTemplatePaths;

    // Material Import/Render Setting: Default - USD Preview surface, MDL - MDL materials
    UPROPERTY(Config, EditAnywhere, Category=Settings)
    ERenderContext RenderContext = ERenderContext::ERC_MDL;

    UPROPERTY(Config, EditAnywhere, Category = Settings)
    bool bDisableMaterialLoading = false;

    // If enabled, meshes will have query collision enabled on import
    UPROPERTY(Config, EditAnywhere, Category = Settings)
    bool bEnableQueryCollision = false;

    /** When enabled, if both old and new UsdLuxLight schema attributes are authored on a light prim, read the new schema attributes */
    UPROPERTY(Config, EditAnywhere, Category = Settings, meta = (DisplayName = "Prefer New USD LuxLight Schema on Import"))
    bool bPreferNewUsdLuxLightSchemaOnImport = true;

    USimReadySettings();
};
