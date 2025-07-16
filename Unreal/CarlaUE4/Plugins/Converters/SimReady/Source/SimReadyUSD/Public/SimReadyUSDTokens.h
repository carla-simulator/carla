// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "SimReadyPxr.h"

struct FSimReadyUSDTokens
{
    const pxr::TfToken ue4Material = pxr::TfToken("ue4Material");
    const pxr::TfToken ue4Asset = pxr::TfToken("ue4Asset"); // and Cloth asset, and...
    const pxr::TfToken mdlMaterial = pxr::TfToken("mdlMaterial");
    const pxr::TfToken mdl = pxr::TfToken("mdl");
    const pxr::TfToken module = pxr::TfToken("module");
    const pxr::TfToken name = pxr::TfToken("name");
    const pxr::TfToken path = pxr::TfToken("path");
    const pxr::TfToken sourceAsset = pxr::TfToken("sourceAsset");
    const pxr::TfToken id = pxr::TfToken("id");
    const pxr::TfToken previewSurface = pxr::TfToken("UsdPreviewSurface");
    const pxr::TfToken uvTexture = pxr::TfToken("UsdUVTexture");
    const pxr::TfToken sRGB = pxr::TfToken("sRGB");
    const pxr::TfToken raw = pxr::TfToken("raw");
    const pxr::TfToken auto_colorspace = pxr::TfToken("auto");
    const pxr::TfToken raw_deprecated = pxr::TfToken("RAW");
    const pxr::TfToken out = pxr::TfToken("out");
    const pxr::TfToken doNotCastShadows = pxr::TfToken("doNotCastShadows");
    const pxr::TfToken none = pxr::TfToken("none");
    const pxr::TfToken subset = pxr::TfToken("subset");
    const pxr::TfToken tangentX = pxr::TfToken("tangentX");
    const pxr::TfToken varname = pxr::TfToken("varname");
    const pxr::TfToken surface = pxr::TfToken("surface");
    const pxr::TfToken st = pxr::TfToken("st");
    const pxr::TfToken file = pxr::TfToken("file");
    const pxr::TfToken result = pxr::TfToken("result");
    const pxr::TfToken displayColor = pxr::TfToken("displayColor");
    const pxr::TfToken leftHanded = pxr::TfToken("leftHanded");
    const pxr::TfToken sourceColorSpace = pxr::TfToken("sourceColorSpace");
    const pxr::TfToken PrimvarReader_float2 = pxr::TfToken("UsdPrimvarReader_float2");
    const pxr::TfToken Transform2d = pxr::TfToken("UsdTransform2d");
};

static const FSimReadyUSDTokens USDTokens;