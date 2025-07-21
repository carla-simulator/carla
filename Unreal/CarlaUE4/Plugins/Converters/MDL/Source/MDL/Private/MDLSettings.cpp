// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "MDLSettings.h"

UMDLSettings::UMDLSettings(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bImportOnlyFirstMaterialPerFile(false)
    , bAutoSaveMaterialFunctions(true)
    , bWorldAlignedTextures(false)
    , bInstanceCompilation(false)
    , bDistillation(true)
    , DistillationTarget(EDistillationTarget::UE4)
    , FluxDivider(50.0f)
    , bUseDisplayNameForParameter(true)
    , BakedTextureHeight(1024)
    , BakedTextureWidth(1024)
    , BakedTextureSamples(16)
    , MetersPerSceneUnit(0.01f)
{}
