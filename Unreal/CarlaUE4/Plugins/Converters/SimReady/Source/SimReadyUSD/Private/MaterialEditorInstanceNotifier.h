// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "CoreMinimal.h"
#include "MaterialEditor/MaterialEditorInstanceConstant.h"
#include "Materials/MaterialLayersFunctions.h"

class FMaterialEditorInstanceNotifier
{
public:
    static void SetVectorParameterValue(class UMaterialEditorInstanceConstant* EditorInstance, const FMaterialParameterInfo& ParameterInfo, FLinearColor Value);
    static void SetScalarParameterValue(class UMaterialEditorInstanceConstant* EditorInstance, const FMaterialParameterInfo& ParameterInfo, float Value);
    static void SetTextureParameterValue(class UMaterialEditorInstanceConstant* EditorInstance, const FMaterialParameterInfo& ParameterInfo, class UTexture* Value);

};