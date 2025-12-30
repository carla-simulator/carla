// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once 
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "SimReadySettingsCommon.h"
#include "SimReadyImporterUI.generated.h"

UENUM()
enum ESimReadyImporterType
{
    OIT_Animation,
    OIT_USD,
    OIT_Max,
};

UCLASS(HideCategories=Object, MinimalAPI)
class USimReadyImporterUI : public UObject
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY()
    TEnumAsByte<enum ESimReadyImporterType> ImporterType;

    UPROPERTY(BlueprintReadOnly, Category = Path)
    FString USDPath;

    UPROPERTY(BlueprintReadOnly, Category = Path)
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = USD, meta = (
        EditCondition = "ImporterType == ESimReadyImporterType::OIT_USD",
        EditConditionHides))
    bool bImportUnusedReferences = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = USD, meta = (
        EditCondition = "ImporterType == ESimReadyImporterType::OIT_USD",
        EditConditionHides))
    bool bImportAsBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = USD, meta = (
        EditCondition = "ImporterType == ESimReadyImporterType::OIT_USD",
        EditConditionHides))
    bool bImportLights = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = USD, meta = (
        EditCondition = "ImporterType == ESimReadyImporterType::OIT_USD",
        EditConditionHides))
    TArray<FString> PathSubstringsToIgnore = {TEXT("rail_bracket"), TEXT("roadmarks")};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, meta = (
        EditCondition = "ImporterType == ESimReadyImporterType::OIT_Animation", 
        EditConditionHides))
    class USkeleton* Skeleton;

    /** Choose animation source type to be imported.
    * Default is 'USD animation' which contains skeleton and blendshape animation.
    * NOTE: If there's no curves data, Importer will try importing 'USD animation' even if 'Audio2Face MetaHuman Curves' is selected.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, meta = (
        EditCondition = "ImporterType == ESimReadyImporterType::OIT_Animation",
        EditConditionHides))
    EAnimationSource AnimationSource = EAnimationSource::EAS_USD;

    /** Use the usd file as the asset name. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Asset, meta = (
        EditCondition = "ImporterType == ESimReadyImporterType::OIT_Animation",
        EditConditionHides))
    bool bUseFileName = true;

    /** Optional name for the animation. USD Prim name will be used if NAME_None. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Asset, meta = (
        EditCondition = "ImporterType == ESimReadyImporterType::OIT_Animation",
        EditConditionHides))
    FName Name = NAME_None;

    FString PreUSDPath[OIT_Max];
    FString PreAssetPath[OIT_Max];
};