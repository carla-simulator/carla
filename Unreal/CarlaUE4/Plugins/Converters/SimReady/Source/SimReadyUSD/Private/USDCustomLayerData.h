// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "SimReadyPxr.h"



class FUSDCustomLayerData
{
public:
    static void ResetMaxRoughness(const pxr::SdfLayerRefPtr& RootLayer);
    static void SetEditorCamera(const pxr::UsdStageRefPtr& USDStage, const FVector& CamPosition, const FVector& CamTarget);
    static void SetRenderSettings(const pxr::SdfLayerRefPtr& RootLayer, const struct FPostProcessSettings& Settings);
    static void SetLayerCustomField(const pxr::SdfLayerRefPtr& RootLayer, const std::string& LayerIdentifier, const std::string& Key, bool Value);
    static void SetAuthoringLayerCustomField(const pxr::SdfLayerRefPtr& RootLayer, const std::string& LayerIdentifier);

    static bool GetLayerCustomField(pxr::SdfLayerRefPtr RootLayer, const std::string& LayerIdentifier, const std::string& Key);

    static const std::string MutenessCustomKey;
    static const std::string LockedCustomKey;
    static const std::string AuthoringLayerCustomKey;
};