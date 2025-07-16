// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once
#include "CoreMinimal.h"
#include "MDLExporterUtility.h"

class FMDLTemplateExporter
{
public:
    FMDLTemplateExporter(class UMaterialInterface* InMaterial, const FMDLExporterSetting& InSetting, FExportTextureCallback InCallback);
    bool ExportModule(FString& Output, struct FMDLTranslatorResult* OutResult);

private:
    void ExportHeader();
    void ExportImports(bool Extension17);
    void ExportAnnotationsDefine();
    void ExportMaterial();
    void ExportParamters();
    void ExportLetBlock();
    void ExportModuleBlock();
    void ExportAnnotation();
    void ExportCustomExpressions();
    bool Translate(struct FMDLTranslatorResult& OutResult);
    FMDLTranslatorResult* IsCached(const FStaticParameterSet& InStaticParamSet);
    void UpdateExportedMaterial(const FMDLTranslatorResult& InResult);
    void AssembleLetExpressions();
    void RemoveUnusedVariables(struct FMDLTranslatorResult& TranslatorResult);

private:
    class UMaterialInterface* OriginalMaterial;
    FMDLExporterSetting ExporterSetting;

    TArray<FString> InputCodes;
    TArray<FString> LocalVariables;
    TArray<FString> InputVariables;
    FString NormalCode;
    FString NormalAssignment;
    FString VertexMembersSetupAndAssignments;
    FString PixelMembersSetupAndAssignments;
    FString CustomExpressions;
    FString CustomVSOutputCode;
    FString CustomPSOutputCode;
    FString LetExpressions;

    FString UsedTemplate;
    
    FString ExportedMDLString; // Main export mdl
    FExportTextureCallback ExportTextureCallback;
};

