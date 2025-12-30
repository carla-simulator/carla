// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyMaterialReparentUtility.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstance.h"

int32 FSimReadyMaterialReparentUtility::FindOmniMaterial(UMaterialInterface* MaterialInterface, FString& MaterialName, ESimReadyReparentUsage Usage)
{
    if (MaterialInterface == nullptr)
    {
        return INDEX_NONE;
    }

    // check parent from mapping
    auto DefaultReparentSettings = GetDefault<USimReadyMaterialReparentSettings>();

    for (int32 Index = 0; Index < DefaultReparentSettings->MaterialInfoList.Num(); ++Index)
    {
        auto Info = DefaultReparentSettings->MaterialInfoList[Index];
        if (Info.bEnabled && (Info.Usage == Usage || Info.Usage == ESimReadyReparentUsage::Both) && !Info.UnrealMaterial.IsNull() && Info.UnrealMaterial == TSoftObjectPtr<UMaterialInterface>(MaterialInterface))
        {
            MaterialName = Info.OmniMaterial;
            return Index;
        }
    }

    if (MaterialInterface->IsA<UMaterialInstance>())
    {
        auto Instance = Cast<UMaterialInstance>(MaterialInterface);
        if (Instance->Parent)
        {
            return FindOmniMaterial(Instance->Parent, MaterialName, Usage);
        }
    }

    return INDEX_NONE;
}

int32 FSimReadyMaterialReparentUtility::FindUnrealMaterial(const FString& OmniMaterial, UMaterialInterface*& UnrealMaterial, ESimReadyReparentUsage Usage)
{
    // we just need constant settings, using GetDefault()
    auto DefaultReparentSettings = GetDefault<USimReadyMaterialReparentSettings>();

    for (int32 Index = 0; Index < DefaultReparentSettings->MaterialInfoList.Num(); ++Index)
    {
        auto Info = DefaultReparentSettings->MaterialInfoList[Index];
        if (Info.bEnabled && (Info.Usage == Usage || Info.Usage == ESimReadyReparentUsage::Both) && Info.OmniMaterial.Equals(OmniMaterial) && !Info.UnrealMaterial.IsNull())
        {
            // Try get material from soft object ptr
            auto Material = Info.UnrealMaterial.Get();

            // failed to get material, try load
            if (Material == nullptr)
            {
                Material = Info.UnrealMaterial.LoadSynchronous();
            }

            UnrealMaterial = Material;
            return Index;
        }
    }

    return INDEX_NONE;
}

UMaterialInterface* FSimReadyMaterialReparentUtility::FindUnrealMaterial(const FString& OmniMaterial, ESimReadyReparentUsage Usage)
{
    UMaterialInterface* Material = nullptr;
    FindUnrealMaterial(OmniMaterial, Material, Usage);
    return Material;
}

bool FSimReadyMaterialReparentUtility::FindUnrealParameter(int32 MaterialIndex, const FString& OmniParameter, FString& UnrealParameter)
{
    auto DefaultReparentSettings = GetDefault<USimReadyMaterialReparentSettings>();

    if (MaterialIndex != INDEX_NONE && MaterialIndex < DefaultReparentSettings->MaterialInfoList.Num())
    {
        auto ParameterList = DefaultReparentSettings->MaterialInfoList[MaterialIndex].ParameterInfoList;
        for (auto Param : ParameterList)
        {
            if (Param.OmniParameter.Equals(OmniParameter))
            {
                UnrealParameter = Param.UnrealParameter;
                return true;
            }
        }
    }

    return false;
}

bool FSimReadyMaterialReparentUtility::FindOmniParameter(int32 MaterialIndex, const FString& UnrealParameter, FString& OmniParameter)
{
    auto DefaultReparentSettings = GetDefault<USimReadyMaterialReparentSettings>();

    if (MaterialIndex != INDEX_NONE && MaterialIndex < DefaultReparentSettings->MaterialInfoList.Num())
    {
        auto ParameterList = DefaultReparentSettings->MaterialInfoList[MaterialIndex].ParameterInfoList;
        for (auto Param : ParameterList)
        {
            if (Param.UnrealParameter.Equals(UnrealParameter))
            {
                OmniParameter = Param.OmniParameter;
                return true;
            }
        }
    }

    return false;
}

bool FSimReadyMaterialReparentUtility::GetAllReparentParametersAndConstants(int32 MaterialIndex, TArray<FSimReadyReparentParameterInfo>& Parameters, TArray<FSimReadyReparentConstantInfo>& Constants)
{
    auto DefaultReparentSettings = GetDefault<USimReadyMaterialReparentSettings>();

    if (MaterialIndex != INDEX_NONE && MaterialIndex < DefaultReparentSettings->MaterialInfoList.Num())
    {
        Parameters = DefaultReparentSettings->MaterialInfoList[MaterialIndex].ParameterInfoList;
        Constants = DefaultReparentSettings->MaterialInfoList[MaterialIndex].ConstantInfoList;
        return true;
    }

    return false;
}

bool FSimReadyMaterialReparentUtility::FindScalarConstant(int32 MaterialIndex, const FString& OmniParameter, float& ScalarConstant)
{
    auto DefaultReparentSettings = GetDefault<USimReadyMaterialReparentSettings>();

    if (MaterialIndex != INDEX_NONE && MaterialIndex < DefaultReparentSettings->MaterialInfoList.Num())
    {
        auto ConstantList = DefaultReparentSettings->MaterialInfoList[MaterialIndex].ConstantInfoList;
        for (auto Constant : ConstantList)
        {
            if (Constant.OmniParameter.Equals(OmniParameter))
            {
                ScalarConstant = Constant.ConstantValue.X;
                return true;
            }
        }
    }

    return false;
}

bool FSimReadyMaterialReparentUtility::FindVectorConstant(int32 MaterialIndex, const FString& OmniParameter, FLinearColor& VectorConstant)
{
    auto DefaultReparentSettings = GetDefault<USimReadyMaterialReparentSettings>();

    if (MaterialIndex != INDEX_NONE && MaterialIndex < DefaultReparentSettings->MaterialInfoList.Num())
    {
        auto ConstantList = DefaultReparentSettings->MaterialInfoList[MaterialIndex].ConstantInfoList;
        for (auto Constant : ConstantList)
        {
            if (Constant.OmniParameter.Equals(OmniParameter))
            {
                VectorConstant = FLinearColor(Constant.ConstantValue);
                return true;
            }
        }
    }

    return false;
}

bool FSimReadyMaterialReparentUtility::FindTextureConstant(int32 MaterialIndex, const FString& OmniParameter, UTexture*& Texture)
{
    auto DefaultReparentSettings = GetDefault<USimReadyMaterialReparentSettings>();

    if (MaterialIndex != INDEX_NONE && MaterialIndex < DefaultReparentSettings->MaterialInfoList.Num())
    {
        auto ConstantList = DefaultReparentSettings->MaterialInfoList[MaterialIndex].ConstantInfoList;
        for (auto Constant : ConstantList)
        {
            if (Constant.OmniParameter.Equals(OmniParameter) && !Constant.TextureValue.IsNull())
            {
                Texture = Constant.TextureValue.Get();
                return true;
            }
        }
    }

    return false;
}