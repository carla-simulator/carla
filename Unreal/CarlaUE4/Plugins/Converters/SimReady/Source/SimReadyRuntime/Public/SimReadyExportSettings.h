// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"

struct SIMREADYRUNTIME_API FSimReadyExportTextureSettings
{
    bool bTextureSource;
    bool bDDSExport;
    bool bReplaceTextureCube;

    FSimReadyExportTextureSettings();
};

struct SIMREADYRUNTIME_API FSimReadyExportMaterialSettings
{
    bool bCopyTemplate;
    bool bRayTracingTranslucency;
    bool bRayTracingRefraction;
    FString DestTemplatePath; // NOTE: always folder path, not file path
    bool bExportTwoSidedSign;

    FSimReadyExportTextureSettings TextureSettings;

    FSimReadyExportMaterialSettings();
};

struct SIMREADYRUNTIME_API FSimReadyExportCommandletSettings
{
    FString CommandletExportPath;

    FSimReadyExportCommandletSettings();
};

struct SIMREADYRUNTIME_API FSimReadyExportSettings
{
    bool bModular;
    bool bAsciiFormat;
    bool bAddExtraExtension;
    bool bMDL;
    bool bPreviewSurface;
    bool bExportAssetToSeperateUSD;
    bool bExportPhysics;
    bool bMeshInstanced;
    bool bCustomLayer;
    bool bExportInvisibleLevel;
    bool bPayloads;
    bool bExportSublayers;
    bool bUpYAxis;
    bool bExportDecalActors;
    bool bMaterialOverSublayer;
    bool bExportPreviewMesh;
    bool bForceShared;
    bool bExportLandscapeGrass;
    bool bRootIdentity;
    bool bExportInvisibleComponent;
    bool bExportXodr;
    bool bEditLayer;

    FSimReadyExportMaterialSettings MaterialSettings;

    FSimReadyExportCommandletSettings CommandletSettings;

    FSimReadyExportSettings();
};