// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "MDLParameter.h"
#include "MDLDependencies.h"
#include "MDLImporter.h"

typedef TFunction<void(const FString&, const FString&, float Gamma)> FLoadInstanceTextureCallback;
typedef TFunction<bool(const FString&, const FString&, FString&)> FGetDisplayNameCallback;


class MDL_API FMDLImporterUtility
{
public:
    static const FString GetProjectMdlRootPath()
    {
        return ProjectMdlRootPath;
    }

    static void SetProjectMdlRootPath(const FString& Path)
    {
        ProjectMdlRootPath = Path;
    }

    static void ResetProjectMdlRootPath()
    {
        static FString Path(TEXT("/Game/MDL"));
        ProjectMdlRootPath = Path;
    }

    static const FString GetProjectMdlBaseModulePath()
    {
        return GetProjectMdlRootPath() / TEXT("Base");
    }

    static const FString GetProjectMdlFunctionPath()
    {
        return GetProjectMdlRootPath() / TEXT("Functions");
    }

    static bool IsBaseModule(const FString& Name);
    static bool IsMaterialGraphModule(const FString& InFileName);
    static class UMaterialInterface* GetBaseMaterial(class UMaterialInterface* MaterialInterface);
    static class UMaterialInterface* FindBaseModule(const FString& FileName, const FString& MaterialName);
    static class UMaterialInterface* LoadBaseModule(const FString& FileName, const FString& MaterialName = TEXT(""));
    static void UpdateBaseModuleParameters(const FString& Name);
    static bool GetDisplayNameFromBaseModule(const FString& MaterialName, const FString& ParameterName, FString& DisplayName);
    static bool GetMdlParameterTypeAndNameFromBaseModule(const FString& InMaterialName, const FString& InDisplayName, EMdlValueType& ValueType, FString& ParameterName);
    static bool GetBaseModuleByMaterialName(const FString& MaterialName, FString& ModuleName);
    static bool GetBaseModuleByMaterial(const class UMaterialInterface* MaterialInterface, FString& ModuleName);
    static void CreateInstanceFromBaseMDL(class UMaterialInstanceConstant* MaterialInstance, const FString& MaterialName, const mi::base::Handle<const mi::neuraylib::IFunction_definition>& MaterialDefinition, FGetDisplayNameCallback InGetDisplayNameCallback, FLoadInstanceTextureCallback InCallback = nullptr);
    static void GetImportModulesByModuleName(const FString& ModuleName, TSet<FString>& OutModuleNames);
    static bool IsParameterUsedFromBaseModule(const FString& MaterialName, const FString& ParameterName);
    static void GetAllBaseMaterials(TArray<FString>& BaseMaterials);
    static void GetAllDisplayNamesFromBaseModule(const FString& InMaterialName, TMap<FString, EMaterialParameterType>& Parameters);

    static bool FindDisplayNameByParameterName(const mi::base::Handle<const mi::neuraylib::IFunction_definition>& MaterialDefinition, const char* ParameterName, FString& DisplayName);
    static void UpdateParametersSheet(const mi::base::Handle<const mi::neuraylib::IFunction_definition>& MaterialDefinition, const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial, FMDLParametersList& List);
    static void MappingParameters(class UMaterial* Material, FMDLParametersList& List);

    static TMap<FString, struct FMDLParametersList> BaseParametersSheet;
    static TMap<FString, FString> BaseMaterialToModuleList;
    static TMap<FString, TArray<FString>> BaseImportSheet;

    static class UMaterialFunction* LoadMDLFunction(const FString& AssetPath, const FString& AssetName, const TArray<int32>& ArrayInputSizes = TArray<int32>());
    static void LoadMaterialGraphDefinitions();
    static void UnloadMaterialGraphDefinitions();
    static bool CreateMdlInstance(const FString& ModuleName, const FString& IdentifierName, const FString& InstanceName);
    static bool SetCall(const FString& InstanceTarget, const FString& ParameterName, const FString& InstanceCall);
    static bool DistillMaterialInstance(class UMaterial* Material, const FString& MaterialInstanceName, bool bUseDisplayName = true);
    static bool DistillCompiledMaterial(UMaterial* Material, const mi::base::Handle<const mi::neuraylib::IFunction_definition>& MaterialDefinition, const mi::base::Handle<const mi::neuraylib::ICompiled_material>& CompiledMaterial, bool bUseDisplayName = true, TArray<FString>* DistillErrors = nullptr, FLoadTextureCallback InCallback = nullptr);
    static void ClearMaterial(UMaterial* Material);
    static FString GetDistillerTargetName();
    static FString GetPrototype(const FString& MaterialName);
    static mi::base::Handle<const mi::neuraylib::ICompiled_material> CreateCompiledMaterial(const mi::base::Handle<mi::neuraylib::IScene_element>& MaterialInstanceSE);
    static mi::base::Handle<const mi::neuraylib::ICompiled_material> CreateCompiledMaterial(const mi::base::Handle<const mi::neuraylib::IFunction_call>& FunctionCall);

    static int32 SceneDataStringToEnum(const FString& SceneData);
    static FString GetContentPath(const FString& Path);

    static FString ProjectMdlRootPath;
    static bool IsHDRTexture(class UTexture* Texture);
};