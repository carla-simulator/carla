// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "SimReadyAsset.h"
#if WITH_EDITOR
#include "Engine/Texture.h"
#include "MDLParameter.h"
#include "MDLDependencies.h"
#endif
#include "SimReadyMDL.generated.h"

UCLASS(meta = (OmniListingNeedsLoading))
class SIMREADYRUNTIME_API USimReadyMDL: public USimReadyAsset
{
    GENERATED_BODY()

public:
    USimReadyMDL();

    static bool IsLocalBaseMDL(const FString& Path);
    static bool IsLocalMaterialGraphMDL(const FString& InPath);
    static UMaterialInterface* GetLocalBaseMaterial(class UMaterialInterface* MaterialInterface);
    static UMaterialInterface* FindLocalBaseMDL(const FString& FileName, const FString& MaterialName);
    static UMaterialInterface* LoadLocalBaseMDL(const FString& FileName, const FString& MaterialName);
    static bool GetMDLModuleByMaterialName(const FString& InMaterialName, FString& ModuleName);
    static bool GetMDLModuleByMaterial(const class UMaterialInterface* MaterialInterface, FString& ModuleName);
    static bool GetDisplayNameFromLocalBaseMDL(const FString& MaterialName, const FString& ParameterName, FString& DisplayName);
    static bool GetMdlParameterTypeAndNameFromLocalBaseMDL(const FString& InMaterialName, const FString& InDisplayName, EMdlValueType& ValueType, FString& ParameterName);
    static void LoadMaterialGraphDefinitions();
    static void UnloadMaterialGraphDefinitions();
    static bool CreateMdlInstance(const FString& ModuleName, const FString& FunctionName, const FString& InstanceName);
    static bool SetCall(const FString& InstanceTarget, const FString& ParameterName, const FString& InstanceCall);
    static bool DistillMaterialInstance(class UMaterial* Material, const FString& MaterialInstanceName, bool bUseDisplayName);
    static class UMaterialInterface* ImportMDL(class UObject* InPackage, const FString& InModuleName, const FString& InMaterialName, 
        const TArray<FString>& ModulePaths, TSharedPtr<class IMDLExternalReader> ExternalReader, 
        FName Name, EObjectFlags Flags, const FString& SourceFile, TFunction<void(const uint8*, uint64, const FString&, class UTexture*&)> Callback = nullptr);
    static void ResetImportParametersSheet();
    static bool GetImportDisplayName(const FString& InMaterialName, const FString& InParameterName, FString& DisplayName);
    static void ImportMdlParameters(const FString& InModuleName, const FString& InMaterialName, const TArray<FString>& ModulePaths, 
    TSharedPtr<IMDLExternalReader> ExternalReader, const FString& InMaterialAssetName);
    static FString GetContentPath(const FString& Path);
    static void GetLocalBaseMaterialNames(TArray<FString>& Names);
    static void GetAllDisplayNamesFromLocalBaseMDL(const FString& InMaterialName, TMap<FString, EMaterialParameterType>& Parameters);
    static FString MaterialParameterTypeToString(EMaterialParameterType Type);

protected:
    static bool IsMDLLibrary(const FString& Header);

private:
    static TMap<FString, struct FMDLParametersList> ImportedParametersSheet;
};