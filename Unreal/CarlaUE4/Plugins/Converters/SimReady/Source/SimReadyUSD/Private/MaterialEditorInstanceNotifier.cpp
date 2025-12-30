// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "MaterialEditorInstanceNotifier.h"
#include "MaterialEditor/MaterialEditorInstanceConstant.h"
#include "MaterialEditor/DEditorParameterValue.h"
#include "MaterialEditor/DEditorScalarParameterValue.h"
#include "MaterialEditor/DEditorTextureParameterValue.h"
#include "MaterialEditor/DEditorVectorParameterValue.h"
#include "Engine/Texture.h"

void FMaterialEditorInstanceNotifier::SetVectorParameterValue(UMaterialEditorInstanceConstant* EditorInstance, const FMaterialParameterInfo& ParameterInfo, FLinearColor Value)
{
    if (EditorInstance)
    {
        for (auto ParameterGroup : EditorInstance->ParameterGroups)
        {
            for (auto Param : ParameterGroup.Parameters)
            {
                UDEditorVectorParameterValue* VectorParam = Cast<UDEditorVectorParameterValue>(Param);
                if (VectorParam && VectorParam->ParameterInfo.Name == ParameterInfo.Name)
                {
                    VectorParam->Modify();
                    VectorParam->ParameterValue = Value;
                }
            }
        }
    }
}

void FMaterialEditorInstanceNotifier::SetScalarParameterValue(UMaterialEditorInstanceConstant* EditorInstance, const FMaterialParameterInfo& ParameterInfo, float Value)
{
    if (EditorInstance)
    {
        for (auto ParameterGroup : EditorInstance->ParameterGroups)
        {
            for (auto Param : ParameterGroup.Parameters)
            {
                UDEditorScalarParameterValue* ScalarParam = Cast<UDEditorScalarParameterValue>(Param);
                if (ScalarParam && ScalarParam->ParameterInfo.Name == ParameterInfo.Name)
                {
                    ScalarParam->Modify();
                    ScalarParam->ParameterValue = Value;
                }
            }
        }
    }
}

void FMaterialEditorInstanceNotifier::SetTextureParameterValue(UMaterialEditorInstanceConstant* EditorInstance, const FMaterialParameterInfo& ParameterInfo, UTexture* Value)
{
    if (EditorInstance)
    {
        for (auto ParameterGroup : EditorInstance->ParameterGroups)
        {
            for (auto Param : ParameterGroup.Parameters)
            {
                UDEditorTextureParameterValue* TextureParam = Cast<UDEditorTextureParameterValue>(Param);
                if (TextureParam && TextureParam->ParameterInfo.Name == ParameterInfo.Name)
                {
                    TextureParam->Modify();
                    TextureParam->ParameterValue = Value;
                }
            }
        }
    }
}

