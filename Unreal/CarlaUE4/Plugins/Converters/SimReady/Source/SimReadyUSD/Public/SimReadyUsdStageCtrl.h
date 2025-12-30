// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "SimReadyPxr.h"

class SIMREADYUSD_API SimReadyUsdStageCtrl
{
public:

    static bool RemovePrimSpecInEditLayer(const pxr::UsdPrim& prim);
    static pxr::SdfLayerHandle  HasOverriddenInStrongerLayer(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& PrimPath, bool bOnlyActivePrim = false);
    static int  FindSubLayerIndex(const pxr::SdfLayerHandle& rootLayer, const std::string& path);
    static bool  SetSubLayerName(const pxr::SdfLayerHandle& rootLayer, int index, const pxr::TfToken& name);
    static pxr::SdfLayerRefPtr  CreateSubLayer(const pxr::SdfLayerHandle& rootLayer, int index);
    static bool  RemovePrim(const pxr::UsdPrim& prim);
    static bool  RestorePrim(const pxr::SdfLayerRefPtr& AnonymousLayer, const std::string& LayerIdentifier, const pxr::SdfPath& PrimPath);
    static bool  CopyPrim(const pxr::UsdPrim& prim, pxr::SdfPath dstPath = pxr::SdfPath());
    static bool  RenamePrim(const pxr::UsdPrim& prim, const pxr::TfToken& newName);
    static bool IsAncestralPrim(const pxr::UsdPrim& Prim);
    static bool IsAncestorGprim(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& PrimPath);
    static bool CanRemovePrim(const pxr::UsdPrim& prim);
    static bool GetIntroducingLayer(const pxr::UsdPrim& prim, pxr::SdfLayerHandle& outLayer, pxr::SdfPath& outPath);
    static void ResolvePaths(const std::string& srcLayerIdentifier, const std::string& targetLayerIdentifier, bool storeRelativePath, bool relativeToSrcLayer = false, bool copySublayerLayerOffsets = false);
    static bool MergePrimSpec(const std::string& dstLayerIdentifier, const std::string& srcLayerIdentifier, const std::string& primSpecPath, bool isDstStrongerThanSrc, const std::string& targetPrimPath = "");
    static std::vector<std::string> GetAllSublayers(const pxr::UsdStageRefPtr& stage, bool includeSessionLayer);
    static void ResolvePrimPathReferences(const pxr::SdfLayerRefPtr& layer, const pxr::SdfPath& oldPath, const pxr::SdfPath& newPath);
    static bool StitchPrimSpecs(const pxr::UsdPrim& prim, pxr::SdfPath dstPath);
    static bool IsLayerExist(const std::string& layerIdentifier);
    static void ResolvePathsInternal(const pxr::SdfLayerRefPtr& srcLayer, pxr::SdfLayerRefPtr dstLayer, bool storeRelativePath, bool relativeToSrcLayer = false, bool copyLayerOffsets = false);
    static bool MergePrimSpecInternal(pxr::SdfLayerRefPtr dstLayer, const pxr::SdfLayerRefPtr& srcLayer, const pxr::SdfPath& primSpecPath, bool isDstStrongerThanSrc, const pxr::SdfPath& targetPrimPath = pxr::SdfPath::EmptyPath());

    static void DebugLayerToFile(const pxr::SdfLayerHandle& Layer, const FString& FileName);
};
