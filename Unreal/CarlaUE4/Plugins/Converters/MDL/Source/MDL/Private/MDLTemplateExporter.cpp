// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "MDLTemplateExporter.h"
#include "MDLExporterUtility.h"
#include "MDLMaterialTranslator.h"

#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Logging/TokenizedMessage.h"

const static FString MDLVersion = TEXT("1.6");
const static FString MDLMagicName = TEXT("<?Name?>");

inline bool IsSubsurfaceColorUsed(FMaterialShadingModelField ShadingModel)
{
    return IsSubsurfaceShadingModel(ShadingModel) && !UseSubsurfaceProfile(ShadingModel);
}

FMDLTemplateExporter::FMDLTemplateExporter(UMaterialInterface* InMaterial, const FMDLExporterSetting& InSetting, FExportTextureCallback InCallback)
    : OriginalMaterial(InMaterial)
    , ExporterSetting(InSetting)
    , ExportTextureCallback(InCallback)
{
    if (ExporterSetting.Name.IsEmpty())
    {
        ExporterSetting.Name = OriginalMaterial ? OriginalMaterial->GetName() : TEXT("");
    }
}

FMDLTranslatorResult* FMDLTemplateExporter::IsCached(const FStaticParameterSet& InStaticParamSet)
{
    UMaterial* BaseMaterial = OriginalMaterial->GetMaterial();
    auto TranslatorResults = FMDLExporterUtility::GlobalExportedMaterials.Find(BaseMaterial);
    if (TranslatorResults == nullptr)
    {
        return nullptr;
    }

    for (int32 Index = 0; Index < TranslatorResults->Num(); ++Index)
    {
        auto TranslatorResult = (*TranslatorResults)[Index];

        if (TranslatorResult.BlendMode != OriginalMaterial->GetBlendMode())
        {
            continue;
        }
        if (TranslatorResult.ShadingModel != OriginalMaterial->GetShadingModels().GetFirstShadingModel())
        {
            continue;
        }
        if (TranslatorResult.TwoSided != OriginalMaterial->IsTwoSided())
        {
            continue;
        }
        if (TranslatorResult.OpacityMaskClipValue != OriginalMaterial->GetOpacityMaskClipValue())
        {
            continue;
        }
        if (TranslatorResult.LandscapeMaterial != ExporterSetting.bLandscape)
        {
            continue;
        }

        if (TranslatorResult.StaticParamSet.StaticSwitchParameters.Num() != InStaticParamSet.StaticSwitchParameters.Num()
            || TranslatorResult.StaticParamSet.StaticComponentMaskParameters.Num() != InStaticParamSet.StaticComponentMaskParameters.Num()
            || TranslatorResult.StaticParamSet.TerrainLayerWeightParameters.Num() != InStaticParamSet.TerrainLayerWeightParameters.Num()
            || TranslatorResult.StaticParamSet.MaterialLayersParameters.Num() != InStaticParamSet.MaterialLayersParameters.Num())
        {
            continue;
        }

        if (TranslatorResult.StaticParamSet.StaticSwitchParameters != InStaticParamSet.StaticSwitchParameters)
        {
            continue;
        }

        if (TranslatorResult.StaticParamSet.StaticComponentMaskParameters != InStaticParamSet.StaticComponentMaskParameters)
        {
            continue;
        }

        if (TranslatorResult.StaticParamSet.TerrainLayerWeightParameters != InStaticParamSet.TerrainLayerWeightParameters)
        {
            continue;
        }

        if (TranslatorResult.StaticParamSet.MaterialLayersParameters != InStaticParamSet.MaterialLayersParameters)
        {
            continue;
        }

        return &((*TranslatorResults)[Index]);
    }

    return nullptr;
}

void FMDLTemplateExporter::UpdateExportedMaterial(const FMDLTranslatorResult& InResult)
{
    FMaterialResource* MaterialResource = OriginalMaterial->GetMaterialResource(GMaxRHIFeatureLevel);
    if (MaterialResource == nullptr)
    {
        return;
    }
    FStaticParameterSet StaticParamSet;
    MaterialResource->GetStaticParameterSet(GMaxRHIShaderPlatform, StaticParamSet);

    FMDLTranslatorResult* CachedResult = IsCached(StaticParamSet);
    if (CachedResult) // Shouldn't be nullptr
    {
        *CachedResult = InResult;
    }
}

bool FMDLTemplateExporter::Translate(FMDLTranslatorResult& TranslatorResult)
{
    bool bSuccess = false;

    FMaterialResource* MaterialResource = OriginalMaterial->GetMaterialResource(GMaxRHIFeatureLevel);
    if (MaterialResource == nullptr)
    {
        return bSuccess;
    }

    FMaterialCompilationOutput NewCompilationOutput;
    FMaterialShaderMapId ShaderMapId;
    MaterialResource->GetShaderMapId(GMaxRHIShaderPlatform, nullptr, ShaderMapId);
    FStaticParameterSet StaticParamSet;
    MaterialResource->GetStaticParameterSet(GMaxRHIShaderPlatform, StaticParamSet);

    if (!ExporterSetting.bIgnoreCache)
    {
        FMDLTranslatorResult* CachedResult = IsCached(StaticParamSet);
        if (CachedResult)
        {
            TranslatorResult = *CachedResult;
            return bSuccess;
        }
    }

    FMDLMaterialTranslator MaterialTranslator(MaterialResource, NewCompilationOutput, StaticParamSet, GMaxRHIShaderPlatform, 
        MaterialResource->GetQualityLevel(), GMaxRHIFeatureLevel, OriginalMaterial->GetMaterial(), 
        ExporterSetting.bModular ? Cast<UMaterialInstance>(OriginalMaterial) : nullptr, ExporterSetting.bModular ? ExporterSetting.Name : TEXT("Textures"), 
        ExporterSetting.bLandscape, ExporterSetting.bDDSTexture,
        ExporterSetting.bRayTracingTranslucency, ExporterSetting.bRayTracingRefraction, ExporterSetting.bUpZAxis);

    MaterialTranslator.CurrentTextureCache = &FMDLExporterUtility::GlobalUsedTextures;

    bSuccess = MaterialTranslator.Translate();
    if (bSuccess)
    {
        // Output the implementation for any custom expressions we will call below.
        for (int32 ExpressionIndex = 0; ExpressionIndex < MaterialTranslator.GetCustomExpressions().Num(); ExpressionIndex++)
        {
            const FMaterialCustomExpressionEntry& CustomExpression = MaterialTranslator.GetCustomExpressions()[ExpressionIndex];
            CustomExpressions += CustomExpression.Implementation + LINE_TERMINATOR + LINE_TERMINATOR;
        }
        for (int32 OutputIndex = 0; OutputIndex < MaterialTranslator.GetCustomVSOutputImplementations().Num(); OutputIndex++)
        {
            CustomVSOutputCode += MaterialTranslator.GetCustomVSOutputImplementations()[OutputIndex] + LINE_TERMINATOR;
        }

        for (int32 OutputIndex = 0; OutputIndex < MaterialTranslator.GetCustomPSOutputImplementations().Num(); OutputIndex++)
        {
            CustomPSOutputCode += MaterialTranslator.GetCustomPSOutputImplementations()[OutputIndex] + LINE_TERMINATOR;
        }
            
        TranslatorResult.ScalarParameters = MaterialTranslator.DefinedScalarParameters;
        TranslatorResult.VectorParameters = MaterialTranslator.DefinedVectorParameters;
        TranslatorResult.TextureParameters = MaterialTranslator.DefinedTextureParameters;
        TranslatorResult.BoolParameters = MaterialTranslator.DefinedBoolParameters;

#define SET_FLAG(Enabled, UsageFlag) \
        { if (Enabled) TranslatorResult.SetUsageFlag(UsageFlag); else TranslatorResult.ClearUsageFlag(UsageFlag); }
        
        SET_FLAG(MaterialTranslator.bPerlinGradientTextureUsed, MaterialUsage_PerlinGradientTexture);
        SET_FLAG(MaterialTranslator.bPerlin3DTextureUsed, MaterialUsage_Perlin3DTexture);
        SET_FLAG(MaterialTranslator.bVertexColorUsed, MaterialUsage_VertexColor);
        SET_FLAG(MaterialTranslator.bPerInstanceDataUsed, MaterialUsage_PerInstanceData);
        SET_FLAG(MaterialTranslator.bTwoSidedSignUsed, MaterialUsage_TwoSidedSign);
        SET_FLAG(MaterialTranslator.bExtension17Used, MaterialUsage_Extension17);
        SET_FLAG(MaterialTranslator.bAOMaterialMaskUsed, MaterialUsage_AOMaterialMaskTexture);
        SET_FLAG(MaterialTranslator.bLightMapCoordinateUsed, MaterialUsage_LightMapCoordinate);
        SET_FLAG(MaterialTranslator.bMaxTexCoordinateUsed, MaterialUsage_MaxTexCoordinate);
#undef SET_FLAG

        //compile
        TranslatorResult.StaticParamSet = StaticParamSet;
        TranslatorResult.BlendMode = OriginalMaterial->GetBlendMode();
        TranslatorResult.ShadingModel = OriginalMaterial->GetShadingModels().GetFirstShadingModel();
        TranslatorResult.OpacityMaskClipValue = OriginalMaterial->GetOpacityMaskClipValue();
        TranslatorResult.TwoSided = OriginalMaterial->IsTwoSided();
        TranslatorResult.LandscapeMaterial = ExporterSetting.bLandscape;
        TranslatorResult.MDLName = ExporterSetting.Name;

        TArray<EMaterialProperty> IgnoredProperties;
        FMaterialShadingModelField MaterialShadingModels = OriginalMaterial->GetShadingModels(); 
        if (IsTranslucentBlendMode(OriginalMaterial->GetBlendMode()))
        {
            IgnoredProperties.Add(MP_OpacityMask);
            // UE4.26 later
            if (MaterialResource->GetMaterialDomain() != MD_Volume)
            {
                // UE4.26 Domain : SubsurfaceColor = UE4.25 Domain : Opacity
            IgnoredProperties.Add(MP_SubsurfaceColor);
            }
            
            IgnoredProperties.Add(MP_CustomData0);
            IgnoredProperties.Add(MP_CustomData1);
            if (ExporterSetting.bRayTracingTranslucency)
            {
                IgnoredProperties.Add(MP_Refraction);
            }
            if (MaterialShadingModels.GetFirstShadingModel() == MSM_Unlit)
            {
                IgnoredProperties.Add(MP_BaseColor);
                IgnoredProperties.Add(MP_Metallic);
                IgnoredProperties.Add(MP_Specular);
                IgnoredProperties.Add(MP_Roughness);
                IgnoredProperties.Add(MP_Normal);
            }
        }
        else if (IsSubsurfaceColorUsed(MaterialShadingModels.GetFirstShadingModel()))
        {
            IgnoredProperties.Add(MP_CustomData0);
            IgnoredProperties.Add(MP_CustomData1);
            IgnoredProperties.Add(MP_Refraction);
        }
        else
        {
            IgnoredProperties.Add(MP_Opacity);
            IgnoredProperties.Add(MP_SubsurfaceColor);
            IgnoredProperties.Add(MP_Refraction);

            if (MaterialShadingModels.GetFirstShadingModel() != MSM_ClearCoat)
            {
                IgnoredProperties.Add(MP_CustomData0);
                IgnoredProperties.Add(MP_CustomData1);	
            }
            
            if (MaterialShadingModels.GetFirstShadingModel() == MSM_Eye)
            {
                IgnoredProperties.Add(MP_Metallic);
            }

            if (MaterialShadingModels.GetFirstShadingModel() == MSM_Unlit)
            {	
                IgnoredProperties.Add(MP_BaseColor);
                IgnoredProperties.Add(MP_Metallic);
                IgnoredProperties.Add(MP_Specular);
                IgnoredProperties.Add(MP_Roughness);
                IgnoredProperties.Add(MP_Normal);
                IgnoredProperties.Add(MP_Refraction);
            }
        }

        IgnoredProperties.Add(MP_Anisotropy); //4.25
        IgnoredProperties.Add(MP_Tangent); //4.25
        IgnoredProperties.Add(MP_AmbientOcclusion);
        IgnoredProperties.Add(MP_PixelDepthOffset);
        IgnoredProperties.Add(MP_ShadingModel);
        IgnoredProperties.Add(MP_MaterialAttributes);
        IgnoredProperties.Add(MP_CustomOutput);

        MaterialTranslator.GetSharedInputsMaterialCode(IgnoredProperties, InputCodes, NormalCode, NormalAssignment, VertexMembersSetupAndAssignments, PixelMembersSetupAndAssignments);
        MaterialTranslator.ExtractConstantProperties(IgnoredProperties, TranslatorResult.ConstantProperties);
        LocalVariables = MaterialTranslator.LocalVariables;
        InputVariables = MaterialTranslator.DefinedParameterNames;

        if (ExportTextureCallback)
        {
            for(auto Texture : *MaterialTranslator.CurrentTextureCache)
            {
                ExportTextureCallback(Texture.Key, Texture.Value, ExporterSetting.Name);
            }
        }

        AssembleLetExpressions();
        RemoveUnusedVariables(TranslatorResult);
    }

    // Collect errors and warnings
    {
        FString OuterPackageName;
        if (!FPackageName::TryConvertFilenameToLongPackageName(OriginalMaterial->GetPathName(), OuterPackageName))
        {
            OuterPackageName = OriginalMaterial->GetPathName();
        }

        for (auto Error : MaterialTranslator.TranslatedErrors)
        {
            TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Error, FText());
            Message->AddToken(FAssetNameToken::Create(OuterPackageName));
            Message->AddToken(FTextToken::Create(FText::FromString(TEXT(":"))));
            Message->AddToken(FTextToken::Create(FText::FromString(Error)));
            TranslatorResult.Messages.Add(Message);
        }

        for (auto Warning : MaterialTranslator.TranslatedWarnings)
        {
            TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Warning, FText());
            Message->AddToken(FAssetNameToken::Create(OuterPackageName));
            Message->AddToken(FTextToken::Create(FText::FromString(TEXT(":"))));
            Message->AddToken(FTextToken::Create(FText::FromString(Warning)));
            TranslatorResult.Messages.Add(Message);
        }
    }

    if (!ExporterSetting.bIgnoreCache)
    {
        UMaterial* BaseMaterial = OriginalMaterial->GetMaterial();
        TArray<FMDLTranslatorResult>& TranslatorResults = FMDLExporterUtility::GlobalExportedMaterials.FindOrAdd(BaseMaterial);
        TranslatorResults.Add(TranslatorResult);
    }
    
    return bSuccess;
}

void FMDLTemplateExporter::ExportHeader()
{
    ExportedMDLString += TEXT("mdl ") + MDLVersion + TEXT(";") + LINE_TERMINATOR + LINE_TERMINATOR;
}

void FMDLTemplateExporter::ExportImports(bool Extension17)
{
    // math
    ExportedMDLString += TEXT("import ::math::*;");
    ExportedMDLString += LINE_TERMINATOR;
    ExportedMDLString += TEXT("import ::state::*;");
    ExportedMDLString += LINE_TERMINATOR;
    ExportedMDLString += TEXT("import ::tex::*;");
    ExportedMDLString += LINE_TERMINATOR;
    ExportedMDLString += TEXT("import ::anno::*;");
    ExportedMDLString += LINE_TERMINATOR;
    ExportedMDLString += TEXT("import ::scene::*;");
    ExportedMDLString += LINE_TERMINATOR;

    // ue4 function
    ExportedMDLString += TEXT("using ::") + UE4Templates[FMDLExporterUtility::MDL_Function].ToString() + TEXT(" import *;") + LINE_TERMINATOR;

    if (Extension17)
    {
        ExportedMDLString += TEXT("using ::") + UE4Templates[FMDLExporterUtility::MDL_FunctionExtension17].ToString() + TEXT(" import *;") + LINE_TERMINATOR;
    }

    // template
    ExportedMDLString += TEXT("using ::") + UsedTemplate + TEXT(" import *;") + LINE_TERMINATOR + LINE_TERMINATOR;
}

void FMDLTemplateExporter::ExportAnnotationsDefine()
{
    for(int32 AnnoIndex = 0; AnnoIndex < MDLSamplerType::ST_MAX; ++AnnoIndex )
    {
        ExportedMDLString += TEXT("export annotation ") + FString(SamplerAnnotations[AnnoIndex]) + TEXT(";") + LINE_TERMINATOR;
    }

    for(int32 AnnoIndex = 0; AnnoIndex < MDLUE4Property::UE4_MAX; ++AnnoIndex )
    {
        if (AnnoIndex != UE4_DistillOff)
        {
            ExportedMDLString += TEXT("export annotation ") + FString(UE4Annotations[AnnoIndex]) + TEXT(";") + LINE_TERMINATOR;
        }
    }

    ExportedMDLString += LINE_TERMINATOR;
}

void FMDLTemplateExporter::ExportCustomExpressions()
{
    ExportedMDLString += CustomExpressions;
}

void FMDLTemplateExporter::ExportAnnotation()
{
    FString Annotation;

#define ANNOTATION_DEFINITION(type) \
    if (!Annotation.IsEmpty()) { Annotation += TEXT(","); Annotation += LINE_TERMINATOR; } \
    Annotation += TEXT("\t") + FString(UE4Annotations[type]);

    const FMaterialResource* MaterialResource = OriginalMaterial->GetMaterialResource(GMaxRHIFeatureLevel);
    check(MaterialResource);
    FMaterialShadingModelField MaterialShadingModels = OriginalMaterial->GetShadingModels();

    if(!MaterialResource->IsTangentSpaceNormal())
    {
        ANNOTATION_DEFINITION(UE4_WorldSpaceNormal);
    }

    if (OriginalMaterial->GetBlendMode() == BLEND_Masked)
    {
        if(!MaterialResource->IsDitherMasked())
        {
            ANNOTATION_DEFINITION(UE4_DitherMaskedOff);
        }
    }
    if (/*IsTranslucentBlendMode(OriginalMaterial->GetBlendMode()) ||*/ IsSubsurfaceColorUsed(MaterialShadingModels.GetFirstShadingModel()))
    {
        ANNOTATION_DEFINITION(UE4_DistillOff);
    }
#undef ANNOTATION_DEFINITION

    if (!Annotation.IsEmpty())
    {
        ExportedMDLString += TEXT("[[");
        ExportedMDLString += LINE_TERMINATOR;
        ExportedMDLString += Annotation;
        ExportedMDLString += LINE_TERMINATOR;
        ExportedMDLString += TEXT("]]");
        ExportedMDLString += LINE_TERMINATOR;
    }

}

void FMDLTemplateExporter::RemoveUnusedVariables(FMDLTranslatorResult& TranslatorResult)
{
    auto IsNameAscii = [](const TCHAR Char)
    {
        if ((Char >= 0x30 && Char <= 0x39) // 0-9
        || (Char >= 0x41 && Char <= 0x5A) // A-Z
        || (Char >= 0x61 && Char <= 0x7A) // a-z
        || Char == 0x5F // _
            )
        {
            return true;
        }

        return false;
    };

    auto SearchVariable = [&](const FString& Variable, int32 StopSearchCount, int32& Start)
    {
        int32 UsedCount = 0;
        int32 StartIndex = INDEX_NONE;
        do
        {
            StartIndex = LetExpressions.Find(Variable, ESearchCase::CaseSensitive, ESearchDir::FromStart, StartIndex);
            if (StartIndex != INDEX_NONE)
            {
                bool PrevOK = false;
                // check previous
                if (StartIndex != 0)
                {
                    if (!IsNameAscii(LetExpressions[StartIndex - 1]))
                    {
                        PrevOK = true;
                    }
                }
                else
                {
                    PrevOK = true;
                }

                int32 EndIndex = StartIndex + Variable.Len();
                if (PrevOK)
                {
                    if (!IsNameAscii(LetExpressions[EndIndex]))
                    {
                        Start = StartIndex;
                        ++UsedCount;
                        if (UsedCount >= StopSearchCount)
                        {
                            break;
                        }
                    }	
                }

                StartIndex = EndIndex;
            }
        } while (StartIndex != INDEX_NONE);

        return UsedCount;
    };

    auto SearchLineEnd = [&](int32 Start, bool bReversed)
    {
        while(Start >= 0 && Start <= LetExpressions.Len() && LetExpressions[Start] != '\n')
        {
            if (bReversed)
            {
                --Start;
            }
            else
            {
                ++Start;
            }
        }

        return Start;
    };

    // Step 1. Search unused local variable
    for (int32 Index = LocalVariables.Num(); Index > 0; --Index)
    {
        int32 IndexInArray = Index - 1;
        int32 VariablePosition = INDEX_NONE;
        int32 UsedCount = SearchVariable(LocalVariables[IndexInArray], 2/*definition and using*/, VariablePosition);
        if (UsedCount == 1) // Only found Definition or nothing
        {
            // Remove the line of the local variable
            check(VariablePosition != INDEX_NONE);
            int32 LineEnd = SearchLineEnd(VariablePosition, false);
            int32 LineStart = SearchLineEnd(VariablePosition, true);
            LetExpressions.RemoveAt(LineStart, LineEnd - LineStart);
        }
    }
    
    check(InputVariables.Num() == InputCodes.Num());
    // Step 2. Search unused input variable
    for (int32 Index = InputVariables.Num(); Index > 0; --Index)
    {
        int32 IndexInArray = Index - 1;
        int32 VariablePosition = INDEX_NONE;
        int32 UsedCount = SearchVariable(InputVariables[IndexInArray], 1/*only search using*/, VariablePosition);
        if (UsedCount < 1) // Nothing found
        {
            // Remove the line in input codes
            if (IndexInArray == 0 && InputCodes.Num() > 1)
            {
                // Remove ",\r\n" at beginning of the next line
                InputCodes[1].RemoveFromStart(TEXT(",\r\n"));
            }
            InputCodes.RemoveAt(IndexInArray);

            // Remove the unused parameters from mdl schema
            auto NameKey = TranslatorResult.ScalarParameters.FindKey(InputVariables[IndexInArray]);
            if (NameKey)
            {
                TranslatorResult.ScalarParameters.Remove(*NameKey);
            }
            else
            {
                NameKey = TranslatorResult.VectorParameters.FindKey(InputVariables[IndexInArray]);

                if (NameKey)
                {
                    TranslatorResult.VectorParameters.Remove(*NameKey);
                }
                else
                {
                    NameKey = TranslatorResult.TextureParameters.FindKey(InputVariables[IndexInArray]);

                    if (NameKey)
                    {
                        TranslatorResult.TextureParameters.Remove(*NameKey);
                    }
                }
            }

            // clear flags from Usage bits
            if (InputVariables[IndexInArray].Compare(TEXT("VertexColorCoordinateIndex")) == 0)
            {
                TranslatorResult.ClearUsageFlag(MaterialUsage_VertexColor);
            }
            else if (InputVariables[IndexInArray].Compare(TEXT("NumberInstances")) == 0)
            {
                TranslatorResult.ClearUsageFlag(MaterialUsage_PerInstanceData);
            }
            else if (InputVariables[IndexInArray].Compare(TEXT("AOMaterialMaskTexture")) == 0)
            {
                TranslatorResult.ClearUsageFlag(MaterialUsage_AOMaterialMaskTexture);
            }
            else if (InputVariables[IndexInArray].Compare(TEXT("LightMapCoordinateIndex")) == 0)
            {
                TranslatorResult.ClearUsageFlag(MaterialUsage_LightMapCoordinate);
            }
            else if (InputVariables[IndexInArray].Compare(TEXT("LightmapCoordinateScale")) == 0)
            {
                TranslatorResult.ClearUsageFlag(MaterialUsage_LightMapCoordinate);
            }
            else if (InputVariables[IndexInArray].Compare(TEXT("LightmapCoordinateBias")) == 0)
            {
                TranslatorResult.ClearUsageFlag(MaterialUsage_LightMapCoordinate);
            }
            else if (InputVariables[IndexInArray].Compare(TEXT("TwoSidedSign")) == 0)
            {
                TranslatorResult.ClearUsageFlag(MaterialUsage_TwoSidedSign);
            }
            else if (InputVariables[IndexInArray].Compare(TEXT("MaxTexCoordIndex")) == 0)
            {
                TranslatorResult.ClearUsageFlag(MaterialUsage_MaxTexCoordinate);
            }
        }
    }
}

void FMDLTemplateExporter::AssembleLetExpressions()
{
    LetExpressions.Empty();
    LetExpressions += TEXT("\tlet {");
    LetExpressions += LINE_TERMINATOR;

    LetExpressions += CustomVSOutputCode;
    LetExpressions += VertexMembersSetupAndAssignments + LINE_TERMINATOR;
    LetExpressions += NormalCode + LINE_TERMINATOR;
    LetExpressions += NormalAssignment + LINE_TERMINATOR;
    LetExpressions += PixelMembersSetupAndAssignments + LINE_TERMINATOR;
    LetExpressions += CustomPSOutputCode;

    LetExpressions += TEXT("\t} in");
    LetExpressions += LINE_TERMINATOR;
}

void FMDLTemplateExporter::ExportLetBlock()
{
    ExportedMDLString += LetExpressions;
}

void FMDLTemplateExporter::ExportModuleBlock()
{
    FMaterialShadingModelField MaterialShadingModels = OriginalMaterial->GetShadingModels(); 
    FMaterialResource* MaterialResource = OriginalMaterial->GetMaterialResource(GMaxRHIFeatureLevel);
    check(MaterialResource);
    ExportedMDLString += TEXT("\t\t::") + UsedTemplate + TEXT("(") + LINE_TERMINATOR;

#define VARIABLE_DEFINITION(name, ue4name) \
    ExportedMDLString += TEXT("\t\t\t"); \
    ExportedMDLString += name; \
    ExportedMDLString += TEXT(": "); \
    ExportedMDLString += ue4name;

#define RETURN \
    ExportedMDLString += TEXT(","); \
    ExportedMDLString += LINE_TERMINATOR;
    bool bRayTracingTranslucency = IsTranslucentBlendMode(OriginalMaterial->GetBlendMode()) && ExporterSetting.bRayTracingTranslucency;
    if (MaterialShadingModels.GetFirstShadingModel() != MSM_Unlit)
    {
        VARIABLE_DEFINITION(TEXT("base_color"), TEXT("BaseColor_mdl"));
        RETURN;
        // The Metallic input channel is not available when using eye model 
        // because the GBuffer space for the Metallic input has been repurposed to accommodate the Subsurface Profile data.
        if (MaterialShadingModels.GetFirstShadingModel() != MSM_Eye)
        {
            VARIABLE_DEFINITION(TEXT("metallic"), TEXT("Metallic_mdl"));
            RETURN;
        }
        VARIABLE_DEFINITION(TEXT("roughness"), TEXT("Roughness_mdl"));
        RETURN;
        if (!bRayTracingTranslucency)
        {
            VARIABLE_DEFINITION(TEXT("specular"), TEXT("Specular_mdl"));
            RETURN;
        }
        VARIABLE_DEFINITION(TEXT("normal"), TEXT("Normal_mdl"));
        RETURN;
        if (MaterialShadingModels.GetFirstShadingModel() == MSM_ClearCoat)
        {
            VARIABLE_DEFINITION(TEXT("clearcoat_weight"), TEXT("ClearCoat_mdl"));
            RETURN;
            VARIABLE_DEFINITION(TEXT("clearcoat_roughness"), TEXT("ClearCoatRoughness_mdl"));
            RETURN;
            // ClearCoat normal isn't always there.
            if (!CustomPSOutputCode.IsEmpty() && CustomPSOutputCode.Contains(TEXT("ClearCoatBottomNormal0_mdl")))
            {
                VARIABLE_DEFINITION(TEXT("clearcoat_normal"), TEXT("ClearCoatBottomNormal0_mdl"));
                RETURN;
            }
        }
    }

    if (IsTranslucentBlendMode(OriginalMaterial->GetBlendMode()) || IsSubsurfaceColorUsed(MaterialShadingModels.GetFirstShadingModel()))
    {
        // UE4.26 Domain : SubsurfaceColor = UE4.25 Domain : Opacity
        if (MaterialResource->GetMaterialDomain() == MD_Volume)
        {
            VARIABLE_DEFINITION(TEXT("opacity"), TEXT("SubsurfaceColor_mdl.x"));
        }
        else
        {
        VARIABLE_DEFINITION(TEXT("opacity"), TEXT("Opacity_mdl"));
    }
    }
    else
    {
        VARIABLE_DEFINITION(TEXT("opacity"), TEXT("OpacityMask_mdl"));
    }
    RETURN;

    if (IsTranslucentBlendMode(OriginalMaterial->GetBlendMode()))
    {
        if (OriginalMaterial->GetCastDynamicShadowAsMasked())
        {
            VARIABLE_DEFINITION(TEXT("opacity_mask"), TEXT("OpacityMask_mdl"));
            RETURN;
        }
        else if (OriginalMaterial->GetBlendMode() == BLEND_AlphaComposite
            || OriginalMaterial->GetBlendMode() == BLEND_AlphaHoldout
            || OriginalMaterial->GetBlendMode() == BLEND_Translucent)
        {
            VARIABLE_DEFINITION(TEXT("opacity_mask"), TEXT("OpacityMask_mdl"));
            RETURN;
        }
    }

    VARIABLE_DEFINITION(TEXT("emissive_color"), TEXT("EmissiveColor_mdl"));
    RETURN;

#if 0 // Turn off displacement
    VARIABLE_DEFINITION(TEXT("displacement"), TEXT("WorldPositionOffset_mdl"));
    RETURN;
#endif

    if (IsSubsurfaceColorUsed(MaterialShadingModels.GetFirstShadingModel()) && !IsTranslucentBlendMode(OriginalMaterial->GetBlendMode()))
    {
        VARIABLE_DEFINITION(TEXT("opacity_mask"), TEXT("OpacityMask_mdl"));
        RETURN;
        VARIABLE_DEFINITION(TEXT("subsurface_color"), TEXT("SubsurfaceColor_mdl"));
        RETURN;
    }
    if (IsTranslucentBlendMode(OriginalMaterial->GetBlendMode()))
    {
        if (bRayTracingTranslucency)
        {
            VARIABLE_DEFINITION(TEXT("refraction"), TEXT("Specular_mdl"));
        }
        else
        {
            VARIABLE_DEFINITION(TEXT("refraction"), TEXT("Refraction_mdl.x"));
        }
        RETURN;
    }

    // NOTE: thin_walled isn't working for translucent in Kit. Make it always off for now.
    if (IsTranslucentBlendMode(OriginalMaterial->GetBlendMode()))
    {
        VARIABLE_DEFINITION(TEXT("two_sided"), TEXT("false"));
    }
    else
    {
        VARIABLE_DEFINITION(TEXT("two_sided"), OriginalMaterial->IsTwoSided() ? TEXT("true") : TEXT("false"));
    }

    if (!MaterialResource->IsTangentSpaceNormal())
    {
        RETURN;
        VARIABLE_DEFINITION(TEXT("is_tangent_space_normal"), TEXT("false"));
    }

    if (MaterialShadingModels.GetFirstShadingModel() == MSM_Unlit)
    {
        RETURN;
        VARIABLE_DEFINITION(TEXT("is_unlit"), TEXT("true"));
    }

#undef RETURN
#undef VARIABLE_DEFINITION

    ExportedMDLString += TEXT(");");
    ExportedMDLString += LINE_TERMINATOR;
}

void FMDLTemplateExporter::ExportMaterial()
{
    ExportedMDLString += TEXT("export material ");
    ExportedMDLString += MDLMagicName;
    ExportedMDLString += TEXT("(");
    ExportedMDLString += LINE_TERMINATOR;
    ExportParamters();
    ExportedMDLString += TEXT(")");
    ExportedMDLString += LINE_TERMINATOR;
    ExportAnnotation();

    ExportedMDLString += TEXT(" = ");
    ExportedMDLString += LINE_TERMINATOR;
    ExportLetBlock();
    ExportModuleBlock();
}

void FMDLTemplateExporter::ExportParamters()
{
    for (auto Input : InputCodes)
    {
        ExportedMDLString += Input;
    }
}

bool FMDLTemplateExporter::ExportModule(FString& Output, FMDLTranslatorResult* OutResult)
{
    bool bSuccess = false;

    if (!OriginalMaterial)
    {
        return bSuccess;
    }

    ExportedMDLString.Empty();
    if (OutResult)
    {
        OutResult->Messages.Reset();
    }

    FMaterialShadingModelField MaterialShadingModels = OriginalMaterial->GetShadingModels(); 
    UsedTemplate = (IsTranslucentBlendMode(OriginalMaterial->GetBlendMode()) ? UE4Templates[FMDLExporterUtility::MDL_Translucent] :
        IsSubsurfaceColorUsed(MaterialShadingModels.GetFirstShadingModel()) ? UE4Templates[FMDLExporterUtility::MDL_Subsurface] : UE4Templates[FMDLExporterUtility::MDL_Base]).ToString();

    FMDLTranslatorResult TranslatorResult;
    FMemory::Memzero(TranslatorResult);
    if (Translate(TranslatorResult))
    {
        if (OutResult)
        {
            *OutResult = TranslatorResult;
        }
        ExportHeader();
        ExportImports(TranslatorResult.IsExtension17Used());
        ExportAnnotationsDefine();
        ExportCustomExpressions();
        ExportMaterial();

        if (!ExporterSetting.bIgnoreCache && OutResult)
        {
            FSHA1 HashState;
            FSHAHash OutHash;
            HashState.UpdateWithString(*ExportedMDLString, ExportedMDLString.Len());

            HashState.Final();
            HashState.GetHash(&OutHash.Hash[0]);

            FString MDLHash = OutHash.ToString();
            // check hash of mdl string
            auto FoundResult = FMDLExporterUtility::GlobalCachedMDL.Find(MDLHash);
            if (FoundResult)
            {
                // update the cache for material
                UpdateExportedMaterial(*FoundResult);
                *OutResult = *FoundResult;
                return bSuccess;
            }
            FMDLExporterUtility::GlobalCachedMDL.Add(MDLHash, *OutResult);
        }

        bSuccess = true;
    }
    else
    {
        if (OutResult)
        {
            *OutResult = TranslatorResult;
        }
    }

    ExportedMDLString.ReplaceInline(*MDLMagicName, *ExporterSetting.Name);
    Output = ExportedMDLString;

    return bSuccess;
}

