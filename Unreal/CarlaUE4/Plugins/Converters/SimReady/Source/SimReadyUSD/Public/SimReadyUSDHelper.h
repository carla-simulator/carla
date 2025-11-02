// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "SimReadyPxr.h"
#include "MeshDescription.h"
#include "SimReadyExportSettings.h"
#include "SceneTypes.h"
#include "LevelSequenceActor.h"

class UMaterialInterface;

class SIMREADYUSD_API FSimReadyUSDHelper
{
public:
    static bool ExportObjectAsUSDToPath(class UObject* Object, const FString& ExportPath, const FSimReadyExportSettings& ExportSettings);

    static bool ExportUWorldAsUSDToPath(class UWorld* World, const FString& ExportWorldStagePath, const FSimReadyExportSettings& ExportSettings);

    static bool ExportActorsAsUSDToPath(const TArray<class AActor*>& Actors, const FString& ExportUSDPath, const FSimReadyExportSettings& ExportSettings);

    static void SetTransfromForXForm(pxr::UsdGeomXformable& XForm, const pxr::GfMatrix4d& Transform);

    static void GetActorsToExportFromWorld(class UWorld* World, bool bSelectedOnly, const FSimReadyExportSettings& ExportSettings, TArray<AActor*>& ActorToExport, TArray<class ALevelSequenceActor*>& LevelSequenceActors);

    // Clear existing or create a new one, if bWithBackgroundLayer is true
    // it will create a stage with a default active background layer
    static pxr::UsdStageRefPtr CreateUSDStageFromPath(const FString& Path, bool bKeepExistingStage, bool bZUpAxis);

    static pxr::UsdStageRefPtr LoadUSDStageFromPath(const FString& Path);

    static pxr::SdfPath InitDefaultPrim(const pxr::UsdStageRefPtr& USDStage, const pxr::TfToken& PrimKind = pxr::KindTokens->assembly);

private:
    static pxr::UsdShadeMaterial BindMaterial(const pxr::UsdStageRefPtr& USDStage, const pxr::UsdPrim& Prim, const pxr::SdfPath& ScopePrimPath, const FString& MaterialName);
    static void CreateMDLSchema(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& MaterialPrimPath, bool MdlSchema, const FString& MaterialName, const FString& MaterialRelativePath);
    static void CreatePSSchema(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& MaterialPrimPath, const FString& MaterialName, const TMap<EMaterialProperty, FString>& PropertyTextures, const TMap<EMaterialProperty, struct FLinearColor>& PropertyConstants);
    static pxr::UsdStageRefPtr CreateUsdMaterialStage(const FString& MaterialUsdPath, const FSimReadyExportSettings& ExportSettings);
    static pxr::UsdShadeMaterial DefineUsdMaterial(const pxr::UsdStageRefPtr& USDStage, const FString& MaterialName);
    static void BindMaterialOverrides(const pxr::UsdStageRefPtr& USDStage, const pxr::UsdPrim& Prim, const class UMaterialInterface* MaterialInt, const FString& MDLRelativePath, const TMap<FString, FString>& ExportedMDLs, const FSimReadyExportSettings& ExportSettings);

    static pxr::UsdGeomXformable ExportStaticMeshToUSD(class UStaticMesh* StaticMesh, class UStaticMeshComponent* StaticMeshComponent, const pxr::UsdStageRefPtr& USDStage,
        const pxr::SdfPath& ExportPrimPath, const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
        bool bOverrideMaterial, const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs);

    static bool ExportStaticMeshAsUSDToPathInternal(class UStaticMesh* StaticMesh, class UStaticMeshComponent* StaticMeshComponent, const FString& OmniMeshPath,
        const FString& ExportMDLBaseAbsolutePath, bool bOverrideMaterial, const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs);

    static bool ExportOverrideMeshAsUSDToPath(UStaticMeshComponent* StaticMeshComponent, const FString& USDAbsolutePath, const FString& MeshUSDRelativePathToRoot, const FString& MeshUSDPath, int32 LODToExport, const FString& MDLAbsoluteBasePath, const FString& MDLRelativeBasePathToRoot, const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs);

    static bool ExportComponentToUSD(class USceneComponent* SceneComp, const pxr::UsdStageRefPtr& USDStage, const pxr::UsdStageRefPtr& MaterialOversStage, const pxr::SdfPath& RootPath, const FSimReadyExportSettings& ExportSettings,
        TMap<FString, FString>& ExportedMeshUSD, TMap<FString, FString>& ExportedMDLs, TMap<FString, FString>& ExportedAnimeUSD);

    static pxr::UsdGeomXformable ExportLandscapeToUSD(class ALandscapeProxy* LandscapeProxy, const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& ExportPrimPath,
            const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
            const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs, TMap<FString, FString>& ExportedMeshUSD,
            bool bStandaloneUSD = false, const FString& ExportMeshPath = TEXT(""));

    static bool ExportLandscapeToUSDInternal(class ALandscapeProxy* LandscapeProxy,
        const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
        const FSimReadyExportSettings& ExportSettings,
        TMap<FString, FString>& ExportedMDLs, TMap<FString, FString>& ExportedMeshUSD, const FString& ExportMeshPath);

    template<typename InstancedComponent>
    static pxr::UsdGeomPointInstancer ExportInstancedMeshesToPointInstancer(const TArray<InstancedComponent*>& InstancedStaticMeshComponents, const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& ExportPrimPath,
        const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs, TMap<FString, FString>& ExportedMeshUSD, bool bRenderData = false, const FString& ExportMeshPath = TEXT(""));

    template<typename InstancedComponent>
    static bool ExportInstancedMeshesToPointInstancerInternal(const TArray<InstancedComponent*>& InstancedStaticMeshComponents,
        const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs, TMap<FString, FString>& ExportedMeshUSD,
        bool bRenderData, const FString& ExportMeshPath);

    static pxr::UsdGeomXformable ExportConvertedMeshToUSD(class USceneComponent* SceneComponent, const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& ExportPrimPath,
        const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
        const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs);

    static bool ExportConvertedMeshToUSDInternal(class USceneComponent* SceneComponent,
        const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
        const FSimReadyExportSettings& ExportSettings,
        TMap<FString, FString>& ExportedMDLs, const FString& ExportMeshPath);

    static pxr::UsdSkelRoot ExportSkeletalMeshToUSD(class USkeletalMesh* SkeletalMesh, class USkeletalMeshComponent* SkeletalMeshComponent,
        const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& ExportPrimPath, const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
        bool bOverrideMaterial, const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs);

    static bool ExportSkeletalMeshAsUSDToPathInternal(class USkeletalMesh* SkeletalMesh, class USkeletalMeshComponent* SkeletalMeshComponent, const FString& OmniMeshPath,
        const FString& ExportMDLBaseAbsolutePath, bool bOverrideMaterial, const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs);

    static bool ExportAnimationAssetAsUSDToPathInternal(class UAnimationAsset * AnimationAsset, class USkeletalMeshComponent* SkeletalMeshComponent, const FString & ExportAnimePath, const FSimReadyExportSettings& ExportSettings);

    static bool ExportAnimationTimeSamplesAsUSDToPathInternal(const TArray<struct FSkeletalAnimationTimeSamples>* AnimationTimeSamples, const double StartTimeCode, const double EndTimeCode, const double TimeCodesPerSecond, class USkeletalMeshComponent* SkeletalMeshComponent, const FString & ExportAnimePath, const FSimReadyExportSettings& ExportSettings);

    static bool ExportObjectAsUSDToPathInternal(class UObject* Object, const FString & ExportPath, const FSimReadyExportSettings& ExportSettings);

    static bool ExportBlueprintAsUSDToPathInternal(class UBlueprint* Blueprint, const FString& ExportPath, const FSimReadyExportSettings& ExportSettings);

    typedef TFunction<void(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& PrimPath, const struct FSimReadyExportMaterialResult&, FString&)> FExportMaterialPreCallback;
    typedef TFunction<void(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& PrimPath, const FString&, const struct FSimReadyExportMaterialResult&, const FString&)> FExportMaterialPostCallback;

    static void ExportAndBindMaterial(const pxr::UsdStageRefPtr& USDStage, class UMaterialInterface* MaterialInterface, const struct FNamedParameterTimeSamples* ParameterTimeSamples, const pxr::UsdPrim& Prim, const FSimReadyExportSettings& ExportSettings, const FString& MDLAbsolutePath, const FString& MDLRelativePath, TMap<FString, FString>& ExportedMDLs, const FString& CustomMaterialName = TEXT(""), const FString& CustomUsdMaterialName = TEXT(""), FExportMaterialPreCallback InPreCallback = nullptr, FExportMaterialPostCallback InPostCallback = nullptr);

    static bool TryExportAndBindMaterialWithAOMask(class UStaticMeshComponent* StaticMeshComponent, const struct FNamedParameterTimeSamples* ParameterTimeSamples, int32 LODToExport, const pxr::UsdStageRefPtr& USDStage, class UMaterialInterface* MaterialInt, const pxr::UsdPrim& Prim,
        const FSimReadyExportSettings& ExportSettings, const FString& MDLBaseAbsolutePath, const FString& MDLBaseRelativePath, TMap<FString, FString>& ExportedMDLs);

    static void BakePreviewSurface(UMaterialInterface* MaterialInterface, const FString& MaterialName, const FString& AbsolutePath, const FString& RelativePath, const TMap<EMaterialProperty, struct FLinearColor>& ConstantProperties, TMap<EMaterialProperty, FString>& OutTextureProperties);

    static void ExportModelToUSD(class UModel* Model, const pxr::UsdStageRefPtr& USDStage, const pxr::UsdStageRefPtr& MaterialOversStage, const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMeshUSD, TMap<FString, FString>& ExportedMDLs);
    
    static void ExportInstanceParametersToMDLSchema(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& MaterialPrimPath, const FString& MaterialName, class UMaterialInstance* InstanceDynamic, const struct FSimReadyExportMaterialResult& MaterialResult, const FString& MDLAbsolutePath, bool bMaterialNameAsTextureFolder, const FSimReadyExportTextureSettings& TextureSettings);

    static void ExportParameterTimeSamplesToMDLSchema(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& MaterialPrimPath, const FString& MaterialName, UMaterialInterface* Material, const struct FNamedParameterTimeSamples& ParameterTimeSamples, const FSimReadyExportMaterialResult& MaterialResult, const FString& MDLAbsolutePath, bool bMaterialNameAsTextureFolder, const FSimReadyExportTextureSettings& TextureSettings);

    static bool IsActorSupported(const AActor* Actor, const FSimReadyExportSettings& ExportSettings);

    static int32 SuffixIndex;

    friend class USimReadyExportCommandlet;
};