// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyExportSettings.h"

FSimReadyExportTextureSettings::FSimReadyExportTextureSettings()
    : bTextureSource(false)
    , bDDSExport(false)
    , bReplaceTextureCube(false)
{
}

FSimReadyExportMaterialSettings::FSimReadyExportMaterialSettings()
    : bCopyTemplate(true)
    , bRayTracingTranslucency(false)
    , bRayTracingRefraction(true)
    , bExportTwoSidedSign(false)
{
}

FSimReadyExportCommandletSettings::FSimReadyExportCommandletSettings()
    : CommandletExportPath{"/Users/commandlet"}
{
}

FSimReadyExportSettings::FSimReadyExportSettings()
    : bModular(false)
    , bAsciiFormat(false)
    , bAddExtraExtension(false)
    , bMDL(true)
    , bPreviewSurface(false)
    , bExportAssetToSeperateUSD(true)
    , bExportPhysics(false)
    , bMeshInstanced(false)
    , bCustomLayer(true)
    , bExportInvisibleLevel(false)
    , bPayloads(true)
    , bExportSublayers(false)
    , bUpYAxis(false)
    , bExportDecalActors(false)
    , bMaterialOverSublayer(false)
    , bExportPreviewMesh(false)
    , bForceShared(false)
    , bExportLandscapeGrass(true)
    , bRootIdentity(false)
    , bExportInvisibleComponent(true)
    , bExportXodr(true)
    , bEditLayer(false)
{
}