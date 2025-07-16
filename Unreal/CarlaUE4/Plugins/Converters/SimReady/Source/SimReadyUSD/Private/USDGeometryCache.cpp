// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "USDGeometryCache.h"
#include "USDConverter.h"
#include "USDHashGenerator.h"

TMap<FString, TStrongObjectPtr<UObject>> FUSDGeometryCache::CachedAssets;
TMap<FString, TStrongObjectPtr<UObject>> FUSDGeometryCache::BackupCache;

UObject* FUSDGeometryCache::Find(const FString& Hash)
{
    auto FoundObject = CachedAssets.Find(Hash);
    if (FoundObject)
    {
        return FoundObject->Get();
    }
    return nullptr;
}

void FUSDGeometryCache::Add(const FString& Hash, UObject* Object)
{
    CachedAssets.Add(Hash, TStrongObjectPtr<UObject>(Object));
}

void FUSDGeometryCache::BackupCurrentCache()
{
    BackupCache = CachedAssets;
    CachedAssets.Empty();
}
void FUSDGeometryCache::RestoreLastBackup()
{
    CachedAssets = BackupCache;
    BackupCache.Empty();
}