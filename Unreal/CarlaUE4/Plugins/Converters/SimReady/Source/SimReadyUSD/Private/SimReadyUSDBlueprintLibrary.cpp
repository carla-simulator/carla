// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyUSDBlueprintLibrary.h"
#include "SimReadyAnimationImporter.h"
#include "SimReadyUSDImporter.h"
#include "SimReadyUSDLog.h"

namespace SimReadyUSDBlueprintLibraryImpl
{
    bool ValidatePackagePath(const FString& Path)
    {
        if (Path.Len() >= 2 && Path[0] == '/')
        {
            return true;
        }

        UE_LOG(LogSimReadyUsd, Error, TEXT("Package path is invalid, start with a \" / \" "));
        return false;
    }

    bool ValidateUSDPath(const FString& Path)
    {
        FString Extension = FPaths::GetExtension(Path).ToLower();
        if (Extension == TEXT("usda") || Extension == TEXT("usd"))
        {
            return true;
        }

        UE_LOG(LogSimReadyUsd, Error, TEXT("Export USD path is invalid, must be usd or usda "));
        return false;
    }
}

bool USimReadyUSDBlueprintLibrary::LoadUSDAnimation(const FString& Path, const FString& Dest, class USkeleton* Skeleton, FName Name, EAnimationSource AnimationSource)
{
    if (SimReadyUSDBlueprintLibraryImpl::ValidatePackagePath(Dest))
    {
        return FSimReadyAnimationImporter::LoadUSDAnimation(Path, Dest, Skeleton, Name, AnimationSource);
    }

    return false;
}

bool USimReadyUSDBlueprintLibrary::LoadUSD(const FString& Path, const FString& Dest, const FSimReadyImportSettings& ImportSettings)
{
    if (SimReadyUSDBlueprintLibraryImpl::ValidatePackagePath(Dest))
    {
        return FSimReadyUSDImporter::LoadUSD(Path, Dest, ImportSettings);
    }

    return false;
}
