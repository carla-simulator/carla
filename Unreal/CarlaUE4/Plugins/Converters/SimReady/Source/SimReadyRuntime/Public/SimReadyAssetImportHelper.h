// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"

class SIMREADYRUNTIME_API FSimReadyAssetImportHelper
{
public:
    static class USkeletalMesh* CreateSkeletalMeshFromImportData(class UPackage* Package, FName Name, enum EObjectFlags Flag, class FSkeletalMeshImportData& SkelMeshImportData, const FString& SkelMeshName);
    static bool LoadSkeletalMeshFromImportData(class USkeletalMesh* SkeletalMesh, class FSkeletalMeshImportData& SkelMeshImportData, const FString& SkelMeshName);
};