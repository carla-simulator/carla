// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "CoreMinimal.h"
#include "SimReadyUSDTokens.h"
#include "USDConverter.h"
#include "SimReadySettings.h"
#include "SimReadyStageActor.h"
#include "SimReadyTexture.h"
#include "SimReadyPathHelper.h"
#include "SimReadyAssetExportHelper.h"
#include "Components/MeshComponent.h"
#include "Containers/ArrayBuilder.h"
#include "Materials/MaterialInstanceConstant.h"
#include "IMaterialEditor.h"
#include "MaterialEditor/MaterialEditorInstanceConstant.h"
#include "MaterialEditorInstanceNotifier.h"
#include "USDHashGenerator.h"
#include "USDGeometryCache.h"
#include "SimReadyAssetImportHelper.h"
#include "SimReadyUSDImporterHelper.h"
#include "ISimReadyRuntimeModule.h"
#include "SimReadyMaterialReparentUtility.h"


void UpdateStaticParameters(class UMaterialInstanceConstant* MaterialInst, const TMap<FName, bool>* SwitchParameters, const TMap<FName, FStaticComponentMask>* ComponentMaskParameters)
{
    if (!SwitchParameters && !ComponentMaskParameters)
    {
        return;
    }

    FStaticParameterSet OutStaticParameters;
    MaterialInst->GetStaticParameterValues(OutStaticParameters);

    bool bUpdateStatic = false;

    if (SwitchParameters)
    {
        for (auto& StaticSwitchParameter : OutStaticParameters.StaticSwitchParameters)
        {
            for (const TPair<FName, bool>& Pair : *SwitchParameters)
            {
                if (Pair.Key == StaticSwitchParameter.ParameterInfo.Name)
                {
                    if (StaticSwitchParameter.Value != Pair.Value)
                    {
                        StaticSwitchParameter.Value = Pair.Value;
                        StaticSwitchParameter.bOverride = Pair.Value;
                        bUpdateStatic = true;
                    }
                }
            }
        }
    }

    if (ComponentMaskParameters)
    {
        for (auto& StaticComponentMaskParameter : OutStaticParameters.StaticComponentMaskParameters)
        {
            for (const TPair<FName, FStaticComponentMask>& Pair : *ComponentMaskParameters)
            {
                if (Pair.Key == StaticComponentMaskParameter.ParameterInfo.Name)
                {
                    if (StaticComponentMaskParameter.R != Pair.Value.R
                    || StaticComponentMaskParameter.G != Pair.Value.G
                    || StaticComponentMaskParameter.B != Pair.Value.B
                    || StaticComponentMaskParameter.A != Pair.Value.A)
                    {
                        StaticComponentMaskParameter.R = Pair.Value.R;
                        StaticComponentMaskParameter.G = Pair.Value.G;
                        StaticComponentMaskParameter.B = Pair.Value.B;
                        StaticComponentMaskParameter.A = Pair.Value.A;
                        StaticComponentMaskParameter.bOverride = true;
                        bUpdateStatic = true;
                    }				
                }
            }
        }
    }

    if (bUpdateStatic)
    {
        FlushRenderingCommands();
        MaterialInst->UpdateStaticPermutation(OutStaticParameters);
    }
}

bool ASimReadyStageActor::IsPreviewSurface(UMaterialInstanceConstant* MaterialInst)
{
    UMaterial* PreviewSurfaceMaterial = LoadObject<UMaterial>(nullptr, *USimReadyMDL::GetContentPath(TEXT("/SimReady/PreviewSurfaceMaterial")));

    return (MaterialInst->Parent && PreviewSurfaceMaterial == MaterialInst->Parent);
}

bool ASimReadyStageActor::LoadPreviewSurface(UMeshComponent* Component, int32 SlotIndex, const pxr::UsdGeomMesh& USDMesh, const pxr::UsdShadeShader& ShadeShader)
{
    if (!ShadeShader)
    {
        return false;
    }

    FString ShaderPath = ShadeShader.GetPath().GetText();

    auto Object = USDPathToObject.Find(FSimReadyPathHelper::PrimPathToKey(ShaderPath));
    if (Object && Object->IsValid())
    {
        if (Component)
        {
            SetMaterial(*Component, SlotIndex, Cast<UMaterialInstanceConstant>(Object->Get()));
        }
        return true;
    }

    auto USDMaterial = FindShadeMaterial(ShadeShader.GetPrim());
    if (!USDMaterial)
    {
        return false;
    }

    FString MaterialName = USDMaterial.GetPrim().GetName().GetText();
    FString Key;
    if (HasValidImportStage())
    {
        Key = FUSDHashGenerator::ComputeSHAHash(USDMaterial).ToString();
    }
    UMaterialInstanceConstant* MaterialInst = nullptr;
    MaterialName = GetUniqueImportName(Key, MaterialName);
    if (HasValidImportStage())
    {
        MaterialInst = Cast<UMaterialInstanceConstant>(FUSDGeometryCache::Find(Key));
    }
    if (MaterialInst == nullptr)
    {
        MaterialInst = LoadImportObject<UMaterialInstanceConstant>(ImportType::Material, MaterialName);	
        if (MaterialInst == nullptr)
        {
            MaterialInst = NewObject<UMaterialInstanceConstant>(GetAssetPackage(ImportType::Material, MaterialName), GetAssetName(MaterialName), GetAssetFlag());
            if (!MaterialInst)
            {
                return false;
            }

            UMaterial* PreviewSurfaceMaterial = LoadObject<UMaterial>(nullptr, *USimReadyMDL::GetContentPath(TEXT("/SimReady/PreviewSurfaceMaterial")));
            MaterialInst->SetParentEditorOnly(PreviewSurfaceMaterial);

            // collect UVSets map for the current geometry mesh.
            TMap<FString, int32> UVSets;
            if (USDMesh)
            {
                int32 UVCount = 0;
                auto Primvars = USDMesh.GetPrimvars();
                for (auto Iter = Primvars.begin(); Iter != Primvars.end(); ++Iter)
                {
                    auto PrimVar = *Iter;

                    // Texture UV
                    if (PrimVar.GetTypeName().GetCPPTypeName().compare(pxr::SdfValueTypeNames->Float2Array.GetCPPTypeName()) == 0)
                    {
                        const pxr::UsdAttribute& Attr = PrimVar.GetAttr();
                        pxr::VtValue CustomData = Attr.GetCustomDataByKey(pxr::TfToken("Maya"));

                        int32 UVIndex = -1;
                        if (!CustomData.IsEmpty() && CustomData.IsHolding<pxr::VtDictionary>()) {
                            pxr::VtDictionary NewDict = CustomData.UncheckedGet<pxr::VtDictionary>();
                            pxr::VtDictionary::iterator Itor = NewDict.find("UVSetIndex");
                            if (Itor != NewDict.end())
                            {
                                UVIndex = Itor->second.Get<int32>();
                            }
                        }

                        // If can't find custom index, still use the output index.
                        if (UVIndex < 0)
                        {
                            UVIndex = UVCount++;
                        }

                        UVSets.Add(PrimVar.GetBaseName().GetText(), UVIndex);
                    }
                }
            }

            FSimReadyUSDImporterHelper::UpdatePreviewSurfaceInputs(
                MaterialInst,
                ShadeShader,
                &UVSets,
                [&](const FString& PrimName)
                {
                    USDPathToObject.FindOrAdd(FSimReadyPathHelper::PrimPathToKey(PrimName), MaterialInst);
                },
                [&](const uint8* Content, uint64 Size, const FString& FileName, UTexture*& OutTexture)
                {
                    CreateTextureFromBuffer(Content, Size, FileName, OutTexture);
                }
            );

            MaterialInst->PostEditChange();
            MaterialInst->MarkPackageDirty();
            if (HasValidImportStage())
            {
                FAssetRegistryModule::AssetCreated(MaterialInst);
            }
        }

        if (HasValidImportStage())
        {
            FUSDGeometryCache::Add(Key, MaterialInst);
        }
    }

    if (MaterialInst)
    {
        USDPathToObject.FindOrAdd(FSimReadyPathHelper::PrimPathToKey(ShaderPath), MaterialInst);
        if (Component)
        {
            SetMaterial(*Component, SlotIndex, MaterialInst);
        }
    }

    return true;
}

void FUSDExporter::ExportPreviewSurface(const pxr::UsdStageRefPtr& Stage, UMaterialInstance& MaterialInst, pxr::UsdShadeShader& ShadeShader)
{
    if (!ShadeShader)
    {
        return;
    }

    const TArray<FString> PSInputNames = TArrayBuilder<FString>()
        .Add(FString(TEXT("diffuseColor")))
        .Add(FString(TEXT("emissiveColor")))
        .Add(FString(TEXT("useSpecularWorkflow")))
        .Add(FString(TEXT("specularColor")))
        .Add(FString(TEXT("metallic")))
        .Add(FString(TEXT("roughness")))
        .Add(FString(TEXT("clearcoat")))
        .Add(FString(TEXT("clearcoatRoughness")))
        .Add(FString(TEXT("opacity")))
        .Add(FString(TEXT("opacityThreshold")))
        .Add(FString(TEXT("ior")))
        .Add(FString(TEXT("normal")))
        .Add(FString(TEXT("displacement")))
        .Add(FString(TEXT("occlusion")));

    const TArray<pxr::SdfValueTypeName> PSInputTypes = TArrayBuilder<pxr::SdfValueTypeName>()
        .Add(pxr::SdfValueTypeNames->Color3f) //diffuseColor
        .Add(pxr::SdfValueTypeNames->Color3f) //emissiveColor
        .Add(pxr::SdfValueTypeNames->Int) //useSpecularWorkflow
        .Add(pxr::SdfValueTypeNames->Color3f) //specularColor
        .Add(pxr::SdfValueTypeNames->Float) //metallic
        .Add(pxr::SdfValueTypeNames->Float) //roughness
        .Add(pxr::SdfValueTypeNames->Float) //clearcoat
        .Add(pxr::SdfValueTypeNames->Float) //clearcoatRoughness
        .Add(pxr::SdfValueTypeNames->Float) //opacity
        .Add(pxr::SdfValueTypeNames->Float) //opacityThreshold
        .Add(pxr::SdfValueTypeNames->Float) //ior
        .Add(pxr::SdfValueTypeNames->Normal3f) //normal
        .Add(pxr::SdfValueTypeNames->Float) //displacement
        .Add(pxr::SdfValueTypeNames->Float); //occlusion

    const auto ShaderID = GetUSDValue<pxr::TfToken>(ShadeShader.GetIdAttr());
    if (ShaderID == USDTokens.previewSurface)
    {
        FString MaterialName;
        int32 MaterialReparentIndex = FSimReadyMaterialReparentUtility::FindOmniMaterial(MaterialInst.Parent, MaterialName, ESimReadyReparentUsage::Export);

        for (int32 InputIndex = 0; InputIndex < PSInputNames.Num(); ++InputIndex)
        {
            auto InputName = PSInputNames[InputIndex];
            FString UnrealParam = InputName;
            FString TextureEnableParam = InputName + TEXT("TextureEnable");
            if (MaterialReparentIndex != INDEX_NONE)
            {
                FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(UnrealParam), UnrealParam);
                FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(TextureEnableParam), TextureEnableParam);
            }

            float TextureEnabled = 0.0f;
            if ((FSimReadyMaterialReparentUtility::FindScalarConstant(MaterialReparentIndex, TextureEnableParam, TextureEnabled) || MaterialInst.GetScalarParameterValue(*TextureEnableParam, TextureEnabled)) && TextureEnabled == 0.0f)
            {
                if (TextureEnabled == 0.0f)
                {
                    auto Input = ShadeShader.GetInput(pxr::TfToken(TCHAR_TO_ANSI(*InputName)));
                    if (Input && Input.HasConnectedSource())
                    {
                        Input.DisconnectSource();
                    }

                    float ScalarValue = 0.0f;
                    FLinearColor VectorValue;
                    if (MaterialInst.GetScalarParameterValue(*UnrealParam, ScalarValue))
                    {
                        FSimReadyMaterialReparentUtility::FindScalarConstant(MaterialReparentIndex, UnrealParam, ScalarValue);
                        ExportParameter<float>(PSInputTypes[InputIndex], InputName, ShadeShader, ScalarValue);
                    }
                    else if (MaterialInst.GetVectorParameterValue(*UnrealParam, VectorValue))
                    {
                        FSimReadyMaterialReparentUtility::FindVectorConstant(MaterialReparentIndex, UnrealParam, VectorValue);
                        ExportParameter<pxr::GfVec4f>(PSInputTypes[InputIndex], InputName, ShadeShader, LinearColorToVec(VectorValue));
                    }
                }
                else
                {
                    UTexture* Texture = nullptr;
                    FString TextureParam = InputName + TEXT("Texture");
                    if (MaterialReparentIndex != INDEX_NONE)
                    {
                        FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(TextureParam), TextureParam);
                    }
                    if ((FSimReadyMaterialReparentUtility::FindTextureConstant(MaterialReparentIndex, TextureParam, Texture) || MaterialInst.GetTextureParameterValue(*TextureParam, Texture)) && Texture)
                    {
                        FString TexturePath;
                        // Do not upload the texture from mdl and simready content, they meant empty texture for mdl
                        if (Texture->GetOutermost() != GetTransientPackage() && !Texture->GetOutermost()->GetName().StartsWith(TEXT("/MDL/")) && !Texture->GetOutermost()->GetName().StartsWith(TEXT("/SimReady/")))
                        {
                            // Need uploading UE4 texture to server
                            FString TextureFileName = FSimReadyAssetExportHelper::GetTextureFileName(Texture);
                            auto LayerPath = (*Stage).GetEditTarget().GetLayer()->GetRealPath().c_str();
                            FString OmniPath = FPaths::GetPath(LayerPath) / TextureFileName;
                            TexturePath = TEXT("./") + TextureFileName;
                            FSimReadyAssetExportHelper::ExportTextureToPath(Texture, OmniPath);
                        }

                        auto PropertyInput = ShadeShader.GetInput(pxr::TfToken(TCHAR_TO_ANSI(*InputName)));
                        if (!PropertyInput)
                        {
                            PropertyInput = ShadeShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*InputName)), PSInputTypes[InputIndex]);
                        }

                        pxr::UsdShadeShader TextureShader;
                        if (PropertyInput && PropertyInput.HasConnectedSource())
                        {
                            pxr::UsdShadeConnectableAPI Source;
                            pxr::TfToken SourceName;
                            pxr::UsdShadeAttributeType SourceType;
                            PropertyInput.GetConnectedSource(&Source, &SourceName, &SourceType);
                            TextureShader = pxr::UsdShadeShader(Source);
                        }
                        else // Can't find texture shader, create one
                        {
                            auto TextureShaderPath = ShadeShader.GetPath().GetParentPath().AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*(InputName + TEXT("Tex")))));
                            TextureShader = pxr::UsdShadeShader::Define(Stage, TextureShaderPath);
                        }

                        if (TextureShader)
                        {
                            FString TextureScaleParam = UnrealParam + TEXT("TextureScale");
                            FString TextureBiasParam = UnrealParam + TEXT("TextureBias");
                            FString TextureUVParam = UnrealParam + TEXT("UV");
                            FString MaskParam = UnrealParam + TEXT("Mask");

                            if (MaterialReparentIndex != INDEX_NONE)
                            {
                                FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(TextureScaleParam), TextureScaleParam);
                                FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(TextureBiasParam), TextureBiasParam);
                                FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(TextureUVParam), TextureUVParam);
                                FSimReadyMaterialReparentUtility::FindUnrealParameter(MaterialReparentIndex, MoveTemp(MaskParam), MaskParam);
                            }

                            // ID
                            TextureShader.SetShaderId(USDTokens.uvTexture);

                            // File
                            auto FileInput = TextureShader.GetInput(USDTokens.file);
                            if (!FileInput)
                            {
                                FileInput = TextureShader.CreateInput(USDTokens.file, pxr::SdfValueTypeNames->Asset);
                            }
                            FileInput.Set(pxr::SdfAssetPath(TCHAR_TO_UTF8(*TexturePath)));

                            // ColorSpace
                            auto ColorSpaceInput = TextureShader.GetInput(USDTokens.sourceColorSpace);
                            if (!ColorSpaceInput)
                            {
                                ColorSpaceInput = TextureShader.CreateInput(USDTokens.sourceColorSpace, pxr::SdfValueTypeNames->Token);
                            }
                            ColorSpaceInput.Set(Texture->SRGB ? USDTokens.sRGB : USDTokens.raw);

                            // Scale
                            FLinearColor TextureScale;
                            if (FSimReadyMaterialReparentUtility::FindVectorConstant(MaterialReparentIndex, TextureScaleParam, TextureScale) || MaterialInst.GetVectorParameterValue(*TextureScaleParam, TextureScale))
                            {
                                auto ScaleInput = TextureShader.GetInput(pxr::TfToken("scale"));
                                if (!ScaleInput)
                                {
                                    ScaleInput = TextureShader.CreateInput(pxr::TfToken("scale"), pxr::SdfValueTypeNames->Float4);
                                }
                                ScaleInput.Set(LinearColorToVec(TextureScale));
                            }

                            // Bias
                            FLinearColor TextureBias;
                            if (FSimReadyMaterialReparentUtility::FindVectorConstant(MaterialReparentIndex, TextureBiasParam, TextureBias) || MaterialInst.GetVectorParameterValue(*TextureBiasParam, TextureBias))
                            {
                                auto BiasInput = TextureShader.GetInput(pxr::TfToken("bias"));
                                if (!BiasInput)
                                {
                                    BiasInput = TextureShader.CreateInput(pxr::TfToken("bias"), pxr::SdfValueTypeNames->Float4);
                                }
                                BiasInput.Set(LinearColorToVec(TextureBias));
                            }

                            // ST
                            float TextureUV;
                            if (FSimReadyMaterialReparentUtility::FindScalarConstant(MaterialReparentIndex, TextureUVParam, TextureUV) || MaterialInst.GetScalarParameterValue(*TextureUVParam, TextureUV))
                            {
                                auto STInput = TextureShader.GetInput(USDTokens.st);

                                if (!STInput)
                                {
                                    STInput = TextureShader.CreateInput(USDTokens.st, pxr::SdfValueTypeNames->Float2);
                                }

                                pxr::UsdShadeShader STShader;
                                if (STInput && STInput.HasConnectedSource())
                                {
                                    pxr::UsdShadeConnectableAPI Source;
                                    pxr::TfToken SourceName;
                                    pxr::UsdShadeAttributeType SourceType;
                                    STInput.GetConnectedSource(&Source, &SourceName, &SourceType);
                                    STShader = pxr::UsdShadeShader(Source);
                                }
                                else // Can't find st shader, create one
                                {
                                    FString STShaderName = TextureUV != 0.0f ? TEXT("PrimvarSt1") : TEXT("PrimvarSt");
                                    auto STShaderPath = ShadeShader.GetPath().GetParentPath().AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*STShaderName)));
                                    STShader = pxr::UsdShadeShader::Define(Stage, STShaderPath);
                                }

                                if (STShader)
                                {
                                    STShader.SetShaderId(pxr::TfToken("UsdPrimvarReader_float2"));

                                    // varname
                                    auto VarnameInput = STShader.GetInput(USDTokens.varname);
                                    if (!VarnameInput)
                                    {
                                        VarnameInput = STShader.CreateInput(USDTokens.varname, pxr::SdfValueTypeNames->Token);
                                    }
                                    FString UVSetName = pxr::UsdUtilsGetPrimaryUVSetName().GetText();
                                    VarnameInput.Set(TextureUV == 1.0f ? pxr::TfToken(TCHAR_TO_ANSI(*(UVSetName + TEXT("1")))) : pxr::UsdUtilsGetPrimaryUVSetName());

                                    // st output
                                    auto Output = STShader.GetOutput(USDTokens.result);
                                    if (!Output)
                                    {
                                        Output = STShader.CreateOutput(USDTokens.result, pxr::SdfValueTypeNames->Float2);
                                    }

                                    if (Output)
                                    {
                                        STInput.ConnectToSource(Output);
                                    }
                                }
                            }

                            // Output
                            pxr::UsdShadeOutput Output;
                            float TextureMask;
                            if (FSimReadyMaterialReparentUtility::FindScalarConstant(MaterialReparentIndex, MaskParam, TextureMask) || MaterialInst.GetScalarParameterValue(*MaskParam, TextureMask))
                            {
                                if (TextureMask == 1.0f)
                                {
                                    Output = TextureShader.GetOutput(pxr::TfToken("g"));
                                    if (!Output)
                                    {
                                        Output = TextureShader.CreateOutput(pxr::TfToken("g"), pxr::SdfValueTypeNames->Float);
                                    }
                                }
                                else if (TextureMask == 2.0f)
                                {
                                    Output = TextureShader.GetOutput(pxr::TfToken("b"));
                                    if (!Output)
                                    {
                                        Output = TextureShader.CreateOutput(pxr::TfToken("b"), pxr::SdfValueTypeNames->Float);
                                    }
                                }
                                else if (TextureMask == 3.0f)
                                {
                                    Output = TextureShader.GetOutput(pxr::TfToken("a"));
                                    if (!Output)
                                    {
                                        Output = TextureShader.CreateOutput(pxr::TfToken("a"), pxr::SdfValueTypeNames->Float);
                                    }
                                }
                                else
                                {
                                    Output = TextureShader.GetOutput(pxr::TfToken("r"));
                                    if (!Output)
                                    {
                                        Output = TextureShader.CreateOutput(pxr::TfToken("r"), pxr::SdfValueTypeNames->Float);
                                    }
                                }
                            }
                            else
                            {
                                Output = TextureShader.GetOutput(pxr::TfToken("rgb"));
                                if (!Output)
                                {
                                    Output = TextureShader.CreateOutput(pxr::TfToken("rgb"), pxr::SdfValueTypeNames->Float3);
                                }
                            }

                            if (Output)
                            {
                                PropertyInput.ConnectToSource(Output);
                            }
                        }
                    }
                }
            }
        }
    }
}