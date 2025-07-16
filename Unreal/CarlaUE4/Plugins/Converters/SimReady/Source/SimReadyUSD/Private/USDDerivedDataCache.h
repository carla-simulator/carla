// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"

class FUSDDerivedDataCache
{
public:

    static void Save(const FString& HashKey, class FStaticMeshRenderData& StaticMeshRenderData);
    static bool Load(const FString& HashKey, class FStaticMeshRenderData& StaticMeshRenderData);

    static void Save(const FString& HashKey, class FSkeletalMeshImportData& SkeletalMeshData);
    static bool Load(const FString& HashKey, class FSkeletalMeshImportData& SkeletalMeshData);
};