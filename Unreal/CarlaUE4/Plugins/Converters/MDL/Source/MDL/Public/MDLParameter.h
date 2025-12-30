// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "MDLParameter.generated.h"

UENUM()
enum class EMdlValueType : uint8
{
    MDL_UNKNOWN,
    MDL_BOOL,
    MDL_INT,
    MDL_FLOAT,
    MDL_FLOAT2,
    MDL_FLOAT3,
    MDL_FLOAT4,
    MDL_COLOR,
    MDL_TEXTURE,
};

USTRUCT()
struct MDL_API FMDLParameterInfo
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    EMdlValueType	ParameterType;

    UPROPERTY()
    FString ParameterName;

    UPROPERTY()
    FString DisplayName;

    UPROPERTY()
    bool bUsedInUnreal = false;
};

USTRUCT()
struct MDL_API FMDLParametersList
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    TArray<FMDLParameterInfo> ParametersList;
};