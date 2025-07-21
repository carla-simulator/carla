// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "SimReadyMaterialReparentSettings.h"


class SIMREADYRUNTIME_API FSimReadyMaterialReparentUtility
{
public:
    static int32 FindOmniMaterial(class UMaterialInterface* MaterialInterface, FString& MaterialName, ESimReadyReparentUsage Usage);
    static int32 FindUnrealMaterial(const FString& OmniMaterial, UMaterialInterface*& UnrealMaterial, ESimReadyReparentUsage Usage);
    static class UMaterialInterface* FindUnrealMaterial(const FString& OmniMaterial, ESimReadyReparentUsage Usage);

    /* MaterialIndex can get from FindOmniMaterial() or FindUnrealMaterial() */
    static bool FindUnrealParameter(int32 MaterialIndex, const FString& OmniParameter, FString& UnrealParameter);
    static bool FindOmniParameter(int32 MaterialIndex, const FString& UnrealParameter, FString& OmniParameter);
    static bool GetAllReparentParametersAndConstants(int32 MaterialIndex, TArray<struct FSimReadyReparentParameterInfo>& Parameters, TArray<struct FSimReadyReparentConstantInfo>& Constants);
    static bool FindScalarConstant(int32 MaterialIndex, const FString& OmniParameter, float& ScalarConstant);
    static bool FindVectorConstant(int32 MaterialIndex, const FString& OmniParameter, FLinearColor& VectorConstant);
    static bool FindTextureConstant(int32 MaterialIndex, const FString& OmniParameter, class UTexture*& Texture);
};
