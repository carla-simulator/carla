// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "SimReadyPxr.h"
#include "SimReadyTexture.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Texture.h"

class SIMREADYUSD_API FSimReadyUSDImporterHelper
{
public:
    //Import
    static bool USDImportMesh(const pxr::UsdGeomMesh& USDMesh, class UStaticMesh& Mesh, const FVector& Offset);

    static bool USDImportSkeleton(const pxr::UsdSkelSkeletonQuery& USDSkeletonQuery, class FSkeletalMeshImportData& SkeletalMeshImportData);
    static bool USDImportSkinning(const pxr::UsdSkelSkeletonQuery& USDSkeletonQuery, const pxr::UsdSkelSkinningQuery& USDSkinningQuery, class FSkeletalMeshImportData& SkeletalMeshImportData);

    static void BuildSourceModel(class UStaticMesh* Mesh);
    static class UStaticMesh* CreateStaticMesh(const pxr::UsdGeomMesh& USDMesh, class UObject* Parent, FName Name, EObjectFlags Flags, const FVector& Offset = FVector::ZeroVector);
    static class UAnimSequence* CreateAnimSequence(const pxr::UsdSkelSkeletonQuery& USDSkeletonQuery, class UObject* Parent, FName Name, EObjectFlags Flags, class USkeleton* Skeleton, class USkeletalMesh* PreviewMesh, double TimeCodesPerSecond, double StartTimeCode, double EndTimeCode, FBox* BoundingBox = nullptr);

    static bool GetMdlPathAndName(pxr::UsdShadeShader ShadeShader, FString& MdlPath, FString& MaterialName, bool& bRelativePath);
    static bool LoadMaterialGraph(const pxr::UsdShadeShader& ShadeShader, class UMaterial* Material);
    
    static void UpdatePreviewSurfaceInputs(
        class UMaterialInstanceConstant* MaterialInst,
        const pxr::UsdShadeShader& ShadeShader,
        const TMap<FString, int32>* UVSets,
        TFunction<void(const FString&)> Callback = nullptr,
        TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback = nullptr
    );
    static bool UpdateMaterialGraphInputs(
        const pxr::UsdShadeShader& ShadeShader,
        class UMaterialInstanceConstant* Instance,
        TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback = nullptr
    );
    static bool LoadMdlInput(
        class UMaterialInstanceConstant& InstanceConstant,
        const pxr::UsdShadeInput& Input,
        const FString& DisplayName,
        bool bCheckParameter = false,
        TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback = nullptr
    );

private:
    static void LinkNodeGraphShader(const pxr::UsdShadeShader& ShadeShader, FString& InstanceName);
    static void LinkNodeGraphShader(const pxr::UsdShadeNodeGraph& NodeGraph, const pxr::TfToken& SourceName, const FString& InputName, const FString& InstanceName);
    static bool USDImportMesh(const pxr::UsdGeomMesh& USDMesh, struct FStaticMeshLODResources& MeshRes, const FVector& Offset);
    static bool USDImportMesh(const pxr::UsdGeomMesh& USDMesh, const FTransform& GeomTransform, TArray<struct FStaticMeshBuildVertex>& Vertices, TArray<FVector>& Points, TArray<uint32>& Indices, TArray<uint32>& FacePosInIndices, TArray<uint32>& FacePosInVertices, int32& TriangleCount, int32& UVCount, const FVector& Offset);
    static void UpdateNodeGraphInput(
        class UMaterialInstanceConstant* Instance,
        const pxr::UsdShadeShader& ShadeShader,
        const FString& ParentInputName,
        TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback
    );
    static void UpdateNodeGraphInput(
        class UMaterialInstanceConstant* Instance,
        const pxr::UsdShadeNodeGraph& NodeGraph,
        const pxr::TfToken& SourceName,
        const FString& ParentInputName,
        TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback
    );
    static bool LoadTextureByShadeInput(
        const pxr::UsdShadeInput& FileInput,
        const pxr::UsdShadeInput& SourceColorSpace,
        class UMaterialInstanceConstant& MaterialInstance,
        class UMaterialEditorInstanceConstant* EditorInst,
        const FString& PropertyName,
        bool bSetDefaultColorSpace,
        bool bDefaultSRGB,
        TFunction<void(const uint8*, uint64, const FString&, UTexture*&)> CreateCallback
    );
};