// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"

// Mesh cache to boost mesh importing
class FUSDGeometryCache
{
public:
    static class UObject* Find(const FString& Hash);
    static void Add(const FString& Hash, UObject* Object);
    static void BackupCurrentCache();
    static void RestoreLastBackup();

protected:
    static TMap<FString, TStrongObjectPtr<class UObject>> CachedAssets;
    static TMap<FString, TStrongObjectPtr<class UObject>> BackupCache;
};