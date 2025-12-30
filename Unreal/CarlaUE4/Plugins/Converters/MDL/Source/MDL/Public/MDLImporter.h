// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#if WITH_MDL_SDK

#include "MDLDependencies.h"
#include "MDLParameter.h"
#include "Engine/Texture.h"

typedef TFunction<void(class UTexture*&, const FString&, float Gamma, TextureCompressionSettings Compression)> FLoadTextureCallback;

class MDL_API IMDLMaterialImporter
{
public:

    virtual bool ImportMaterial(class UMaterial* Material, const mi::base::Handle<const mi::neuraylib::IFunction_definition>& InMaterialDefinition, const mi::base::Handle<const mi::neuraylib::ICompiled_material>& InCompiledMaterial, class UMaterialExpressionClearCoatNormalCustomOutput*& OutClearCoatNormalCustomOutput, FLoadTextureCallback InCallback = nullptr) = 0;
    virtual bool ImportDistilledMaterial(class UMaterial* Material, const mi::base::Handle<const mi::neuraylib::IFunction_definition>& InMaterialDefinition, const mi::base::Handle<const mi::neuraylib::ICompiled_material>& InCompiledMaterial, class UMaterialExpressionClearCoatNormalCustomOutput*& OutClearCoatNormalCustomOutput, FLoadTextureCallback InCallback = nullptr) = 0;
    virtual bool IsDistillOff(const mi::base::Handle<const mi::neuraylib::IFunction_definition>& InMaterialDefinition) = 0;
    virtual const TArray<FString>& GetLastImportErrors() = 0;
    virtual ~IMDLMaterialImporter() {}
};

#endif
