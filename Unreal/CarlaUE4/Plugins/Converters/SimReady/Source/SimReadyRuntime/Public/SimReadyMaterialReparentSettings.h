// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once 
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "SimReadyMaterialReparentSettings.generated.h"

#define USDPreviewSurfaceDisplayName TEXT("USD Preview Surface")

UENUM()
enum class ESimReadyReparentUsage : uint8
{
    Both = 0,
    Import,
    Export,
};

USTRUCT()
struct SIMREADYRUNTIME_API FSimReadyReparentConstantInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Config, Category = Material)
    FString OmniParameter;

    UPROPERTY(EditAnywhere, Config, Category = Material)
    FVector4 ConstantValue = FVector4(0.0f, 0.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, Config, Category = Material)
    TSoftObjectPtr<class UTexture> TextureValue;

    UPROPERTY(EditAnywhere, Config, Category = Material)
    bool ConstantBoolean = false;
};

USTRUCT()
struct SIMREADYRUNTIME_API FSimReadyReparentParameterInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Config, Category = Material)
    FString OmniParameter;

    UPROPERTY(EditAnywhere, Config, Category = Material)
    FString UnrealParameter;
};

USTRUCT()
struct SIMREADYRUNTIME_API FSimReadyReparentMaterialInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Config, Category = Material)
    FString OmniMaterial;

    UPROPERTY(EditAnywhere, Config, Category = Material)
    TSoftObjectPtr<class UMaterialInterface> UnrealMaterial;

    UPROPERTY(EditAnywhere, Config, Category = Material)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, Config, Category = Material)
    ESimReadyReparentUsage	Usage = ESimReadyReparentUsage::Both;

    UPROPERTY(EditAnywhere, Config, Category = Material)
    TArray<FSimReadyReparentParameterInfo> ParameterInfoList;

    UPROPERTY(EditAnywhere, Config, Category = Material, meta = (DisplayName = "Constant Info List (Export)"))
    TArray<FSimReadyReparentConstantInfo> ConstantInfoList;
};

UCLASS(Config = Engine, DefaultConfig)
class SIMREADYRUNTIME_API USimReadyMaterialReparentSettings : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, Category = Material)
    TArray<FSimReadyReparentMaterialInfo> MaterialInfoList;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
