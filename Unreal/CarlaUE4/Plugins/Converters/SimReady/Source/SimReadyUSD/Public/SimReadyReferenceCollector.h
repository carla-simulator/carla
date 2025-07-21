// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "SimReadyExportSettings.h"
#include "SimReadyImportSettings.h"
#include "SimReadyPxr.h"

class SIMREADYUSD_API FSimReadyReferenceCollector
{
public:
    FSimReadyReferenceCollector();

    virtual ~FSimReadyReferenceCollector();

    int32 GetAllDependencies(const TArray<class AActor*>& Actors, const FSimReadyExportSettings& ExportSettings);

    int32 GetAllDependencies(const pxr::UsdStageRefPtr& USDStage, const FSimReadyImportSettings& ImportSettings);

    bool FindDependency(const FName& PackageName);

    void Reset();

    static FSimReadyReferenceCollector& Get();

private:

    void GetDependenciesInternal(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& Path, bool bFromSkelRoot);
    void GetDependenciesInternal(const class USceneComponent& Component);
    void GetTextureDependencies(const FName& PackageName, const FString& Root);

    // all the dependencies for current package
    TSet<FName> LoadedDependencies;
    // the dependencies which are needed by omniverse
    TSet<FName> SupportedDependencies;

    bool CurrentMaterialSkipped;
    bool CurrentTextureSkipped;
    bool CurrentDecalSkipped;
    bool CurrentUnusedSkipped;
};