// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "CoreMinimal.h"
#include "USDConverter.h"
#include "SimReadySettings.h"
#include "SimReadyStageActor.h"
#include "SimReadyMDL.h"
#include "SimReadyTexture.h"
#include "SimReadyAssetExportHelper.h"
#include "SimReadyNotificationHelper.h"
#include "SimReadyUSDSequenceImporter.h"
#include "SimReadyUSDTokens.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Extractors/TimeSamplesData.h"
#include "IMaterialEditor.h"
#include "MaterialEditor/MaterialEditorInstanceConstant.h"
#include "MaterialEditorInstanceNotifier.h"
#include "USDHashGenerator.h"
#include "USDGeometryCache.h"
#include "SimReadyAssetImportHelper.h"
#include "SimReadyUSDImporterHelper.h"
#include "ISimReadyRuntimeModule.h"
#include "SimReadyMaterialReparentSettings.h"
#include "SimReadyMaterialReparentUtility.h"


// NOTE: Texture (depending on type) might be recreated by TextureFactory, so can't use weakptr here. Or the ptr will be stale.
void ASimReadyStageActor::OverrideTextureSRGB(UTexture* Texture, bool SRGB)
{
    if (!(Texture && Texture->IsValidLowLevel()))
    {
        return;
    }

    if (Texture && Texture->SRGB != SRGB)
    {
        Texture->SRGB = SRGB;

        if (Texture->SRGB && Texture->CompressionSettings == TC_Normalmap)
        {
            Texture->CompressionSettings = TC_Default;
        }

        Texture->PostEditChange();
        Texture->MarkPackageDirty();
    }
}

bool IsNodeValid(const pxr::UsdPrim& TargetPrim, const pxr::UsdPrim& SourcePrim)
{
    if (TargetPrim == SourcePrim)
    {
        return true;
    }
    else
    {
        std::vector<pxr::UsdShadeInput> Inputs;
        if (SourcePrim.IsA<pxr::UsdShadeShader>())
        {
            Inputs = pxr::UsdShadeShader(SourcePrim).GetInputs();
        }
        else if (SourcePrim.IsA<pxr::UsdShadeNodeGraph>())
        {
            Inputs = pxr::UsdShadeNodeGraph(SourcePrim).GetInputs();
        }

        for (auto Input : Inputs)
        {
            if (Input.HasConnectedSource())
            {
                pxr::UsdShadeConnectableAPI Source;
                pxr::TfToken SourceName;
                pxr::UsdShadeAttributeType SourceType;
                Input.GetConnectedSource(&Source, &SourceName, &SourceType);

                if (IsNodeValid(TargetPrim, Source.GetPrim()))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool ASimReadyStageActor::IsValidNodeFromGraph(const pxr::UsdPrim& TargetPrim, const pxr::UsdShadeMaterial& ShadeMaterial)
{
    auto SurfaceOutput = ShadeMaterial.GetSurfaceOutput();
    if (SurfaceOutput)
    {
        pxr::UsdShadeConnectableAPI Source;
        pxr::TfToken SourceName;
        pxr::UsdShadeAttributeType SourceType;

        if (SurfaceOutput.GetConnectedSource(&Source, &SourceName, &SourceType))
        {
            return IsNodeValid(TargetPrim, Source.GetPrim());
        }
    }

    return false;
}

bool ASimReadyStageActor::GetGraphNodeInputName(const pxr::UsdShadeInput& TargetInput, const pxr::UsdPrim& Prim, const pxr::TfToken& SourceName, const FString& ParentInputName, FString& OutInputName)
{
    if (Prim.IsA<pxr::UsdShadeNodeGraph>())
    {
        auto NodeGraph = pxr::UsdShadeNodeGraph(Prim);
        auto NodeOutput = NodeGraph.GetOutput(SourceName);
        auto NodeInput = NodeGraph.GetInput(SourceName);
        
        if (NodeOutput && NodeOutput.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI OutputSource;
            pxr::TfToken OutputSourceName;
            pxr::UsdShadeAttributeType OutputSourceType;
            NodeOutput.GetConnectedSource(&OutputSource, &OutputSourceName, &OutputSourceType);

            if (GetGraphNodeInputName(TargetInput, OutputSource.GetPrim(), OutputSourceName, ParentInputName, OutInputName))
            {
                return true;
            }
        }
        else if (NodeInput && NodeInput.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI InputSource;
            pxr::TfToken InputSourceName;
            pxr::UsdShadeAttributeType InputSourceType;
            NodeInput.GetConnectedSource(&InputSource, &InputSourceName, &InputSourceType);

            if (GetGraphNodeInputName(TargetInput, InputSource.GetPrim(), InputSourceName, ParentInputName, OutInputName))
            {
                return true;
            }
        }
        
        if (NodeInput == TargetInput)
        {
            FString DisplayName = NodeInput.GetAttr().GetDisplayName().c_str();
            OutInputName = ParentInputName.IsEmpty() ? DisplayName : ParentInputName;
            return true;
        }
    }
    else if (Prim.IsA<pxr::UsdShadeShader>())
    {
        auto ShadeShader = pxr::UsdShadeShader(Prim);
        std::vector<pxr::UsdShadeInput> Inputs = ShadeShader.GetInputs();
        for (auto Input : Inputs)
        {
            FString InputName = Input.GetBaseName().GetText();
            FString UEInputName = ParentInputName.IsEmpty() ? InputName : (ParentInputName + TEXT(".") + InputName);
            if (Input == TargetInput)
            {
                FString DisplayName = Input.GetAttr().GetDisplayName().c_str();
                OutInputName = ParentInputName.IsEmpty() ? DisplayName : UEInputName;
                return true;
            }

            if (Input.HasConnectedSource())
            {
                pxr::UsdShadeConnectableAPI InputSource;
                pxr::TfToken InputSourceName;
                pxr::UsdShadeAttributeType InputSourceType;
                Input.GetConnectedSource(&InputSource, &InputSourceName, &InputSourceType);

                if (GetGraphNodeInputName(TargetInput, InputSource.GetPrim(), InputSourceName, UEInputName, OutInputName))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void ASimReadyStageActor::CreateTextureFromBuffer(const uint8* Content, uint64 Size, const FString& FileName, class UTexture*& OutTexture)
{
    FString TextureFileName = FileName;
    if (FSimReadyPathHelper::IsPackagePath(FileName))
    {
        TextureFileName = FSimReadyPathHelper::GetPackagedSubPath(FileName);
    }
    FString TextureName = FPaths::GetBaseFilename(TextureFileName);
    FSimReadyPathHelper::FixAssetName(TextureName);
    TextureName = GetUniqueImportName(FileName, TextureName);

    if (HasValidImportStage())
    {
        auto Object = USDPathToObject.Find(FSimReadyPathHelper::PrimPathToKey(FileName));
        if (Object && Object->IsValid())
        {
            OutTexture = Cast<UTexture>(Object->Get());
        }
    }
    
    if (OutTexture == nullptr)
    {
        OutTexture = LoadImportObject<UTexture>(ImportType::Texture, TextureName);
        if (OutTexture == nullptr && Content && Size > 0)
        {
            OutTexture = USimReadyTexture::CreateTextureFromBuffer(Content, Size, GetAssetPackage(ImportType::Texture, TextureName), *TextureName, GetAssetFlag(), FileName);
        }

        if (HasValidImportStage() && OutTexture)
        {
            USDPathToObject.FindOrAdd(FSimReadyPathHelper::PrimPathToKey(FileName), OutTexture);
        }
    }
}

void ASimReadyStageActor::UpdateGraphNodePrim(const pxr::SdfPath& Path)
{
#ifdef TODO_USD_UI_NODE_GRAPH_TYPEID
    auto Prim = GetUSDStage()->GetPrimAtPath(Path.GetAbsoluteRootOrPrimPath());
    if (Prim && Prim.HasAPI<pxr::UsdUINodeGraphNodeAPI>())
    {
        FString Token = Path.GetName().c_str();
        if (!Token.StartsWith(TEXT("ui:nodegraph")))
        {
            // Check valid node shader
            auto USDMaterial = FindShadeMaterial(Prim);
            if (USDMaterial)
            {
                if (IsValidNodeFromGraph(Prim, USDMaterial))
                {
                    // Node graph material
                    pxr::UsdShadeConnectableAPI Source;
                    pxr::TfToken SourceName;
                    pxr::UsdShadeAttributeType SourceType;

                    auto SurfaceOutput = USDMaterial.GetSurfaceOutput();
                    if (SurfaceOutput.GetConnectedSource(&Source, &SourceName, &SourceType))
                    {
                        auto GraphShader = pxr::UsdShadeShader(Source);
                        auto MaterialInst = FindObjectFromPath<UMaterialInstanceConstant>(USDMaterial.GetPath());
                        if (MaterialInst)
                        {
                            bool bRecompile = false;
                            pxr::UsdShadeInput ShaderInput = pxr::UsdShadeInput(GetUSDStage()->GetAttributeAtPath(Path));
                            if (ShaderInput)
                            {
                                if (ShaderInput.HasConnectedSource())
                                {
                                    bRecompile = true;
                                }
                                else
                                {
                                    FString UEInputName;
                                    if (!GetGraphNodeInputName(ShaderInput, Source.GetPrim(), SourceName, TEXT(""), UEInputName))
                                    {
                                        bRecompile = true;

                                    }
                                    else if (!FSimReadyUSDImporterHelper::LoadMdlInput(*MaterialInst, ShaderInput, UEInputName, true, 
                                    [&](const uint8* Content, uint64 Size, const FString& FileName, UTexture*& OutTexture)
                                    {
                                        CreateTextureFromBuffer(Content, Size, FileName, OutTexture);
                                    }))
                                    {
                                        bRecompile = true;
                                    }
                                }
                            }
                            else
                            {
                                bRecompile = true;
                            }

                            if (bRecompile)
                            {
                                FSimReadyUSDImporterHelper::LoadMaterialGraph(GraphShader, MaterialInst->GetMaterial());
                                FSimReadyUSDImporterHelper::UpdateMaterialGraphInputs(
                                    GraphShader, 
                                    MaterialInst, 
                                    [&](const uint8* Content, uint64 Size, const FString& FileName, UTexture*& OutTexture)
                                    {
                                        CreateTextureFromBuffer(Content, Size, FileName, OutTexture);
                                    }
                                );
                            }
                        }
                    }
                }
            }
        }
    }
#endif // TODO_USD_UI_NODE_GRAPH_TYPEID
}

bool ASimReadyStageActor::LoadMdlSchema(UMeshComponent* Component, int32 SlotIndex, const pxr::UsdShadeShader& ShadeShader)
{
    if (!ShadeShader)
    {
        return false;
    }

    const auto ImplementationSource = ShadeShader.GetImplementationSource();
    if (ImplementationSource == USDTokens.sourceAsset)
    {
        FString MdlPath, MaterialName;
        bool bRelativePath = false;
        if (FSimReadyUSDImporterHelper::GetMdlPathAndName(ShadeShader, MdlPath, MaterialName, bRelativePath))
        {
            FString ShaderPath = ShadeShader.GetPath().GetText();
            auto USDMaterial = FindShadeMaterial(ShadeShader.GetPrim());
            if (!USDMaterial)
            {
                return false;
            }

            FString MaterialPrimName = USDMaterial.GetPrim().GetName().GetText();

            auto Object = USDPathToObject.Find(FSimReadyPathHelper::PrimPathToKey(ShaderPath));
            if (Object && Object->IsValid())
            {
                if (Component)
                {
                    SetMaterial(*Component, SlotIndex, Cast<UMaterialInstanceConstant>(Object->Get()));
                }
            }
#ifdef TODO_USD_UI_NODE_GRAPH_TYPEID
            else if (ShadeShader.GetPrim().HasAPI<pxr::UsdUINodeGraphNodeAPI>() && 
                (USimReadyMDL::IsLocalMaterialGraphMDL(MdlPath) || FSimReadyUSDImporterHelper::IsValidMaterialGraph(ShadeShader)))
            {
                FString MaterialPath = USDMaterial.GetPath().GetText();
                auto Node = USDPathToObject.Find(FSimReadyPathHelper::PrimPathToKey(MaterialPath));
                if (Node && Node->IsValid())
                {
                    if (Component)
                    {
                        SetMaterial(*Component, SlotIndex, Cast<UMaterialInstanceConstant>(Node->Get()));
                    }
                }
                else
                {
                    FString Key;
                    if (HasValidImportStage())
                    {
                        Key = FUSDHashGenerator::ComputeSHAHash(USDMaterial).ToString();
                    }
                    MaterialPrimName = GetUniqueImportName(Key, MaterialPrimName);
                    UMaterialInstanceConstant* MaterialInst = nullptr;
                    if (HasValidImportStage())
                    {
                        MaterialInst = Cast<UMaterialInstanceConstant>(FUSDGeometryCache::Find(Key));
                    }

                    // NOTE: instance and material bind together for Material Graph
                    if (MaterialInst == nullptr)
                    {
                        auto MaterialInstPrimName = MaterialPrimName + TEXT("_Instance");
                        MaterialInst = LoadImportObject<UMaterialInstanceConstant>(ImportType::Material, MaterialInstPrimName);

                        if (MaterialInst == nullptr)
                        {
                            UMaterial* Material = NewObject<UMaterial>(GetAssetPackage(ImportType::Material, MaterialPrimName), GetAssetName(MaterialPrimName), GetAssetFlag());
                            if (Material)
                            {
                                FSimReadyUSDImporterHelper::LoadMaterialGraph(ShadeShader, Material);
                            }
                            else
                            {
                                return false;
                            }

                            if (HasValidImportStage())
                            {
                                FAssetRegistryModule::AssetCreated(Material);
                            }

                            MaterialInst = NewObject<UMaterialInstanceConstant>(GetAssetPackage(ImportType::Material, MaterialInstPrimName), GetAssetName(MaterialInstPrimName), GetAssetFlag());
                            if (MaterialInst)
                            {
                                MaterialInst->SetParentEditorOnly(Material);
                                FSimReadyUSDImporterHelper::UpdateMaterialGraphInputs(
                                    ShadeShader, 
                                    MaterialInst, 
                                    [&](const uint8* Content, uint64 Size, const FString& FileName, UTexture*& OutTexture)
                                    {
                                        CreateTextureFromBuffer(Content, Size, FileName, OutTexture);
                                    }
                                );
                            }
                            else
                            {
                                return false;
                            }
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
                        USDPathToObject.FindOrAdd(FSimReadyPathHelper::PrimPathToKey(MaterialPath), MaterialInst);
                        if (Component)
                        {
                            SetMaterial(*Component, SlotIndex, MaterialInst);
                        }
                    }
                }
            }
#endif			
            else
            {
                FString Key;
                if (HasValidImportStage())
                {
                    Key = FUSDHashGenerator::ComputeSHAHash(USDMaterial).ToString();
                }
                FString InstanceName = MaterialPrimName + TEXT("_Instance");
                if (ImportSettings.bSimReadyVehicle == false)
                {
                    InstanceName = GetUniqueImportName(Key, InstanceName);
                }

                UMaterialInstanceConstant* InstanceDynamic = nullptr;
                if (HasValidImportStage() && ImportSettings.bSimReadyVehicle == false)
                {
                    InstanceDynamic = Cast<UMaterialInstanceConstant>(FUSDGeometryCache::Find(Key));
                }

                if (InstanceDynamic == nullptr)
                {
                    InstanceDynamic = LoadImportObject<UMaterialInstanceConstant>(ImportType::Material, InstanceName);
                    if (InstanceDynamic == nullptr)
                    {
                        InstanceDynamic = CreateDynamicInstanceFromMdl(Component, SlotIndex, MdlPath, bRelativePath, MaterialName, InstanceName);

                        if (InstanceDynamic)
                        {
                            FNamedParameterTimeSamples ParameterTimeSamples;
                            UpdateShadeInputs(InstanceDynamic, ShadeShader, &ParameterTimeSamples);

                            InstanceDynamic->PostEditChange();
                            InstanceDynamic->MarkPackageDirty();

                            if (HasValidImportStage())
                            {
                                FAssetRegistryModule::AssetCreated(InstanceDynamic);
                            }

                            if (Component)
                            {
                                USDSequenceImporter->CreateMaterialTrack(Component, SlotIndex, ParameterTimeSamples);
                            }						
                        }
                    }

                    if (HasValidImportStage())
                    {
                        FUSDGeometryCache::Add(Key, InstanceDynamic);						
                    }
                }

                if (InstanceDynamic)
                {
                    USDPathToObject.FindOrAdd(FSimReadyPathHelper::PrimPathToKey(ShaderPath), InstanceDynamic);
                    if (Component)
                    {
                        SetMaterial(*Component, SlotIndex, InstanceDynamic);
                    }
                }
            }
            return true;
        }
    }

    return false;
}

void LoadInputTimeSamples(const pxr::UsdShadeInput& Input, const FString& DisplayName, FNamedParameterTimeSamples* ParameterTimeSamples)
{
    if (ParameterTimeSamples)
    {
        auto Attribute = Input.GetAttr();
        std::vector<double> TimeSamples;
        Attribute.GetTimeSamples(&TimeSamples);

        if (TimeSamples.size() > 0)
        {
            FFloatTimeSamples FloatTimeSamples;
            FColorTimeSamples ColorTimeSamples;
            for(auto TimeCode : TimeSamples)
            {
                pxr::VtValue VtValue;
                if (Input.Get(&VtValue, pxr::UsdTimeCode(TimeCode)))
                {
                    auto Type = VtValue.GetType();
                    if (Type == pxr::SdfValueTypeNames->Float.GetType())
                    {
                        float Value = VtValue.Get<float>();
                        FloatTimeSamples.TimeSamples.Add(TimeCode, Value);
                    }
                    else if (Type == pxr::SdfValueTypeNames->Int.GetType())
                    {
                        int32 Value = VtValue.Get<int>();
                        FloatTimeSamples.TimeSamples.Add(TimeCode, (float)Value);
                    }
                    else if (Type == pxr::SdfValueTypeNames->Bool.GetType())
                    {
                        bool Value = VtValue.Get<bool>();
                        FloatTimeSamples.TimeSamples.Add(TimeCode, Value ? 1.0f : 0.0f);
                    }
                    else if (Type == pxr::SdfValueTypeNames->Float2.GetType())
                    {
                        FLinearColor Value = USDConvertToLinearColor(VtValue.Get<pxr::GfVec2f>());
                        ColorTimeSamples.TimeSamples.Add(TimeCode, Value);
                    }
                    else if (Type == pxr::SdfValueTypeNames->Float3.GetType())
                    {
                        FLinearColor Value = USDConvertToLinearColor(VtValue.Get<pxr::GfVec3f>());
                        ColorTimeSamples.TimeSamples.Add(TimeCode, Value);
                    }
                    else if (Type == pxr::SdfValueTypeNames->Float4.GetType())
                    {
                        FLinearColor Value = USDConvertToLinearColor(VtValue.Get<pxr::GfVec4f>());
                        ColorTimeSamples.TimeSamples.Add(TimeCode, Value);
                    }
                }
            }

            if (FloatTimeSamples.TimeSamples.Num() > 0)
            {
                ParameterTimeSamples->ScalarTimeSamples.Add(*DisplayName, FloatTimeSamples);
            }
            else if (ColorTimeSamples.TimeSamples.Num() > 0)
            {
                ParameterTimeSamples->ColorTimeSamples.Add(*DisplayName, ColorTimeSamples);
            }
        }
    }
}

void ASimReadyStageActor::UpdateShadeInputs(UMaterialInstanceConstant* InstanceDynamic, const pxr::UsdShadeShader& ShadeShader, const FString& ParentInputName)
{
    std::vector<pxr::UsdShadeInput> Inputs = ShadeShader.GetInputs();
    for (auto Input: Inputs)
    {
        auto ConnectInput = Input;
        FString InputName = ParentInputName + TEXT(".") + Input.GetBaseName().GetText();
        if (Input.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI Source;
            pxr::TfToken SourceName;
            pxr::UsdShadeAttributeType SourceType;
            Input.GetConnectedSource(&Source, &SourceName, &SourceType);

            if (SourceType == pxr::UsdShadeAttributeType::Output)
            {
                auto InputShader = pxr::UsdShadeShader(Source);
                if (InputShader)
                {
                    UpdateShadeInputs(InstanceDynamic, InputShader, InputName);
                    continue;
                }
            }
            else
            {
                ConnectInput = Source.GetInput(SourceName);
            }
        }

        FSimReadyUSDImporterHelper::LoadMdlInput(
            *InstanceDynamic,
            ConnectInput,
            InputName,
            false,
            [&](const uint8* Content, uint64 Size, const FString& FileName, UTexture*& OutTexture)
            {
                CreateTextureFromBuffer(Content, Size, FileName, OutTexture);
            }
        );
    }
}

void ASimReadyStageActor::UpdateShadeInputs(UMaterialInstanceConstant* InstanceDynamic, const pxr::UsdShadeShader& ShadeShader, FNamedParameterTimeSamples* ParameterTimeSamples)
{
    if (!InstanceDynamic->Parent)
    {
        return;
    }

    if (!InstanceDynamic->GetMaterial())
    {
        return;
    }

    auto LocalBaseMDL = USimReadyMDL::GetLocalBaseMaterial(InstanceDynamic);
    bool bIsLocalBaseMDL = (LocalBaseMDL != nullptr);
    FString LocalBaseMDLName = bIsLocalBaseMDL ? LocalBaseMDL->GetName() : TEXT("");
    int32 MappedMaterialIndex = INDEX_NONE;
    if (!bIsLocalBaseMDL)
    {
        if (HasValidImportStage())
        {
            MappedMaterialIndex = FSimReadyMaterialReparentUtility::FindOmniMaterial(InstanceDynamic->Parent, LocalBaseMDLName, ESimReadyReparentUsage::Import);
            if (MappedMaterialIndex != INDEX_NONE)
            {
                bIsLocalBaseMDL = true;
            }
        }
    }

    //TMap<FName, bool> SwitchParameters;
    std::vector<pxr::UsdShadeInput> Inputs = ShadeShader.GetInputs();
    for(auto Input : Inputs)
    {
        // Mdl input name should get from the input from shader, not connect
        FString MdlInputName = Input.GetBaseName().GetText();		
        auto ConnectInput = Input;
        if (Input.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI Source;
            pxr::TfToken SourceName;
            pxr::UsdShadeAttributeType SourceType;
            Input.GetConnectedSource(&Source, &SourceName, &SourceType);
                        
            if (SourceType == pxr::UsdShadeAttributeType::Output)
            {
                auto InputShader = pxr::UsdShadeShader(Source);
                if (InputShader)
                {
                    UpdateShadeInputs(InstanceDynamic, InputShader, Input.GetBaseName().GetText());
                    continue;
                }
            }
            else
            {
                ConnectInput = Source.GetInput(SourceName);
            }
        }

        FString DisplayName;
        if (bIsLocalBaseMDL)
        {
            USimReadyMDL::GetDisplayNameFromLocalBaseMDL(LocalBaseMDLName, MdlInputName, DisplayName);
            if (MappedMaterialIndex != INDEX_NONE)
            {
                FSimReadyMaterialReparentUtility::FindUnrealParameter(MappedMaterialIndex, MoveTemp(DisplayName), DisplayName);
            }
        }
        else if (HasValidImportStage())
        {
            USimReadyMDL::GetImportDisplayName(InstanceDynamic->Parent->GetName(), MdlInputName, DisplayName);
        }

        if (!DisplayName.IsEmpty())
        {
            LoadInputTimeSamples(ConnectInput, DisplayName, ParameterTimeSamples);
            FSimReadyUSDImporterHelper::LoadMdlInput(
                *InstanceDynamic,
                ConnectInput,
                DisplayName,
                false,
                [&](const uint8* Content, uint64 Size, const FString& FileName, UTexture*& OutTexture)
                {
                    CreateTextureFromBuffer(Content, Size, FileName, OutTexture);
                }
            );
        }
    }

    //UpdateStaticParameters(InstanceDynamic, &SwitchParameters);
}

UMaterialInstanceConstant* ASimReadyStageActor::CreateDynamicInstanceFromMdl(UMeshComponent* Component, int32 SlotIndex, const FString& MdlPath,  bool bRelativePath, const FString& InMaterialName, const FString& InstanceName)
{
    UMaterialInstanceConstant* MaterialInst = nullptr;
    UMaterialInterface* Material = nullptr;

    // NOTE: Material Name could be with parameters
    FString MdlMaterialName = InMaterialName;
    int32 ParamStart = MdlMaterialName.Find(TEXT("("));
    if (ParamStart != INDEX_NONE)
    {
        MdlMaterialName = MdlMaterialName.Left(ParamStart);
    }

    if (!bRelativePath && USimReadyMDL::IsLocalBaseMDL(MdlPath))
    {
        Material = FSimReadyMaterialReparentUtility::FindUnrealMaterial(MdlMaterialName, ESimReadyReparentUsage::Import);
        if (Material == nullptr)
        {
            Material = USimReadyMDL::LoadLocalBaseMDL(MdlPath, MdlMaterialName);
        }
    }
    else
    {
        if (HasValidImportStage())
        {
            if (bRelativePath && USimReadyMDL::IsLocalBaseMDL(FPaths::GetCleanFilename(MdlPath)))
            {
                Material = FSimReadyMaterialReparentUtility::FindUnrealMaterial(MdlMaterialName, ESimReadyReparentUsage::Import);
            }
            
            if (Material == nullptr)
            {
                FString AssetMaterialName = GetUniqueImportName(MdlPath, MdlMaterialName);
                auto Object = USDPathToObject.Find(FSimReadyPathHelper::PrimPathToKey(MdlPath));
                if (Object && Object->IsValid())
                {
                    Material = Cast<UMaterialInterface>(Object->Get());
                }

                if (Material == nullptr)
                {
                    Material = LoadImportObject<UMaterialInterface>(ImportType::Material, AssetMaterialName);
                }

                FString AssetPath = MdlPath;
                AssetPath.ReplaceInline(TEXT("\\"), TEXT("/"));
                FString ModuleName = FPaths::GetPath(AssetPath) / FPaths::GetBaseFilename(AssetPath);
                int32 Colon = ModuleName.Find(TEXT(":"));
                if (Colon != INDEX_NONE)
                {
                    ModuleName = ModuleName.RightChop(Colon + 1);
                }
                ModuleName.ReplaceInline(TEXT("/"), TEXT("::"));

                TArray<FString> ModulePaths;
                if (Material == nullptr)
                {
                    Material = USimReadyMDL::ImportMDL(GetAssetPackage(ImportType::Material, AssetMaterialName), ModuleName, MdlMaterialName, ModulePaths, nullptr, GetAssetName(AssetMaterialName), GetAssetFlag(), MdlPath, [&](const uint8* Content, uint64 Size, const FString& FileName, UTexture*& OutTexture)
                    {
                        CreateTextureFromBuffer(Content, Size, FileName, OutTexture);
                    });
                }
                else
                {
                    USimReadyMDL::ImportMdlParameters(ModuleName, MdlMaterialName, ModulePaths, nullptr, AssetMaterialName);
                }

                // only for importing
                if (Material)
                {
                    USDPathToObject.FindOrAdd(FSimReadyPathHelper::PrimPathToKey(MdlPath), Material);
                }
            }
        }
    }

    if (Material)
    {
        MaterialInst = NewObject<UMaterialInstanceConstant>(GetAssetPackage(ImportType::Material, InstanceName), GetAssetName(InstanceName), GetAssetFlag());
        if (MaterialInst)
        {
            MaterialInst->SetParentEditorOnly(Material);
        }
    }

    return MaterialInst;
}

void ASimReadyStageActor::SetMaterial(UMeshComponent& Component, int32 SlotIndex, UMaterialInterface* Material)
{
    Component.SetMaterial(SlotIndex, Material);

    if (HasValidImportStage() && Material)
    {
        UMaterial* DefaultMaterial = LoadObject<UMaterial>(nullptr, *SimReadyDefaultMaterial);

        if (Component.IsA<UStaticMeshComponent>())
        {
            auto StaticMesh = Cast<UStaticMeshComponent>(&Component)->GetStaticMesh();
            if (StaticMesh)
            {
                if (StaticMesh->GetMaterial(SlotIndex) == nullptr || StaticMesh->GetMaterial(SlotIndex) == DefaultMaterial)
                {
                    StaticMesh->SetMaterial(SlotIndex, Material);
                }
            }
        }
        else if (Component.IsA<USkeletalMeshComponent>())
        {
            auto SkeletalMesh = Cast<USkeletalMeshComponent>(&Component)->SkeletalMesh;
            if (SkeletalMesh && SlotIndex < SkeletalMesh->Materials.Num())
            {
                if (SkeletalMesh->Materials[SlotIndex].MaterialInterface == nullptr
                || SkeletalMesh->Materials[SlotIndex].MaterialInterface == DefaultMaterial)
                {
                    SkeletalMesh->Materials[SlotIndex].MaterialInterface = Material;
                }
            }
        }
    }
}

pxr::SdfValueTypeName ConvertValueType(EMdlValueType ValueType)
{
    switch(ValueType)
    {
    case EMdlValueType::MDL_BOOL:
        return pxr::SdfValueTypeNames->Bool;
    case EMdlValueType::MDL_INT:
        return pxr::SdfValueTypeNames->Int;	
    case EMdlValueType::MDL_FLOAT:
        return pxr::SdfValueTypeNames->Float;	
    case EMdlValueType::MDL_FLOAT2:
        return pxr::SdfValueTypeNames->Float2;	
    case EMdlValueType::MDL_FLOAT3:
        return pxr::SdfValueTypeNames->Float3;	
    case EMdlValueType::MDL_FLOAT4:
        return pxr::SdfValueTypeNames->Float4;	
    case EMdlValueType::MDL_COLOR:
        return pxr::SdfValueTypeNames->Color3f;	
    case EMdlValueType::MDL_TEXTURE:
        return pxr::SdfValueTypeNames->Asset;
    default:
        return pxr::SdfValueTypeNames->Float;
    }
}

void FUSDExporter::ExportMdlSchemaParameters(const pxr::UsdStageRefPtr& Stage, UMaterialInstance& MaterialInst, pxr::UsdShadeShader& ShadeShader, const FSimReadyExportTextureSettings& TextureSettings, const FString& CustomPath, bool bUniqueName)
{
    auto SetTexturePath = [&](UTexture* Texture, const FString& InputName, const pxr::SdfValueTypeName& TypeName)
    {
        FString TexturePath;
        if (bUniqueName && Texture->GetOutermost() != GetTransientPackage() && !Texture->GetOutermost()->GetName().StartsWith(TEXT("/MDL/")) && !Texture->GetOutermost()->GetName().StartsWith(TEXT("/SimReady/")))
        {
            // Need uploading UE4 texture to server
            FString TextureFileName = bUniqueName ? FSimReadyAssetExportHelper::GetUniqueTextureName(Texture, TextureSettings.bDDSExport) : FSimReadyAssetExportHelper::GetTextureFileName(Texture);
            FString LayerPath = UTF8_TO_TCHAR((*Stage).GetEditTarget().GetLayer()->GetRealPath().c_str());
            FString OmniPath = FPaths::GetPath(LayerPath) / TextureFileName;

            if (CustomPath.IsEmpty())
            {
                TexturePath = TEXT("./") + TextureFileName;
            }
            else
            {
                TexturePath = FSimReadyPathHelper::ComputeRelativePath(CustomPath, LayerPath, true) / TextureFileName;
                OmniPath = CustomPath / TextureFileName;
            }

            FSimReadyAssetExportHelper::ExportTextureToPath(Texture, OmniPath, TextureSettings);

            if (bUniqueName)
            {
                FSimReadyAssetExportHelper::RegisterExportedTexture(Texture);
            }
        }

        if (!TexturePath.IsEmpty() && Texture->IsCurrentlyVirtualTextured())
        {
            TexturePath = FPaths::GetPath(TexturePath) / FPaths::GetBaseFilename(TexturePath) + TEXT(".<UDIM>.") + FPaths::GetExtension(TexturePath);
        }

        // NOTE: TexturePath could be empty
        ExportParameter<FString>(TypeName, InputName, ShadeShader, TexturePath);
        SetColorSpace(InputName, ShadeShader, Texture->SRGB);
    };

    UMaterialInterface* ParentMaterial = MaterialInst.Parent;
    if (MaterialInst.GetMaterial() == nullptr || ParentMaterial == nullptr)
    {
        return;
    }
    auto LocalBaseMaterial = USimReadyMDL::GetLocalBaseMaterial(&MaterialInst);
    bool bLocalBase = (LocalBaseMaterial != nullptr);
    FString BaseMaterialName = LocalBaseMaterial ? LocalBaseMaterial->GetName() : TEXT("");
    int32 MaterialReparentIndex = INDEX_NONE;
    if (!bLocalBase)
    {
        MaterialReparentIndex = FSimReadyMaterialReparentUtility::FindOmniMaterial(&MaterialInst, BaseMaterialName, ESimReadyReparentUsage::Export);
        bLocalBase = MaterialReparentIndex != INDEX_NONE;
    }

    auto GetMdlParamName = [&](FString ParameterName, EMdlValueType& ValueType, FString& MdlParamName)
    {
        if (bLocalBase)
        {
            FSimReadyMaterialReparentUtility::FindOmniParameter(MaterialReparentIndex, MoveTemp(ParameterName), ParameterName);
            USimReadyMDL::GetMdlParameterTypeAndNameFromLocalBaseMDL(BaseMaterialName, ParameterName, ValueType, MdlParamName);
        }
    };

    TArray<FString> UsedInputNames;
    // Export default parameters here
    if (bLocalBase)
    {
        if (BaseMaterialName == TEXT("OmniGlass"))
        {
            FString MdlParamName = TEXT("thin_walled");
            UsedInputNames.Add(MdlParamName);
            ExportParameter<bool>(pxr::SdfValueTypeNames->Bool, MdlParamName, ShadeShader, true);
        }
        else if (BaseMaterialName == TEXT("SimPBR"))
        {
            FString MdlParamName = TEXT("enable_transmission");
            UsedInputNames.Add(MdlParamName);
            ExportParameter<bool>(pxr::SdfValueTypeNames->Bool, MdlParamName, ShadeShader, true);
            MdlParamName = TEXT("enable_emission");
            UsedInputNames.Add(MdlParamName);
            ExportParameter<bool>(pxr::SdfValueTypeNames->Bool, MdlParamName, ShadeShader, true);
            MdlParamName = TEXT("emissive_intensity");
            UsedInputNames.Add(MdlParamName);
            ExportParameter<float>(pxr::SdfValueTypeNames->Float, MdlParamName, ShadeShader, 0.0f);
        }
    }

    FStaticParameterSet OutStaticParameters;
    MaterialInst.GetStaticParameterValues(OutStaticParameters);
    for (auto& StaticSwitchParameter : OutStaticParameters.StaticSwitchParameters)
    {
        if (StaticSwitchParameter.bOverride)
        {
            EMdlValueType ValueType = EMdlValueType::MDL_UNKNOWN;
            FString MdlParamName;
            GetMdlParamName(StaticSwitchParameter.ParameterInfo.Name.ToString(), ValueType, MdlParamName);

            if (ValueType == EMdlValueType::MDL_UNKNOWN)
            {
                continue;
            }

            UsedInputNames.Add(MdlParamName);
            ExportParameter<bool>(ConvertValueType(ValueType), MdlParamName, ShadeShader, StaticSwitchParameter.Value);
        }
    }

    if (MaterialReparentIndex != INDEX_NONE)
    {
        TArray<FSimReadyReparentParameterInfo> ParameterInfos;
        TArray<FSimReadyReparentConstantInfo> ConstantInfos;
        if (FSimReadyMaterialReparentUtility::GetAllReparentParametersAndConstants(MaterialReparentIndex, ParameterInfos, ConstantInfos))
        {
            for (auto ParameterInfo : ParameterInfos)
            {
                EMdlValueType ValueType = EMdlValueType::MDL_UNKNOWN;
                FString MdlParamName;
                USimReadyMDL::GetMdlParameterTypeAndNameFromLocalBaseMDL(BaseMaterialName, ParameterInfo.OmniParameter, ValueType, MdlParamName);
                if (ValueType == EMdlValueType::MDL_UNKNOWN)
                {
                    continue;
                }

                UsedInputNames.Add(MdlParamName);
                switch (ValueType)
                {
                case EMdlValueType::MDL_FLOAT:
                case EMdlValueType::MDL_INT:
                {
                    float OutValue = 0.0f;
                    if (MaterialInst.GetScalarParameterValue(*ParameterInfo.UnrealParameter, OutValue))
                    {
                        ExportParameter<float>(ConvertValueType(ValueType), MdlParamName, ShadeShader, OutValue);
                    }
                    break;
                }
                case EMdlValueType::MDL_COLOR:
                case EMdlValueType::MDL_FLOAT2:
                case EMdlValueType::MDL_FLOAT3:
                case EMdlValueType::MDL_FLOAT4:
                {
                    FLinearColor OutValue(EForceInit::ForceInitToZero);
                    if (MaterialInst.GetVectorParameterValue(*ParameterInfo.UnrealParameter, OutValue))
                    {
                        // converting from unreal translucent basic color to omni glass glass color
                        if (bLocalBase && BaseMaterialName == TEXT("OmniGlass") && MdlParamName == TEXT("glass_color"))
                        {
                            FLinearColor GlassColor = OutValue * 0.5f + FLinearColor(0.5f, 0.5f, 0.5f);
                            ExportParameter<pxr::GfVec4f>(ConvertValueType(ValueType), MdlParamName, ShadeShader, LinearColorToVec(GlassColor));
                        }
                        else
                        {
                            ExportParameter<pxr::GfVec4f>(ConvertValueType(ValueType), MdlParamName, ShadeShader, LinearColorToVec(OutValue));
                        }
                    }
                    break;
                }
                case EMdlValueType::MDL_TEXTURE:
                    UTexture* Texture = nullptr;
                    if (MaterialInst.GetTextureParameterValue(*ParameterInfo.UnrealParameter, Texture))
                    {
                        if (Texture)
                        {
                            SetTexturePath(Texture, MdlParamName, ConvertValueType(ValueType));
                        }
                    }
                    break;
                }
            }

            for (auto ConstantInfo : ConstantInfos)
            {
                EMdlValueType ValueType = EMdlValueType::MDL_UNKNOWN;
                FString MdlParamName;
                USimReadyMDL::GetMdlParameterTypeAndNameFromLocalBaseMDL(BaseMaterialName, ConstantInfo.OmniParameter, ValueType, MdlParamName);
                if (ValueType == EMdlValueType::MDL_UNKNOWN)
                {
                    continue;
                }

                UsedInputNames.Add(MdlParamName);
                switch (ValueType)
                {
                case EMdlValueType::MDL_BOOL:
                    ExportParameter<bool>(ConvertValueType(ValueType), MdlParamName, ShadeShader, ConstantInfo.ConstantBoolean);
                    break;
                case EMdlValueType::MDL_FLOAT:
                case EMdlValueType::MDL_INT:
                {
                    ExportParameter<float>(ConvertValueType(ValueType), MdlParamName, ShadeShader, ConstantInfo.ConstantValue.X);
                    break;
                }
                case EMdlValueType::MDL_COLOR:
                case EMdlValueType::MDL_FLOAT2:
                case EMdlValueType::MDL_FLOAT3:
                case EMdlValueType::MDL_FLOAT4:
                {
                    ExportParameter<pxr::GfVec4f>(ConvertValueType(ValueType), MdlParamName, ShadeShader, LinearColorToVec(FLinearColor(ConstantInfo.ConstantValue)));
                    break;
                }
                case EMdlValueType::MDL_TEXTURE:
                    if (!ConstantInfo.TextureValue.IsNull())
                    {
                        UTexture* Texture = ConstantInfo.TextureValue.Get();
                        if (Texture)
                        {
                            SetTexturePath(Texture, MdlParamName, ConvertValueType(ValueType));
                        }
                    }
                    break;
                }
            }
        }
    }
    else
    {
        for (auto ScalarParameter : MaterialInst.ScalarParameterValues)
        {
            EMdlValueType ValueType = EMdlValueType::MDL_UNKNOWN;
            FString MdlParamName;
            GetMdlParamName(ScalarParameter.ParameterInfo.Name.ToString(), ValueType, MdlParamName);

            if (ValueType == EMdlValueType::MDL_UNKNOWN)
            {
                continue;
            }
            UsedInputNames.Add(MdlParamName);
            ExportParameter<float>(ConvertValueType(ValueType), MdlParamName, ShadeShader, ScalarParameter.ParameterValue);
        }

        for (auto VectorParameter : MaterialInst.VectorParameterValues)
        {
            EMdlValueType ValueType = EMdlValueType::MDL_UNKNOWN;
            FString MdlParamName;
            GetMdlParamName(VectorParameter.ParameterInfo.Name.ToString(), ValueType, MdlParamName);

            if (ValueType == EMdlValueType::MDL_UNKNOWN)
            {
                continue;
            }
            UsedInputNames.Add(MdlParamName);

            // converting from unreal translucent basic color to omni glass glass color
            if (bLocalBase && BaseMaterialName == TEXT("OmniGlass") && MdlParamName == TEXT("glass_color"))
            {
                FLinearColor GlassColor = VectorParameter.ParameterValue * 0.5f + FLinearColor(0.5f, 0.5f, 0.5f);
                ExportParameter<pxr::GfVec4f>(ConvertValueType(ValueType), MdlParamName, ShadeShader, LinearColorToVec(GlassColor));
            }
            else
            {
                ExportParameter<pxr::GfVec4f>(ConvertValueType(ValueType), MdlParamName, ShadeShader, LinearColorToVec(VectorParameter.ParameterValue));
            }
        }

        for (auto TextureParameter : MaterialInst.TextureParameterValues)
        {
            UTexture* Texture = TextureParameter.ParameterValue;
            if (Texture)
            {
                EMdlValueType ValueType = EMdlValueType::MDL_UNKNOWN;
                FString MdlParamName;
                FString TextureParameterName = TextureParameter.ParameterInfo.Name.ToString();
                if (bLocalBase)
                {
                    FSimReadyMaterialReparentUtility::FindOmniParameter(MaterialReparentIndex, MoveTemp(TextureParameterName), TextureParameterName);
                }

                if (bLocalBase)
                {
                    USimReadyMDL::GetMdlParameterTypeAndNameFromLocalBaseMDL(BaseMaterialName, TextureParameterName, ValueType, MdlParamName);
                }

                if (ValueType == EMdlValueType::MDL_UNKNOWN)
                {
                    continue;
                }

                UsedInputNames.Add(MdlParamName);

                SetTexturePath(Texture, MdlParamName, ConvertValueType(ValueType));
            }
        }
    }

    //Remove unused inputs from mdl schema
    std::vector<pxr::UsdShadeInput> Inputs = ShadeShader.GetInputs();
    for(auto Input : Inputs)
    {
        auto ConnectInput = Input;
        if (Input.HasConnectedSource())
        {
            pxr::UsdShadeConnectableAPI Source;
            pxr::TfToken SourceName;
            pxr::UsdShadeAttributeType SourceType;
            Input.GetConnectedSource(&Source, &SourceName, &SourceType);
                                    
            ConnectInput = Source.GetInput(SourceName);
        }
        FString MdlInputName = ConnectInput.GetBaseName().GetText();
        if (UsedInputNames.Find(MdlInputName) == INDEX_NONE)
        {
            FString PropertyName = TEXT("inputs:")+MdlInputName;
            ShadeShader.GetPrim().RemoveProperty(pxr::TfToken(TCHAR_TO_ANSI(*PropertyName)));
        }
    }
}

void FUSDExporter::ExportMdlSchema(const pxr::UsdStageRefPtr& Stage, UMaterialInstance& MaterialInst, pxr::UsdShadeShader& ShadeShader)
{
    if (!ShadeShader)
    {
        return;
    }

    const auto ImplementationSource = ShadeShader.GetImplementationSource();
    if (ImplementationSource == USDTokens.sourceAsset)
    {
        FString MdlPath, MaterialName;
        bool bRelativePath = false;
        if (FSimReadyUSDImporterHelper::GetMdlPathAndName(ShadeShader, MdlPath, MaterialName, bRelativePath))
        {
            UMaterialInterface* ParentMaterial = MaterialInst.Parent;
            check(ParentMaterial);
            auto LocalBaseMdl = USimReadyMDL::GetLocalBaseMaterial(&MaterialInst);
            bool bLocalBase = (LocalBaseMdl != nullptr);
            FString MdlMaterialName = LocalBaseMdl ? LocalBaseMdl->GetName() : TEXT("");
            if (!bLocalBase)
            {
                bLocalBase = FSimReadyMaterialReparentUtility::FindOmniMaterial(&MaterialInst, MdlMaterialName, ESimReadyReparentUsage::Export) != INDEX_NONE;
            }

            if (bLocalBase)
            {
                // Update sourceAsset
                // NOTE: material name might not be the mdl module name
                FString ModuleName;
                if (USimReadyMDL::GetMDLModuleByMaterialName(MdlMaterialName, ModuleName))
                {
                    FString CurrentMdlPath = ModuleName + TEXT(".mdl");
                    if (CurrentMdlPath != MdlPath)
                    {
                        ShadeShader.SetSourceAsset(pxr::SdfAssetPath(TCHAR_TO_ANSI(*CurrentMdlPath)), USDTokens.mdl);
                    }
                }

                if (MdlMaterialName != MaterialName)
                {
                    ShadeShader.SetSourceAssetSubIdentifier(pxr::TfToken(TCHAR_TO_ANSI(*MdlMaterialName)), USDTokens.mdl);
                }
            }
            else
            {
                return;
            }

            ExportMdlSchemaParameters(Stage, MaterialInst, ShadeShader);
        }
    }
}