// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"

class SIMREADYRUNTIME_API FSimReadyPathHelper
{
public:
    // Compute relative path of target path relative to SourcePath.
    // So ComputeRelativePath(omniverse:/a/b/c, omniverse:/a/c/d) will return ../b/c/
    // while ComputeRelativePath(omniverse:/a/b/c, omniverse:/a/c/d/) will return ../../b/c/
    // If source path and target path are not in the same domain or server, it will
    // return target path directly.
    static FString ComputeRelativePath(const FString& TargetPath, const FString& SourcePath, bool TargetIsDirectory = false, bool SourceIsDirectory = false);

    static void ManglePath(const FString& OmniPath, FString& OutUE4Path, bool bStartWithSlash = true); // only used for mangling folder path
    
    static void UnmanglePath(const FString& UE4Path, FString& OutOmniPath, bool bStartWithSlash = true);

    static FString PrimPathToKey(const FString& PrimPath);

    static FString KeyToPrimPath(const FString& Key);

    static void FixAssetName(FString& AssetName);

    // usdz package path
    static bool IsPackagePath(const FString& Path);
    static FString GetPackageRoot(const FString& Path);
    static FString GetPackagedSubPath(const FString& Path);

    // Parse string version to integer
    static bool ParseVersion(const FString& Version, int32& MajorVersion, int32& MinorVersion);

    static class UMaterialInterface* GetPreviewSurfaceMaterial(bool bNative = false);
};