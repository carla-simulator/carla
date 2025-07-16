// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyUSDHelper.h"
#include "SimReadyUSDLog.h"
#include "SimReadyUSDSequenceImporter.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "LevelUtils.h"
#include "Engine/MapBuildDataRegistry.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/ScopedSlowTask.h"
#include "HAL/FileManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/DecalComponent.h"
#include "Components/SkyLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/RectLight.h"
#include "Engine/TextRenderActor.h"
#include "Engine/Font.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Engine/DecalActor.h"
#include "Engine/InstancedStaticMesh.h"
#include "Engine/SkyLight.h"
#include "CinematicCamera/Public/CineCameraActor.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "AnimNodes/AnimNode_CopyPoseFromMesh.h"
#include "LandscapeProxy.h"
#include "LandscapeComponent.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeDataAccess.h"
#include "LandscapeInfo.h"
#include "LandscapeEdit.h"
#include "LandscapeMaterialInstanceConstant.h"
#include "Materials/MaterialExpressionLandscapeVisibilityMask.h"
#include "MeshUtilitiesCommon.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "StaticMeshAttributes.h"
#include "Editor/UnrealEdTypes.h"
#include "Exporters/Exporter.h"

#include "SimReadyUSDModule.h"
#include "ISimReadyRuntimeModule.h"
#include "SimReadyPathHelper.h"
#include "SimReadyMDL.h"
#include "SimReadyAssetExportHelper.h"
#include "SimReadyStageActor.h"
#include "SimReadyNotificationHelper.h"
#include "SimReadyUSDTranslator.h"
#include "SimReadyUSDTokens.h"
#include "SimReadySlowTask.h"
#include "SimReadyReferenceCollector.h"
#include "SimReadyMaterialReparentUtility.h"
#include "SimReadyCarlaWrapper.h"
#include "USDConverter.h"
#include "USDCustomLayerData.h"
#include "IMaterialBakingModule.h"
#include "MaterialBakingStructures.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Interfaces/Interface_PostProcessVolume.h"
#include "Extractors/SequenceExtractor.h"
#include "USDCARLAVehicleTools.h"

#include "ILevelSequenceEditorToolkit.h"
#include "ISequencer.h"
#include "ISequencerModule.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "SLevelViewport.h"

#include <SimReadyUsdStageCtrl.h>

int32 FSimReadyUSDHelper::SuffixIndex = 0;

const FString BaseLayerSuffix = TEXT("Base");
const FString EditLayerSuffix = TEXT("Edit");

FString GetUSDExtension(bool bAsciiFormat)
{
    return bAsciiFormat ? TEXT(".usda") : TEXT(".usd");
}

namespace SimReadyTokens
{
    extern const pxr::TfToken SignalId;
    extern const pxr::TfToken WikiDataQCode;
    extern const pxr::TfToken SignalType;
    extern const pxr::TfToken TrafficLight;
    extern const pxr::TfToken SignalOrder;
    extern const pxr::TfToken Red;
    extern const pxr::TfToken Green;
    extern const pxr::TfToken Amber;
    extern const pxr::TfToken Signal;
    extern const pxr::TfToken Skin;
    extern const pxr::TfToken Fabric;
    extern const pxr::TfToken FurHair;
    extern const pxr::TfToken None;
}

void CheckMissedQCodes(pxr::UsdStage& Stage)
{
    auto PrimRange = Stage.Traverse(pxr::UsdTraverseInstanceProxies());
    for (auto Itr = PrimRange.begin(); Itr != PrimRange.end(); ++Itr)
    {
        auto Attr = Itr->GetAttribute(SimReadyTokens::WikiDataQCode);
        if (Attr && Attr.HasAuthoredValue())
        {
            Itr.PruneChildren();
            continue;
        }

        if (pxr::UsdGeomGprim(*Itr))
        {
            auto USDMaterialBinding = pxr::UsdShadeMaterialBindingAPI(*Itr);
            pxr::UsdShadeMaterial Material = USDMaterialBinding.ComputeBoundMaterial();
            if (Material)
            {
                if (FString(Material.GetPrim().GetName().GetText()).Contains("sidewalk"))
                {
                    SetQcode(*Itr, "Q177749");	// Sidewalk
                    continue;
                }
            }

            UE_LOG(LogSimReadyUsd, Warning, TEXT("Prim %s doesn't have Wiki Q code"), *FString(Itr->GetPath().GetText()));
        }
    }
}

void PostExportWorld(const UWorld& World, pxr::UsdStage& Stage, bool bExportXodr)
{
    CheckMissedQCodes(Stage);

    auto Prim = Stage.GetDefaultPrim();
    if (bExportXodr && Prim)
    {
        auto Path = FPaths::GetPath(TSoftObjectPtr<UWorld>(&World).GetLongPackageName());
        Path.RemoveFromStart("/Game/");
        Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + Path;
        Path = FPaths::Combine(Path, TEXT("OpenDrive"), World.GetName() + ".xodr");

        if (FPaths::FileExists(Path))
        {
            static const char* XodrFilename = "main.xodr";
            auto DestPath = FPaths::Combine(FPaths::GetPath(Stage.GetRootLayer()->GetIdentifier().c_str()), FString(XodrFilename));

            auto& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
            if (!PlatformFile.CopyFile(*DestPath, *Path))
            {
                UE_LOG(LogSimReadyUsd, Error, TEXT("Failed copy file from %s to %s"), *Path, *DestPath);
            }
            else
            {
                auto Attr = Prim.CreateAttribute(pxr::TfToken("omni:simready:openDrive"), pxr::SdfValueTypeNames->Asset, pxr::SdfVariabilityUniform);
                Attr.Set(pxr::SdfAssetPath(std::string("./") + XodrFilename));
            }
        }
        else
        {
            UE_LOG(LogSimReadyUsd, Warning, TEXT("Failed to find xodr for map %s"), *World.GetName());
        }
    }
}

pxr::UsdStageRefPtr FSimReadyUSDHelper::LoadUSDStageFromPath(const FString& Path)
{
    pxr::UsdStageRefPtr USDStage;
    const std::string PathANSI = TCHAR_TO_UTF8(*Path);
    pxr::SdfLayerRefPtr RootLayer = pxr::SdfLayer::FindOrOpen(PathANSI);
    if (RootLayer)
    {
        USDStage = pxr::UsdStage::Open(RootLayer);
    }

    return USDStage;
}

pxr::UsdStageRefPtr FSimReadyUSDHelper::CreateUSDStageFromPath(const FString& Path, bool bKeepExistingStage, bool bZUpAxis)
{
    UE_LOG(LogSimReadyUsd, Display, TEXT("Exporting %s"), *Path);
    // Clear existing or create a new one
    pxr::UsdStageRefPtr USDStage;
    const std::string PathANSI = TCHAR_TO_UTF8(*Path);
    pxr::SdfLayerRefPtr RootLayer = pxr::SdfLayer::FindOrOpen(PathANSI);
    if (RootLayer)
    {
        // Clear all content
        if (!bKeepExistingStage)
        {
            RootLayer->Clear();
        }

        USDStage = pxr::UsdStage::Open(RootLayer);
    }
    else
    {		
        USDStage = pxr::UsdStage::CreateNew(PathANSI);
    }

    if(!USDStage)
    {
        return nullptr;
    }

    USDStage->GetRootLayer()->SetPermissionToEdit(true);
    pxr::UsdGeomSetStageUpAxis(USDStage, bZUpAxis ? pxr::UsdGeomTokens->z : pxr::UsdGeomTokens->y);
    //pxr::UsdGeomSetStageMetersPerUnit(USDStage, pxr::UsdGeomLinearUnits::centimeters);
    pxr::UsdGeomSetStageMetersPerUnit(USDStage, pxr::UsdGeomLinearUnits::meters);

    USDStage->GetRootLayer()->SetStartTimeCode(FSequenceExtractor::Get().GetMasterStartTimeCode());
    USDStage->GetRootLayer()->SetEndTimeCode(FSequenceExtractor::Get().GetMasterEndTimeCode());
    USDStage->GetRootLayer()->SetTimeCodesPerSecond(FSequenceExtractor::Get().GetMasterTimeCodesPerSecond());

    FUSDCustomLayerData::ResetMaxRoughness(USDStage->GetRootLayer());

    return USDStage;	
}

pxr::SdfPath FSimReadyUSDHelper::InitDefaultPrim(const pxr::UsdStageRefPtr& USDStage, const pxr::TfToken& PrimKind)
{
    if (!USDStage->HasDefaultPrim())
    {
        pxr::SdfPath RootPrimPath = pxr::SdfPath::AbsoluteRootPath().AppendElementString(TCHAR_TO_UTF8(*SimReadyRootPrim));
        auto RootPrim = pxr::UsdGeomXform::Define(USDStage, RootPrimPath);
        pxr::UsdModelAPI(RootPrim).SetKind(PrimKind);
        USDStage->SetDefaultPrim(RootPrim.GetPrim());
    }

    return USDStage->GetDefaultPrim().GetPath();
}

inline bool ShouldCastDynamicShadows(UMaterialInterface* Material)
{
    return !Material->GetShadingModels().HasOnlyShadingModel(MSM_SingleLayerWater) &&
        (Material->GetBlendMode() == BLEND_Opaque
            || Material->GetBlendMode() == BLEND_Masked
            || (Material->GetBlendMode() == BLEND_Translucent && Material->GetCastDynamicShadowAsMasked()));
}

void ExportShadowFromMaterial(const pxr::UsdGeomXformable& USDGeom, UMaterialInterface* Material)
{
    if (Material && (!Material->CastsRayTracedShadows() || !ShouldCastDynamicShadows(Material)))
    {
        USDGeom.CreatePrimvar(USDTokens.doNotCastShadows, pxr::SdfValueTypeNames->Bool).Set(true);
    }
}

void ExportShadowFromStaticMesh(const pxr::UsdGeomXformable& USDGeom, UStaticMesh* StaticMesh)
{
    // check unlit
    bool bHasAllUnLitMaterials = true;
    const int32 NumMaterials = StaticMesh->StaticMaterials.Num();
    for (int32 MaterialIndex = 0; (MaterialIndex < NumMaterials); ++MaterialIndex)
    {
        UMaterialInterface* Material = StaticMesh->GetMaterial(MaterialIndex);

        if (Material)
        {
            if (Material->GetShadingModels().IsLit())
            {
                bHasAllUnLitMaterials = false;
                break;
            }
        }
        else
        {
            // Default material is lit
            bHasAllUnLitMaterials = false;
            break;
        }
    }

    if (bHasAllUnLitMaterials && NumMaterials > 0)
    {
        USDGeom.CreatePrimvar(USDTokens.doNotCastShadows, pxr::SdfValueTypeNames->Bool).Set(true);
    }
}

void ExportShadowFromPrimitiveComponent(const pxr::UsdGeomXformable& USDGeom, UPrimitiveComponent* PrimitiveComp)
{
    // Shadow
    bool bHasAllUnLitMaterials = true;
    const int32 NumMaterials = PrimitiveComp->GetNumMaterials();
    for (int32 MaterialIndex = 0; (MaterialIndex < NumMaterials); ++MaterialIndex)
    {
        UMaterialInterface* Material = PrimitiveComp->GetMaterial(MaterialIndex);

        if (Material)
        {
            if (Material->GetShadingModels().IsLit())
            {
                bHasAllUnLitMaterials = false;
                break;
            }
        }
        else
        {
            // Default material is lit
            bHasAllUnLitMaterials = false;
            break;
        }
    }

    if (!PrimitiveComp->CastShadow || (bHasAllUnLitMaterials && NumMaterials > 0))
    {
        USDGeom.CreatePrimvar(USDTokens.doNotCastShadows, pxr::SdfValueTypeNames->Bool).Set(true);
    }
}

bool LookupSupportedComponent(USceneComponent* ParentComponent, const FSimReadyExportSettings& ExportSettings)
{
    if (!ParentComponent)
    {
        return false;
    }

    for (auto Child : ParentComponent->GetAttachChildren())
    {
        if (Child && (
            // mesh
            Child->IsA<UMeshComponent>()
            // light
            || Child->IsA<ULightComponentBase>()
            // camera
            || Child->IsA<UCameraComponent>()
            // landscape
            || Child->IsA<ULandscapeComponent>()
            // text
            || Child->IsA<UTextRenderComponent>()
            // decal
            || (Child->IsA<UDecalComponent>() && ExportSettings.bExportDecalActors)
            ))
        {
            return true;
        }
        else if (LookupSupportedComponent(Child, ExportSettings))
        {
            return true;
        }
    }

    return false;
}

pxr::UsdPrim GetSectionPrimByIndex(const pxr::UsdStageRefPtr& Stage, pxr::SdfPath MeshPath, int32 SectionIndex)
{
    FString SectionName = FString::Printf(TEXT("Section%d"), SectionIndex);
    auto SectionPath = MeshPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SectionName)));
    auto Mesh = pxr::UsdGeomMesh::Get(Stage, SectionPath);
    if (Mesh)
    {
        return Mesh.GetPrim();
    }
    return pxr::UsdPrim();
}

FString GetLayerPathBySuffix(const FString& InPath, const FString& Suffix, bool bEditLayer = true)
{
    return bEditLayer ? FPaths::GetPath(InPath) / FPaths::GetBaseFilename(InPath) + TEXT("_") + Suffix + FPaths::GetExtension(InPath, true) : InPath;
}

void CreateEditLayerIfNotExist(const FString& InPath, bool bUpYAxis)
{
    auto BaseLayerPath = GetLayerPathBySuffix(InPath, BaseLayerSuffix);
    auto EditLayerPath = GetLayerPathBySuffix(InPath, EditLayerSuffix);

    // Create or Find the edit layer
    FSimReadyUSDHelper::CreateUSDStageFromPath(EditLayerPath, true, !bUpYAxis);

    auto RelativeBaseLayerPath = FSimReadyPathHelper::ComputeRelativePath(BaseLayerPath, InPath);
    auto RelativeEditLayerPath = FSimReadyPathHelper::ComputeRelativePath(EditLayerPath, InPath);

    // Create or Find the root layer
    auto Stage = FSimReadyUSDHelper::CreateUSDStageFromPath(InPath, true, !bUpYAxis);
    
    auto AddSubLayer = [](const pxr::SdfLayerHandle& RootLayer, const FString& LayerPath, int32 Index)
    {
        if (RootLayer && SimReadyUsdStageCtrl::FindSubLayerIndex(RootLayer, TCHAR_TO_UTF8(*LayerPath)) == INDEX_NONE)
        {			
            RootLayer->InsertSubLayerPath(TCHAR_TO_UTF8(*LayerPath), Index);
            RootLayer->Save();
        }
    };

    AddSubLayer(Stage->GetRootLayer(), RelativeBaseLayerPath, INDEX_NONE);
    AddSubLayer(Stage->GetRootLayer(), RelativeEditLayerPath, 0);

    // Set the default prim
    if (!Stage->HasDefaultPrim())
    {
        pxr::SdfPath RootPrimPath = pxr::SdfPath::AbsoluteRootPath().AppendElementString(TCHAR_TO_UTF8(*SimReadyRootPrim));
        auto RootPrim = Stage->GetPrimAtPath(RootPrimPath);
        if (RootPrim)
        {
            Stage->SetDefaultPrim(RootPrim);
        }
    }

    // Set Authoring to Edit layer
    FUSDCustomLayerData::SetAuthoringLayerCustomField(Stage->GetRootLayer(), TCHAR_TO_UTF8(*RelativeEditLayerPath));
    // Lock Base layer
    FUSDCustomLayerData::SetLayerCustomField(Stage->GetRootLayer(), TCHAR_TO_UTF8(*RelativeBaseLayerPath), FUSDCustomLayerData::LockedCustomKey, true);
    Stage->GetRootLayer()->Save();
}

template<typename Component>
bool IsChildrenSupported(const AActor * Actor)
{
    TArray<Component*> Components;
    Actor->GetComponents<Component>(Components, true);
    return (Components.Num() > 0);
}

bool FSimReadyUSDHelper::IsActorSupported(const AActor * Actor, const FSimReadyExportSettings& ExportSettings)
{
    // This is not necessary, but fast to check
    if (
        // light
        Actor->IsA<ADirectionalLight>() || Actor->IsA<APointLight>() 
        || Actor->IsA<ASpotLight>() || Actor->IsA<ARectLight>() 
        // camera
        || Actor->IsA<ACameraActor>() || Actor->IsA<ACineCameraActor>() 
        // mesh
        || Actor->IsA<AStaticMeshActor>() || Actor->IsA<ASkeletalMeshActor>()
        // landscape
        || Actor->IsA<ALandscapeProxy>() 
        // text
        || Actor->IsA<ATextRenderActor>()
        // decal
        || (Actor->IsA<ADecalActor>() && ExportSettings.bExportDecalActors)
        // sky light
        || Actor->IsA<ASkyLight>()
        )
    {
        return true;
    }

    // check owned components and children components
    if (
        // mesh
        IsChildrenSupported<UMeshComponent>(Actor)
        // light
        || IsChildrenSupported<ULightComponent>(Actor)
        // camera
        || IsChildrenSupported<UCameraComponent>(Actor)
        // landscape
        || IsChildrenSupported<ULandscapeComponent>(Actor)
        // text
        || IsChildrenSupported<UTextRenderComponent>(Actor)
        // decal
        || (IsChildrenSupported<UDecalComponent>(Actor) && ExportSettings.bExportDecalActors)
        )
    {
        return true;
    }

    // check attached components
    USceneComponent* ParentDefaultAttachComponent = Actor->GetDefaultAttachComponent();
    if (LookupSupportedComponent(ParentDefaultAttachComponent, ExportSettings))
    {
        return true;
    }

    auto USDModule = FModuleManager::GetModulePtr<ISimReadyUSDModule>("SimReadyUSD");
    if (USDModule && USDModule->HasUSDTranslator())
    {
        for(auto USDTranslator : USDModule->GetUSDTranslators())
        {
            if (USDTranslator->IsActorSupported(Actor))
            {
                return true;
            }
        }
    }

    return false;
}

bool IsUsingCopyPoseFromMesh(USkeletalMeshComponent* Comp)
{
    return (Comp->PostProcessAnimInstance && Comp->PostProcessAnimInstance->bUsingCopyPoseFromMesh) 
    || (Comp->AnimScriptInstance && Comp->AnimScriptInstance->bUsingCopyPoseFromMesh)
    || Comp->MasterPoseComponent.IsValid();
}

USkeletalMeshComponent* GetCopyPoseSourceComponent(USkeletalMeshComponent* Comp)
{
    USkeletalMeshComponent* SourceComponent = nullptr;

    if (Comp->MasterPoseComponent.IsValid())
    {
        SourceComponent = Cast<USkeletalMeshComponent>(Comp->MasterPoseComponent.Get());
    }
    else if (Comp->AnimScriptInstance && Comp->AnimScriptInstance->bUsingCopyPoseFromMesh)
    {
        UAnimBlueprintGeneratedClass* AnimClass = Cast<UAnimBlueprintGeneratedClass>(Comp->AnimScriptInstance->GetClass());
        if (AnimClass)
        {
            for (const FStructProperty* Property : AnimClass->GetAnimNodeProperties())
            {
                if (Property->Struct->IsChildOf(FAnimNode_CopyPoseFromMesh::StaticStruct()))
                {
                    FAnimNode_CopyPoseFromMesh* AnimNode = Property->ContainerPtrToValuePtr<FAnimNode_CopyPoseFromMesh>(Comp->AnimScriptInstance);
                    if (AnimNode)
                    {
                        if (AnimNode->SourceMeshComponent.IsValid())
                        {
                            SourceComponent = AnimNode->SourceMeshComponent.Get();
                        }
                        else if (AnimNode->bUseAttachedParent)
                        {
                            SourceComponent = Cast<USkeletalMeshComponent>(Comp->GetAttachParent());
                        }
                        break;
                    }
                }
            }
        }
    }
    else if (Comp->PostProcessAnimInstance && Comp->PostProcessAnimInstance->bUsingCopyPoseFromMesh)
    {
        UAnimBlueprintGeneratedClass* AnimClass = Cast<UAnimBlueprintGeneratedClass>(Comp->PostProcessAnimInstance->GetClass());
        if (AnimClass)
        {
            for (const FStructProperty* Property : AnimClass->GetPreUpdateNodeProperties())
            {
                if (Property->Struct->IsChildOf(FAnimNode_CopyPoseFromMesh::StaticStruct()))
                {
                    FAnimNode_CopyPoseFromMesh* AnimNode = Property->ContainerPtrToValuePtr<FAnimNode_CopyPoseFromMesh>(Comp->PostProcessAnimInstance);
                    if (AnimNode)
                    {
                        if (AnimNode->SourceMeshComponent.IsValid())
                        {
                            SourceComponent = AnimNode->SourceMeshComponent.Get();
                        }
                        else if (AnimNode->bUseAttachedParent)
                        {
                            SourceComponent = Cast<USkeletalMeshComponent>(Comp->GetAttachParent());
                        }
                        break;
                    }
                }
            }
        }
    }

    return SourceComponent;
}

void BindPreviewSurfaceTexture(const pxr::UsdStageRefPtr& USDStage, pxr::UsdShadeShader& PSUSDShader, const pxr::SdfPath& MaterialPrimPath, const FString& ShaderName, 
    const pxr::TfToken& name, const pxr::SdfValueTypeName& typeName, const pxr::TfToken& out, const pxr::SdfValueTypeName& outTypeName, const FString& TexFilePath, bool SRGB)
{
    auto TexPath = MaterialPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*ShaderName)));
    auto TexShader = pxr::UsdShadeShader::Define(USDStage, TexPath);
    PSUSDShader.CreateInput(name, typeName).ConnectToSource(TexShader.CreateOutput(out, outTypeName));
    TexShader.SetShaderId(USDTokens.uvTexture);
    // set file
    auto TexInput = TexShader.CreateInput(USDTokens.file, pxr::SdfValueTypeNames->Asset);
    TexInput.Set(pxr::SdfAssetPath(TCHAR_TO_ANSI(*TexFilePath)));
    //TexInput.GetAttr().SetColorSpace(SRGB ? USDTokens.sRGB : USDTokens.raw);
    // set sourceColorSpace
    TexShader.CreateInput(USDTokens.sourceColorSpace, pxr::SdfValueTypeNames->Token).Set(SRGB ? USDTokens.sRGB : USDTokens.raw);
    // set st
    auto STPath = MaterialPrimPath.AppendElementString("PrimST");
    auto STShader = pxr::UsdShadeShader::Define(USDStage, STPath);
    TexShader.CreateInput(USDTokens.st, pxr::SdfValueTypeNames->Float2).ConnectToSource(STShader.CreateOutput(USDTokens.result, pxr::SdfValueTypeNames->Float2));
    STShader.SetShaderId(pxr::TfToken("UsdPrimvarReader_float2"));
    // Version 2.2 varname is token
    STShader.CreateInput(USDTokens.varname, pxr::SdfValueTypeNames->Token).Set(pxr::UsdUtilsGetPrimaryUVSetName());
    // Version 2.3 varname is string
    //STShader.CreateInput(USDTokens.varname, pxr::SdfValueTypeNames->String).Set("st_0");
    // set wrap
    TexShader.CreateInput(pxr::TfToken("wrapS"), pxr::SdfValueTypeNames->Token).Set(pxr::TfToken("repeat"));
    TexShader.CreateInput(pxr::TfToken("wrapT"), pxr::SdfValueTypeNames->Token).Set(pxr::TfToken("repeat"));
    if (name == pxr::TfToken("normal"))
    {
        // Flip Y
        TexShader.CreateInput(pxr::TfToken("scale"), pxr::SdfValueTypeNames->Float4).Set(pxr::GfVec4f(2.0f, -2.0f, 2.0f, 2.0f));
        TexShader.CreateInput(pxr::TfToken("bias"), pxr::SdfValueTypeNames->Float4).Set(pxr::GfVec4f(-1.0f, 1.0f, -1.0f, -1.0f));
    }
}

const FNamedParameterTimeSamples* GetMaterialParameterTimeSample(USceneComponent* SceneComponent, int32 MaterialIndex)
{
    auto MaterialTimeSamples = FSequenceExtractor::Get().GetMaterialTimeSamples(SceneComponent);

    if (MaterialTimeSamples)
    {
        return MaterialTimeSamples->Find(MaterialIndex);
    }

    return nullptr;
}

pxr::UsdShadeMaterial FSimReadyUSDHelper::BindMaterial(const pxr::UsdStageRefPtr& USDStage, const pxr::UsdPrim& Prim, const pxr::SdfPath& ScopePrimPath, const FString& MaterialName)
{
    auto ScopePrim = pxr::UsdGeomScope::Define(USDStage, ScopePrimPath);

    auto MaterialPrimPath = ScopePrim.GetPath().AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*MaterialName)));

    auto USDMaterial = pxr::UsdShadeMaterial::Define(USDStage, MaterialPrimPath);
    if (USDMaterial)
    {
        if (Prim.IsA<pxr::UsdGeomSubset>())
        {
            pxr::UsdGeomSubset(Prim).CreateFamilyNameAttr().Set(pxr::UsdShadeTokens->materialBind);
        }

        // Bind material
        auto USDMaterialBinding = pxr::UsdShadeMaterialBindingAPI::Apply(Prim);
        USDMaterialBinding.Bind(USDMaterial);
    }

    return USDMaterial;
}

pxr::UsdStageRefPtr FSimReadyUSDHelper::CreateUsdMaterialStage(const FString& MaterialUsdPath, const FSimReadyExportSettings& ExportSettings)
{
    // keep exist materials
    pxr::UsdStageRefPtr USDStage = CreateUSDStageFromPath(MaterialUsdPath, true, !ExportSettings.bUpYAxis);

    return USDStage;
}

pxr::UsdShadeMaterial FSimReadyUSDHelper::DefineUsdMaterial(const pxr::UsdStageRefPtr& USDStage, const FString& MaterialName)
{
    auto MaterialPrimPath = pxr::SdfPath::AbsoluteRootPath().AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_UTF8(*MaterialName)));
    auto USDMaterial = pxr::UsdShadeMaterial::Define(USDStage, MaterialPrimPath);
    USDStage->SetDefaultPrim(USDMaterial.GetPrim());
    return USDMaterial;
}

void FSimReadyUSDHelper::ExportParameterTimeSamplesToMDLSchema(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& MaterialPrimPath, const FString& MaterialName, UMaterialInterface* Material, const FNamedParameterTimeSamples& ParameterTimeSamples, const FSimReadyExportMaterialResult& MaterialResult, const FString& MDLAbsolutePath, bool bMaterialNameAsTextureFolder, const FSimReadyExportTextureSettings& TextureSettings)
{
    auto USDMaterial = pxr::UsdShadeMaterial::Define(USDStage, MaterialPrimPath);
    if (!USDMaterial)
    {
        return;
    }

    // Create shader prim
    auto ShaderPrimPath = MaterialPrimPath.AppendElementString(TCHAR_TO_ANSI(*MaterialName));
    auto USDShader = pxr::UsdShadeShader::Define(USDStage, ShaderPrimPath);
    if (!USDShader)
    {
        return;
    }

    for (auto ScalarParameter : MaterialResult.ScalarParameters)
    {
        const FString Name = ScalarParameter.Value;

        auto TraverseCallback = [&](int32 TimeCode, float TimeSample)
        {
#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
            auto Input = USDShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float);
#else // Create input to UsdMaterial and connect to shader
            auto Input = USDMaterial.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float);
            USDShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float).ConnectToSource(Input);
#endif
            Input.Set(TimeSample, pxr::UsdTimeCode(TimeCode));
        };

        ParameterTimeSamples.TraverseScalarTimeSamples(ScalarParameter.Key, TraverseCallback);
    }

    for (auto VectorParameter : MaterialResult.VectorParameters)
    {
        const FString Name = VectorParameter.Value;

        auto TraverseCallback = [&](int32 TimeCode, const FLinearColor& TimeSample)
        {
#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
            auto Input = USDShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float4);
#else // Create input to UsdMaterial and connect to shader
            auto Input = USDMaterial.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float4);
            USDShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float4).ConnectToSource(Input);
#endif
            Input.Set(pxr::GfVec4f(TimeSample.R, TimeSample.G, TimeSample.B, TimeSample.A), pxr::UsdTimeCode(TimeCode));
        };
        ParameterTimeSamples.TraverseColorTimeSamples(VectorParameter.Key, TraverseCallback);
    }
}

void FSimReadyUSDHelper::ExportInstanceParametersToMDLSchema(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& MaterialPrimPath, const FString& MaterialName, UMaterialInstance* Instance, const FSimReadyExportMaterialResult& MaterialResult, const FString& MDLAbsolutePath, bool bMaterialNameAsTextureFolder, const FSimReadyExportTextureSettings& TextureSettings)
{
    auto USDMaterial = pxr::UsdShadeMaterial::Define(USDStage, MaterialPrimPath);
    if (!USDMaterial)
    {
        return;
    }

    // Create shader prim
    auto ShaderPrimPath = MaterialPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*MaterialName)));
    auto USDShader = pxr::UsdShadeShader::Define(USDStage, ShaderPrimPath);
    if (!USDShader)
    {
        return;
    }

    auto ParentMaterial = Instance->GetMaterial();
    bool bReplaceTextureCube = ParentMaterial && (ParentMaterial->GetName() == CARLA_FAKE_INTERIOR_MAT);

    for(auto ScalarParameter : MaterialResult.ScalarParameters)
    {
        const FString Name = ScalarParameter.Value;

        float OutValue = 0.0f;
        if (!Instance->GetScalarParameterValue(ScalarParameter.Key, OutValue))
        {
            continue;
        }

#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
        auto Input = USDShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float);
#else // Create input to UsdMaterial and connect to shader
        auto Input = USDMaterial.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float);
        USDShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float).ConnectToSource(Input);
#endif
        Input.Set(OutValue);
    }

    for(auto VectorParameter : MaterialResult.VectorParameters)
    {
        const FString Name = VectorParameter.Value;

        FLinearColor OutValue;
        if (!Instance->GetVectorParameterValue(VectorParameter.Key, OutValue))
        {
            continue;
        }

#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
        auto Input = USDShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float4);
#else // Create input to UsdMaterial and connect to shader
        auto Input = USDMaterial.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float4);
        USDShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Float4).ConnectToSource(Input);
#endif
        Input.Set(pxr::GfVec4f(OutValue.R, OutValue.G, OutValue.B, OutValue.A));
    }

    for(auto TextureParameter : MaterialResult.TextureParameters)
    {
        const FString Name = TextureParameter.Value;

        UTexture* OutValue;
        if (!Instance->GetTextureParameterValue(TextureParameter.Key, OutValue))
        {
            continue;
        }

        if (!OutValue)
        {
            continue;
        }

#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
        auto Input = USDShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Asset);
#else // Create input to UsdMaterial and connect to shader
        auto Input = USDMaterial.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Asset);
        USDShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*Name)), pxr::SdfValueTypeNames->Asset).ConnectToSource(Input);
#endif

        FString BasePath = FPaths::GetPath(MDLAbsolutePath);
        FString TexturePath = (bMaterialNameAsTextureFolder ? MaterialName : TEXT("Textures")) / FSimReadyAssetExportHelper::GetUniqueTextureName(OutValue, TextureSettings.bDDSExport);
        FString OmniPath = BasePath / TexturePath;		
        auto LayerPath = (*USDStage).GetEditTarget().GetLayer()->GetRealPath().c_str();
        FString RelativePath = FSimReadyPathHelper::ComputeRelativePath(BasePath, LayerPath, true) / FPaths::GetPath(TexturePath) / FPaths::GetBaseFilename(TexturePath) + (bReplaceTextureCube ? TEXT("_f.") : TEXT(".")) + FPaths::GetExtension(TexturePath);

        if (!RelativePath.IsEmpty() && OutValue->IsCurrentlyVirtualTextured())
        {
            RelativePath = FPaths::GetPath(RelativePath) / FPaths::GetBaseFilename(RelativePath) + TEXT(".<UDIM>.") + FPaths::GetExtension(RelativePath);
        }

        Input.Set(pxr::SdfAssetPath(TCHAR_TO_ANSI(*RelativePath)));

        Input.GetAttr().SetColorSpace(OutValue->SRGB ? USDTokens.sRGB : USDTokens.raw);
        
        if (!FSimReadyAssetExportHelper::IsTextureExported(OutValue))
        {
            if (FSimReadyReferenceCollector::Get().FindDependency(OutValue->GetOuter()->GetFName()))
            {
                FString Text = FString::Printf(TEXT("Exporting Object %s..."), *OutValue->GetName());
                FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
            }

            if (FSimReadySlowTask::Get().IsProgressCancelled())
            {
                return;
            }

            auto NewTextureSettings = TextureSettings;
            NewTextureSettings.bReplaceTextureCube = bReplaceTextureCube;
            if (FSimReadyAssetExportHelper::ExportTextureToPath(OutValue, OmniPath, NewTextureSettings))
            {
                FSimReadyAssetExportHelper::RegisterExportedTexture(OutValue);
            }
        }
    }
}

void FSimReadyUSDHelper::CreateMDLSchema(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& MaterialPrimPath, bool MdlSchema, const FString& MaterialName,
    const FString & MaterialRelativePath)
{
    auto USDMaterial = pxr::UsdShadeMaterial::Define(USDStage, MaterialPrimPath);
    if (!USDMaterial)
    {
        return;
    }

    // Create shader prim
    auto ShaderPrimPath = MaterialPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*MaterialName)));
    auto USDShader = pxr::UsdShadeShader::Define(USDStage, ShaderPrimPath);
    if (!USDShader)
    {
        return;
    }

    if (MdlSchema)
    {
        //The MDL Schema in USD requires the subIdentifier attribute to be the material name
        USDMaterial.CreateSurfaceOutput(USDTokens.mdl).ConnectToSource(USDShader.CreateOutput(USDTokens.out, pxr::SdfValueTypeNames->Token));
        USDShader.SetSourceAsset(pxr::SdfAssetPath(TCHAR_TO_ANSI(*MaterialRelativePath)), USDTokens.mdl);
        USDShader.SetSourceAssetSubIdentifier(pxr::TfToken(TCHAR_TO_ANSI(*MaterialName)), USDTokens.mdl);
    }
    else
    {
        // Set surface
        USDMaterial.CreateSurfaceOutput().ConnectToSource(USDShader.CreateOutput(USDTokens.out, pxr::SdfValueTypeNames->Token));
        USDShader.CreateIdAttr(pxr::VtValue(USDTokens.mdlMaterial));
        USDShader.GetPrim().CreateAttribute(USDTokens.module, pxr::SdfValueTypeNames->Asset).Set(pxr::SdfAssetPath(TCHAR_TO_ANSI(*MaterialRelativePath)));
        USDShader.GetPrim().CreateAttribute(USDTokens.name, pxr::SdfValueTypeNames->String).Set(std::string(TCHAR_TO_ANSI(*MaterialName)));
    }
}

void FSimReadyUSDHelper::CreatePSSchema(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& MaterialPrimPath, const FString& MaterialName,
    const TMap<EMaterialProperty, FString>& PropertyTextures, const TMap<EMaterialProperty, FLinearColor>& PropertyConstants)
{
    auto USDMaterial = pxr::UsdShadeMaterial::Define(USDStage, MaterialPrimPath);
    if (!USDMaterial)
    {
        return;
    }

    // Preview surface output
    auto PSShaderPrimPath = MaterialPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*(MaterialName + "PreviewSurface"))));
    auto PSUSDShader = pxr::UsdShadeShader::Define(USDStage, PSShaderPrimPath);
    if (!PSUSDShader)
    {
        return;
    }

    USDMaterial.CreateSurfaceOutput().ConnectToSource(PSUSDShader.CreateOutput(USDTokens.surface, pxr::SdfValueTypeNames->Token));

    PSUSDShader.SetShaderId(USDTokens.previewSurface);

    bool bMaskedShading = false;

    // preview surface inputs
    for (auto Iterator : PropertyTextures)
    {
        EMaterialProperty Property = Iterator.Key;

        switch (Property)
        {
        case MP_BaseColor:
            BindPreviewSurfaceTexture(USDStage, PSUSDShader, MaterialPrimPath, MaterialName + "DiffuseColorTex",
                pxr::TfToken("diffuseColor"), pxr::SdfValueTypeNames->Color3f,
                pxr::TfToken("rgb"), pxr::SdfValueTypeNames->Float3, Iterator.Value, true);
            break;
        case MP_Normal:
            BindPreviewSurfaceTexture(USDStage, PSUSDShader, MaterialPrimPath, MaterialName + "NormalTex",
                pxr::TfToken("normal"), pxr::SdfValueTypeNames->Normal3f,
                pxr::TfToken("rgb"), pxr::SdfValueTypeNames->Float3, Iterator.Value, false);
            break;
        case MP_Metallic:
            BindPreviewSurfaceTexture(USDStage, PSUSDShader, MaterialPrimPath, MaterialName + "MetallicTex",
                pxr::TfToken("metallic"), pxr::SdfValueTypeNames->Float,
                pxr::TfToken("r"), pxr::SdfValueTypeNames->Float, Iterator.Value, false);
            break;
        case MP_Specular:
            BindPreviewSurfaceTexture(USDStage, PSUSDShader, MaterialPrimPath, MaterialName + "SpecularColorTex",
                pxr::TfToken("specularColor"), pxr::SdfValueTypeNames->Color3f,
                pxr::TfToken("rgb"), pxr::SdfValueTypeNames->Float3, Iterator.Value, false);
            break;
        case MP_Roughness:
            BindPreviewSurfaceTexture(USDStage, PSUSDShader, MaterialPrimPath, MaterialName + "RoughnessTex",
                pxr::TfToken("roughness"), pxr::SdfValueTypeNames->Float,
                pxr::TfToken("r"), pxr::SdfValueTypeNames->Float, Iterator.Value, false);
            break;
        case MP_Opacity:
            BindPreviewSurfaceTexture(USDStage, PSUSDShader, MaterialPrimPath, MaterialName + "OpacityTex",
                pxr::TfToken("opacity"), pxr::SdfValueTypeNames->Float,
                pxr::TfToken("r"), pxr::SdfValueTypeNames->Float, Iterator.Value, false);
            break;
        case MP_OpacityMask:
            BindPreviewSurfaceTexture(USDStage, PSUSDShader, MaterialPrimPath, MaterialName + "OpacityMaskTex",
                pxr::TfToken("opacity"), pxr::SdfValueTypeNames->Float,
                pxr::TfToken("r"), pxr::SdfValueTypeNames->Float, Iterator.Value, false);
            bMaskedShading = true;
            break;
        case MP_EmissiveColor:
            BindPreviewSurfaceTexture(USDStage, PSUSDShader, MaterialPrimPath, MaterialName + "EmissiveColorTex",
                pxr::TfToken("emissiveColor"), pxr::SdfValueTypeNames->Color3f,
                pxr::TfToken("rgb"), pxr::SdfValueTypeNames->Float3, Iterator.Value, true);
            break;
        }
    }

    for (auto Iterator : PropertyConstants)
    {
        EMaterialProperty Property = Iterator.Key;
        auto PropertyValue = Iterator.Value;
        switch (Property)
        {
        case MP_BaseColor:
            PSUSDShader.CreateInput(pxr::TfToken("diffuseColor"), pxr::SdfValueTypeNames->Color3f).Set(pxr::GfVec3f(PropertyValue.R, PropertyValue.G, PropertyValue.B));
            break;
        case MP_Normal:
            PSUSDShader.CreateInput(pxr::TfToken("normal"), pxr::SdfValueTypeNames->Normal3f).Set(pxr::GfVec3f(PropertyValue.R, PropertyValue.G, PropertyValue.B));
            break;
        case MP_Metallic:
            PSUSDShader.CreateInput(pxr::TfToken("metallic"), pxr::SdfValueTypeNames->Float).Set(PropertyValue.R);
            break;
        case MP_Specular:
            //UE4 specular is float, convert to float3 specularColor of USD preview surface
            PSUSDShader.CreateInput(pxr::TfToken("specularColor"), pxr::SdfValueTypeNames->Color3f).Set(pxr::GfVec3f(PropertyValue.R, PropertyValue.R, PropertyValue.R));
            break;
        case MP_Roughness:
            PSUSDShader.CreateInput(pxr::TfToken("roughness"), pxr::SdfValueTypeNames->Float).Set(PropertyValue.R);
            break;
        case MP_Opacity:
            PSUSDShader.CreateInput(pxr::TfToken("opacity"), pxr::SdfValueTypeNames->Float).Set(PropertyValue.R);
            break;
        case MP_OpacityMask:
            PSUSDShader.CreateInput(pxr::TfToken("opacity"), pxr::SdfValueTypeNames->Float).Set(PropertyValue.R);
            bMaskedShading = true;
            break;
        case MP_EmissiveColor:
            PSUSDShader.CreateInput(pxr::TfToken("emissiveColor"), pxr::SdfValueTypeNames->Color3f).Set(pxr::GfVec3f(PropertyValue.R, PropertyValue.G, PropertyValue.B));
            break;
        case MP_ShadingModel: // ShadingModel is used for opacity threshold here.
            if (bMaskedShading)
            {
                PSUSDShader.CreateInput(pxr::TfToken("opacityThreshold"), pxr::SdfValueTypeNames->Float).Set(PropertyValue.R);
            }
            break;
        }
    }
}

void PropagateSplineDeformationToRawMesh(const USplineMeshComponent* InSplineMeshComponent, FMeshDescription &OutRawMesh)
{
    TVertexAttributesRef<FVector> VertexPositions = OutRawMesh.VertexAttributes().GetAttributesRef<FVector>(MeshAttribute::Vertex::Position);
    TVertexInstanceAttributesRef<FVector> VertexInstanceNormals = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<FVector>(MeshAttribute::VertexInstance::Normal);
    TVertexInstanceAttributesRef<FVector> VertexInstanceTangents = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<FVector>(MeshAttribute::VertexInstance::Tangent);
    TVertexInstanceAttributesRef<float> VertexInstanceBinormalSigns = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<float>(MeshAttribute::VertexInstance::BinormalSign);

    // Apply spline deformation for each vertex's tangents
    int32 WedgeIndex = 0;
    for (const FPolygonID PolygonID : OutRawMesh.Polygons().GetElementIDs())
    {
        for (const FTriangleID TriangleID : OutRawMesh.GetPolygonTriangleIDs(PolygonID))
        {
            for (int32 Corner = 0; Corner < 3; ++Corner, ++WedgeIndex)
            {
                const FVertexInstanceID VertexInstanceID = OutRawMesh.GetTriangleVertexInstance(TriangleID, Corner);
                const FVertexID VertexID = OutRawMesh.GetVertexInstanceVertex(VertexInstanceID);
                const float& AxisValue = USplineMeshComponent::GetAxisValue(VertexPositions[VertexID], InSplineMeshComponent->ForwardAxis);
                FTransform SliceTransform = InSplineMeshComponent->CalcSliceTransform(AxisValue);
                FVector TangentY = FVector::CrossProduct(VertexInstanceNormals[VertexInstanceID], VertexInstanceTangents[VertexInstanceID]).GetSafeNormal() * VertexInstanceBinormalSigns[VertexInstanceID];
                VertexInstanceTangents[VertexInstanceID] = SliceTransform.TransformVector(VertexInstanceTangents[VertexInstanceID]);
                TangentY = SliceTransform.TransformVector(TangentY);
                VertexInstanceNormals[VertexInstanceID] = SliceTransform.TransformVector(VertexInstanceNormals[VertexInstanceID]);
                VertexInstanceBinormalSigns[VertexInstanceID] = GetBasisDeterminantSign(VertexInstanceTangents[VertexInstanceID], TangentY, VertexInstanceNormals[VertexInstanceID]);
            }
        }
    }

    // Apply spline deformation for each vertex position
    for (const FVertexID& VertexID : OutRawMesh.Vertices().GetElementIDs())
    {
        float& AxisValue = USplineMeshComponent::GetAxisValue(VertexPositions[VertexID], InSplineMeshComponent->ForwardAxis);
        FTransform SliceTransform = InSplineMeshComponent->CalcSliceTransform(AxisValue);
        AxisValue = 0.0f;
        VertexPositions[VertexID] = SliceTransform.TransformPosition(VertexPositions[VertexID]);
    }
}

void FSimReadyUSDHelper::BakePreviewSurface(UMaterialInterface* MaterialInterface, const FString& MaterialName, const FString& AbsolutePath, const FString& RelativePath,
    const TMap<EMaterialProperty, FLinearColor>& ConstantProperties, TMap<EMaterialProperty, FString>& OutTextureProperties)
{
    TArray<FMeshData> MeshSettings;
    TArray<FMaterialData> MaterialSettings;

    FMeshData MeshSet;
    MeshSet.RawMeshDescription = nullptr;
    MeshSet.TextureCoordinateBox = FBox2D(FVector2D(0.0f, 0.0f), FVector2D(1.0f, 1.0f));
    MeshSet.TextureCoordinateIndex = 0;
    MeshSettings.Add(MeshSet);

    // Determine whether or not certain properties can be rendered
    FMaterialData MatSet;
    FIntPoint TextureSize(2048, 2048);
    MatSet.Material = MaterialInterface;

    bool bTranslucentBake = IsTranslucentBlendMode(MaterialInterface->GetBlendMode());
    if (bTranslucentBake)
    {
        // translucent material can't be baked directly, changing to opaque model to bake
        UMaterialInstanceConstant* MaterialInstOpaque = NewObject<UMaterialInstanceConstant>(GetTransientPackage());
        if (MaterialInterface->IsA<UMaterialInstanceDynamic>())
        {
            // Dynamic instance can't be set to parent directly
            auto MaterialInstDynamic = Cast<UMaterialInstanceDynamic>(MaterialInterface);
            MaterialInstOpaque->SetParentEditorOnly(MaterialInstDynamic->Parent);
            // Copy scalar/vector/texture to InstOpaque
            for (auto& it : MaterialInstDynamic->ScalarParameterValues)
            {
                MaterialInstOpaque->SetScalarParameterValueEditorOnly(it.ParameterInfo.Name, it.ParameterValue);
            }

            for (auto& it : MaterialInstDynamic->VectorParameterValues)
            {
                MaterialInstOpaque->SetVectorParameterValueEditorOnly(it.ParameterInfo.Name, it.ParameterValue);
            }

            for (auto& it : MaterialInstDynamic->TextureParameterValues)
            {
                MaterialInstOpaque->SetTextureParameterValueEditorOnly(it.ParameterInfo.Name, it.ParameterValue);
            }

            for (auto& it : MaterialInstDynamic->FontParameterValues)
            {
                MaterialInstOpaque->SetFontParameterValueEditorOnly(it.ParameterInfo.Name, it.FontValue, it.FontPage);
            }
        }
        else
        {
            MaterialInstOpaque->SetParentEditorOnly(MaterialInterface);
        }
        
        MaterialInstOpaque->BasePropertyOverrides.bOverride_BlendMode = true;
        MaterialInstOpaque->BasePropertyOverrides.BlendMode = EBlendMode::BLEND_Opaque;
        MaterialInstOpaque->PostEditChange();
        MatSet.Material = MaterialInstOpaque;
    }
    
    if (ConstantProperties.Find(MP_BaseColor) == nullptr)
    {
        MatSet.PropertySizes.Add(MP_BaseColor, TextureSize);
    }

    if (ConstantProperties.Find(MP_Normal) == nullptr)
    {
        MatSet.PropertySizes.Add(MP_Normal, TextureSize);
    }

    if (ConstantProperties.Find(MP_Metallic) == nullptr)
    {
        MatSet.PropertySizes.Add(MP_Metallic, TextureSize);
    }

    if (ConstantProperties.Find(MP_Specular) == nullptr)
    {
        MatSet.PropertySizes.Add(MP_Specular, TextureSize);
    }

    if (ConstantProperties.Find(MP_Roughness) == nullptr)
    {
        MatSet.PropertySizes.Add(MP_Roughness, TextureSize);
    }

    if (MaterialInterface->GetBlendMode() == BLEND_Masked && ConstantProperties.Find(MP_OpacityMask) == nullptr)
    {			
        MatSet.PropertySizes.Add(MP_OpacityMask, TextureSize);
    }

    if (ConstantProperties.Find(MP_EmissiveColor) == nullptr)
    {
        MatSet.PropertySizes.Add(MP_EmissiveColor, TextureSize);
    }

    MaterialSettings.Add(MatSet);

    if (bTranslucentBake && ConstantProperties.Find(MP_Opacity) == nullptr)
    {
        // opcaity need to be baked separately
        FMaterialData MatSetOpacity;
        MatSetOpacity.Material = MaterialInterface;
        MatSetOpacity.PropertySizes.Add(MP_Opacity, TextureSize);
        MaterialSettings.Add(MatSetOpacity);

        FMeshData MeshSetOpacity;
        MeshSetOpacity.RawMeshDescription = nullptr;
        MeshSetOpacity.TextureCoordinateBox = FBox2D(FVector2D(0.0f, 0.0f), FVector2D(1.0f, 1.0f));
        MeshSetOpacity.TextureCoordinateIndex = 0;
        MeshSettings.Add(MeshSetOpacity);
    }

    TArray<FMeshData*> MeshSettingPtrs;
    for (int32 SettingsIndex = 0; SettingsIndex < MeshSettings.Num(); ++SettingsIndex)
    {
        MeshSettingPtrs.Add(&MeshSettings[SettingsIndex]);
    }

    TArray<FMaterialData*> MaterialSettingPtrs;
    for (int32 SettingsIndex = 0; SettingsIndex < MaterialSettings.Num(); ++SettingsIndex)
    {
        MaterialSettingPtrs.Add(&MaterialSettings[SettingsIndex]);
    }

    TArray<FBakeOutput> BakeOutputs;
    IMaterialBakingModule& Module = FModuleManager::Get().LoadModuleChecked<IMaterialBakingModule>("MaterialBaking");
    Module.BakeMaterials(MaterialSettingPtrs, MeshSettingPtrs, BakeOutputs);

    for (int32 OutputIndex = 0; OutputIndex < BakeOutputs.Num(); ++OutputIndex)
    {
        auto BakeOutput = BakeOutputs[OutputIndex];
        for (auto Iterator : BakeOutput.PropertySizes)
        {
            EMaterialProperty Property = Iterator.Key;
            FIntPoint BakeTextureSize = Iterator.Value;
            TArray<FColor>& ColorData = BakeOutput.PropertyData.FindChecked(Property);

            const UEnum* PropertyEnum = StaticEnum<EMaterialProperty>();
            FName PropertyName = PropertyEnum->GetNameByValue(Property);
            FString TrimmedPropertyName = PropertyName.ToString();
            TrimmedPropertyName.RemoveFromStart(TEXT("MP_"));

            FString Path = TEXT("PreviewSurfaceTextures") / MaterialName + TEXT("_") + TrimmedPropertyName + TEXT(".png");
            FString OmniPath = AbsolutePath / Path;
            if (FMaterialAttributeDefinitionMap::GetValueType(Property) == MCT_Float)
            {
                TArray<uint8> GrayColorData;
                GrayColorData.AddUninitialized(ColorData.Num());
                for (int32 ColorIndex = 0; ColorIndex < ColorData.Num(); ++ColorIndex)
                {
                    GrayColorData[ColorIndex] = ColorData[ColorIndex].R;
                }

                if (FSimReadyAssetExportHelper::ExportRawDataToPath(GrayColorData.GetData(), GrayColorData.Num(), OmniPath, BakeTextureSize.X, BakeTextureSize.Y, ERGBFormat::Gray, 8))
                {
                    OutTextureProperties.Add(Property, RelativePath / Path);
                }
            }
            else
            {
                // The alpha of baked color is 0, set to 255 to save to png
                for (auto& Color : ColorData)
                {
                    Color.A = 255;
                }

                if (FSimReadyAssetExportHelper::ExportRawDataToPath(ColorData.GetData(), ColorData.Num() * sizeof(FColor), OmniPath, BakeTextureSize.X, BakeTextureSize.Y, ERGBFormat::BGRA, 8))
                {
                    OutTextureProperties.Add(Property, RelativePath / Path);
                }
            }
        }
    }
}

void BindMaxTexCoordIndex(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& PrimPath, const int32 NumTexCoord)
{
    auto UsdMaterial = pxr::UsdShadeMaterial::Get(USDStage, PrimPath);
    // UsdMaterial might be failed
    if (UsdMaterial)
    {
        pxr::UsdShadeConnectableAPI Source;
        pxr::TfToken SourceName;
        pxr::UsdShadeAttributeType SourceType;
        UsdMaterial.GetSurfaceOutput(USDTokens.mdl).GetConnectedSource(&Source, &SourceName, &SourceType);
        auto MdlSurfaceShader = pxr::UsdShadeShader(Source);

        auto MaxTexCoordIndexInput = MdlSurfaceShader.GetInput(pxr::TfToken("MaxTexCoordIndex"));
        if (!MaxTexCoordIndexInput)
        {
#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
            MaxTexCoordIndexInput = MdlSurfaceShader.CreateInput(pxr::TfToken("MaxTexCoordIndex"), pxr::SdfValueTypeNames->Int);
#else // Create input to UsdMaterial and connect to shader
            MaxTexCoordIndexInput = UsdMaterial.CreateInput(pxr::TfToken("MaxTexCoordIndex"), pxr::SdfValueTypeNames->Int);
            MdlSurfaceShader.CreateInput(pxr::TfToken("MaxTexCoordIndex"), pxr::SdfValueTypeNames->Int).ConnectToSource(MaxTexCoordIndexInput);
#endif

            MaxTexCoordIndexInput.Set(FMath::Max(NumTexCoord - 1, 0));
        }
    }
}

void BindVertexColorIndex(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& PrimPath, const int32 VertexColorCoordinateIndex)
{
#if 0 // Workaround with UVSet
    auto UsdMaterial = pxr::UsdShadeMaterial::Get(USDStage, PrimPath);
    // UsdMaterial might be failed
    if (UsdMaterial)
    {
        pxr::UsdShadeConnectableAPI Source;
        pxr::TfToken SourceName;
        pxr::UsdShadeAttributeType SourceType;
        UsdMaterial.GetSurfaceOutput(USDTokens.mdl).GetConnectedSource(&Source, &SourceName, &SourceType);
        auto MdlSurfaceShader = pxr::UsdShadeShader(Source);

        // check if there's already vertex color index bound
        auto VertexColorCoordinateIndexInput = MdlSurfaceShader.GetInput(pxr::TfToken("VertexColorCoordinateIndex"));
        if (!VertexColorCoordinateIndexInput)
        {
#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
            VertexColorCoordinateIndexInput = MdlSurfaceShader.CreateInput(pxr::TfToken("VertexColorCoordinateIndex"), pxr::SdfValueTypeNames->Int);
#else // Create input to UsdMaterial and connect to shader
            VertexColorCoordinateIndexInput = UsdMaterial.CreateInput(pxr::TfToken("VertexColorCoordinateIndex"), pxr::SdfValueTypeNames->Int);
            MdlSurfaceShader.CreateInput(pxr::TfToken("VertexColorCoordinateIndex"), pxr::SdfValueTypeNames->Int).ConnectToSource(VertexColorCoordinateIndexInput);
#endif

            VertexColorCoordinateIndexInput.Set(VertexColorCoordinateIndex);
        }

        // Also for max texcoord index
        auto MaxTexCoordIndexInput = MdlSurfaceShader.GetInput(pxr::TfToken("MaxTexCoordIndex"));
        if (!MaxTexCoordIndexInput)
        {
#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
            MaxTexCoordIndexInput = MdlSurfaceShader.CreateInput(pxr::TfToken("MaxTexCoordIndex"), pxr::SdfValueTypeNames->Int);
#else // Create input to UsdMaterial and connect to shader
            MaxTexCoordIndexInput = UsdMaterial.CreateInput(pxr::TfToken("MaxTexCoordIndex"), pxr::SdfValueTypeNames->Int);
            MdlSurfaceShader.CreateInput(pxr::TfToken("MaxTexCoordIndex"), pxr::SdfValueTypeNames->Int).ConnectToSource(MaxTexCoordIndexInput);
#endif

            MaxTexCoordIndexInput.Set(FMath::Max(VertexColorCoordinateIndex - 1, 0));
        }
    }
#endif
}

void BindNumTexCoord(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& PrimPath, const FMeshDescription& MeshDescription, const FSimReadyExportMaterialResult& MaterialResult)
{
    const int32 NumTexCoord = MeshDescription.VertexInstanceAttributes().GetAttributeIndexCount<FVector2D>(MeshAttribute::VertexInstance::TextureCoordinate);
    if (MaterialResult.IsVertexColorUsed())
    {
        BindVertexColorIndex(USDStage, PrimPath, NumTexCoord);
    }
    else if(MaterialResult.IsMaxTexCoordinateUsed())
    {
        BindMaxTexCoordIndex(USDStage, PrimPath, NumTexCoord);
    }
}

void BindNumTexCoord(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& PrimPath, UStaticMesh* StaticMesh, int32 LODIndex, const FSimReadyExportMaterialResult& MaterialResult)
{
    const int32 NumTexCoord = StaticMesh->RenderData->LODResources[LODIndex].VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
    if (MaterialResult.IsVertexColorUsed())
    {
        if (NumTexCoord > 2)
        {
            FSimReadyAssetExportHelper::LogMessage(StaticMesh, FSimReadyExportMessageSeverity::Warning, TEXT("Meshes with more than 2 UV channels with materials that use Vertex Color won't have correct materials in Omniverse Kit."));
        }

        BindVertexColorIndex(USDStage, PrimPath, NumTexCoord);
    }
    else if (MaterialResult.IsMaxTexCoordinateUsed())
    {
        BindMaxTexCoordIndex(USDStage, PrimPath, NumTexCoord);
    }
}

void BindInstanceData(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& PrimPath, int32 NumInstances, const FSimReadyExportMaterialResult& MaterialResult)
{
    if (!MaterialResult.IsPerInstanceDataUsed())
    {
        return;
    }

    auto UsdMaterial = pxr::UsdShadeMaterial::Get(USDStage, PrimPath);
    // UsdMaterial might be failed
    if (UsdMaterial)
    {
        pxr::UsdShadeConnectableAPI Source;
        pxr::TfToken SourceName;
        pxr::UsdShadeAttributeType SourceType;
        UsdMaterial.GetSurfaceOutput(USDTokens.mdl).GetConnectedSource(&Source, &SourceName, &SourceType);
        auto MdlSurfaceShader = pxr::UsdShadeShader(Source);

        // check if there's already instances data bound
        auto NumberInstancesInput = MdlSurfaceShader.GetInput(pxr::TfToken("NumberInstances"));
        if (!NumberInstancesInput)
        {
#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
            NumberInstancesInput = MdlSurfaceShader.CreateInput(pxr::TfToken("NumberInstances"), pxr::SdfValueTypeNames->Int);
#else // Create input to UsdMaterial and connect to shader
            NumberInstancesInput = UsdMaterial.CreateInput(pxr::TfToken("NumberInstances"), pxr::SdfValueTypeNames->Int);
            MdlSurfaceShader.CreateInput(pxr::TfToken("NumberInstances"), pxr::SdfValueTypeNames->Int).ConnectToSource(NumberInstancesInput);
#endif

            NumberInstancesInput.Set(NumInstances);
        }
    }
}

void BindAOMask(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& PrimPath, int32 LightMapCoordinateIndex, const FVector2D& CoordinateScale, const FVector2D& CoordinateBias, const FString& RelativeTextureFileName)
{
    auto UsdMaterial = pxr::UsdShadeMaterial::Get(USDStage, PrimPath);
    // UsdMaterial might be failed
    if (UsdMaterial)
    {
        pxr::UsdShadeConnectableAPI Source;
        pxr::TfToken SourceName;
        pxr::UsdShadeAttributeType SourceType;
        UsdMaterial.GetSurfaceOutput(USDTokens.mdl).GetConnectedSource(&Source, &SourceName, &SourceType);
        auto MdlSurfaceShader = pxr::UsdShadeShader(Source);
#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
        auto AOMaterialMaskTextureInput = MdlSurfaceShader.CreateInput(pxr::TfToken("AOMaterialMaskTexture"), pxr::SdfValueTypeNames->Asset);
        auto LightMapCoordinateIndexInput = MdlSurfaceShader.CreateInput(pxr::TfToken("LightMapCoordinateIndex"), pxr::SdfValueTypeNames->Int);
        auto LightmapCoordinateScaleInput = MdlSurfaceShader.CreateInput(pxr::TfToken("LightmapCoordinateScale"), pxr::SdfValueTypeNames->Float2);
        auto LightmapCoordinateBiasInput = MdlSurfaceShader.CreateInput(pxr::TfToken("LightmapCoordinateBias"), pxr::SdfValueTypeNames->Float2);
#else // Create input to UsdMaterial and connect to shader
        auto AOMaterialMaskTextureInput = UsdMaterial.CreateInput(pxr::TfToken("AOMaterialMaskTexture"), pxr::SdfValueTypeNames->Asset);
        auto LightMapCoordinateIndexInput = UsdMaterial.CreateInput(pxr::TfToken("LightMapCoordinateIndex"), pxr::SdfValueTypeNames->Int);
        auto LightmapCoordinateScaleInput = UsdMaterial.CreateInput(pxr::TfToken("LightmapCoordinateScale"), pxr::SdfValueTypeNames->Float2);
        auto LightmapCoordinateBiasInput = UsdMaterial.CreateInput(pxr::TfToken("LightmapCoordinateBias"), pxr::SdfValueTypeNames->Float2);

        MdlSurfaceShader.CreateInput(pxr::TfToken("AOMaterialMaskTexture"), pxr::SdfValueTypeNames->Asset).ConnectToSource(AOMaterialMaskTextureInput);
        MdlSurfaceShader.CreateInput(pxr::TfToken("LightMapCoordinateIndex"), pxr::SdfValueTypeNames->Int).ConnectToSource(LightMapCoordinateIndexInput);
        MdlSurfaceShader.CreateInput(pxr::TfToken("LightmapCoordinateScale"), pxr::SdfValueTypeNames->Float2).ConnectToSource(LightmapCoordinateScaleInput);
        MdlSurfaceShader.CreateInput(pxr::TfToken("LightmapCoordinateBias"), pxr::SdfValueTypeNames->Float2).ConnectToSource(LightmapCoordinateBiasInput);
#endif

        LightMapCoordinateIndexInput.Set(LightMapCoordinateIndex);
        LightmapCoordinateScaleInput.Set(USDConvert(CoordinateScale));
        LightmapCoordinateBiasInput.Set(USDConvert(CoordinateBias));

        AOMaterialMaskTextureInput.Set(pxr::SdfAssetPath(TCHAR_TO_UTF8(*RelativeTextureFileName)));
        AOMaterialMaskTextureInput.GetAttr().SetColorSpace(USDTokens.raw);
    }
}

void CheckUsdNameByMaxTexCoordIndex(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& PrimPath, const int32 NumTexCoord, FString& UsdMaterialName)
{
    auto UsdMaterial = pxr::UsdShadeMaterial::Get(USDStage, PrimPath);
    // UsdMaterial might be failed
    if (UsdMaterial)
    {
        pxr::UsdShadeConnectableAPI Source;
        pxr::TfToken SourceName;
        pxr::UsdShadeAttributeType SourceType;
        UsdMaterial.GetSurfaceOutput(USDTokens.mdl).GetConnectedSource(&Source, &SourceName, &SourceType);
        auto MdlSurfaceShader = pxr::UsdShadeShader(Source);

        // check if there's already max tex coord index bound
        auto MaxTexCoordIndexInput = MdlSurfaceShader.GetInput(pxr::TfToken("MaxTexCoordIndex"));
        if (MaxTexCoordIndexInput)
        {
            if (MaxTexCoordIndexInput.HasConnectedSource())
            {
                pxr::UsdShadeConnectableAPI InputSource;
                pxr::TfToken InputSourceName;
                pxr::UsdShadeAttributeType InputSourceType;
                MaxTexCoordIndexInput.GetConnectedSource(&InputSource, &InputSourceName, &InputSourceType);

                MaxTexCoordIndexInput = InputSource.GetInput(pxr::TfToken("MaxTexCoordIndex"));
            }

            int32 MaterialMaxTexCoordIndex = INDEX_NONE;
            if (MaxTexCoordIndexInput)
            {
                MaxTexCoordIndexInput.Get<int32>(&MaterialMaxTexCoordIndex);
            }

            if (MaterialMaxTexCoordIndex != INDEX_NONE && NumTexCoord != MaterialMaxTexCoordIndex + 1)
            {
                // if the current TexCoordIndex bound is different from the exist, we need to create new usd material binding here			
                UsdMaterialName += FString::FromInt(NumTexCoord);
            }
        }
    }
}

void CheckUsdNameByVertexColorIndex(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& PrimPath, const int32 VertexColorCoordinateIndex, FString& UsdMaterialName)
{
#if 0 // Workaround with UVSet
    auto UsdMaterial = pxr::UsdShadeMaterial::Get(USDStage, PrimPath);
    // UsdMaterial might be failed
    if (UsdMaterial)
    {
        pxr::UsdShadeConnectableAPI Source;
        pxr::TfToken SourceName;
        pxr::UsdShadeAttributeType SourceType;
        UsdMaterial.GetSurfaceOutput(USDTokens.mdl).GetConnectedSource(&Source, &SourceName, &SourceType);
        auto MdlSurfaceShader = pxr::UsdShadeShader(Source);

        // check if there's already vertex color index bound
        auto VertexColorInput = MdlSurfaceShader.GetInput(pxr::TfToken("VertexColorCoordinateIndex"));
        if (VertexColorInput)
        {
            if (VertexColorInput.HasConnectedSource())
            {
                pxr::UsdShadeConnectableAPI InputSource;
                pxr::TfToken InputSourceName;
                pxr::UsdShadeAttributeType InputSourceType;
                VertexColorInput.GetConnectedSource(&InputSource, &InputSourceName, &InputSourceType);

                VertexColorInput = InputSource.GetInput(pxr::TfToken("VertexColorCoordinateIndex"));
            }

            int32 MaterialVertexColorCoordinateIndex = INDEX_NONE;
            if (VertexColorInput)
            {
                VertexColorInput.Get<int32>(&MaterialVertexColorCoordinateIndex);
            }

            if (MaterialVertexColorCoordinateIndex != INDEX_NONE && VertexColorCoordinateIndex != MaterialVertexColorCoordinateIndex)
            {
                // if the current vertex color bound is different from the exist, we need to create new usd material binding here			
                UsdMaterialName += FString::FromInt(VertexColorCoordinateIndex);
            }
        }
    }
#endif
}

void CheckUsdNameByNumTexCoord(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& PrimPath, const FMeshDescription& MeshDescription, const FSimReadyExportMaterialResult& MaterialResult, FString& UsdMaterialName)
{
    const int32 NumTexCoord = MeshDescription.VertexInstanceAttributes().GetAttributeIndexCount<FVector2D>(MeshAttribute::VertexInstance::TextureCoordinate);
    if (MaterialResult.IsVertexColorUsed())
    {
        CheckUsdNameByVertexColorIndex(USDStage, PrimPath, NumTexCoord, UsdMaterialName);
    }
    else if (MaterialResult.IsMaxTexCoordinateUsed())
    {
        CheckUsdNameByMaxTexCoordIndex(USDStage, PrimPath, NumTexCoord, UsdMaterialName);
    }
}

void CheckUsdNameByNumTexCoord(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& PrimPath, const FStaticMeshLODResources& LODModel, const FSimReadyExportMaterialResult& MaterialResult, FString& UsdMaterialName)
{
    const int32 NumTexCoord = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
    if (MaterialResult.IsVertexColorUsed())
    {
        CheckUsdNameByVertexColorIndex(USDStage, PrimPath, NumTexCoord, UsdMaterialName);
    }
    else if (MaterialResult.IsMaxTexCoordinateUsed())
    {
        CheckUsdNameByMaxTexCoordIndex(USDStage, PrimPath, NumTexCoord, UsdMaterialName);
    }
}

void CheckUsdNameByInstanceData(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& PrimPath, int32 NumInstances, const FSimReadyExportMaterialResult& MaterialResult, FString& UsdMaterialName)
{
    if (!MaterialResult.IsPerInstanceDataUsed())
    {
        return;
    }

    auto UsdMaterial = pxr::UsdShadeMaterial::Get(USDStage, PrimPath);
    // UsdMaterial might be failed
    if (UsdMaterial)
    {
        pxr::UsdShadeConnectableAPI Source;
        pxr::TfToken SourceName;
        pxr::UsdShadeAttributeType SourceType;
        UsdMaterial.GetSurfaceOutput(USDTokens.mdl).GetConnectedSource(&Source, &SourceName, &SourceType);
        auto MdlSurfaceShader = pxr::UsdShadeShader(Source);

        // check if there's already number instances bound
        auto NumberInstancesInput = MdlSurfaceShader.GetInput(pxr::TfToken("NumberInstances"));
        if (NumberInstancesInput)
        {
            if (NumberInstancesInput.HasConnectedSource())
            {
                pxr::UsdShadeConnectableAPI InputSource;
                pxr::TfToken InputSourceName;
                pxr::UsdShadeAttributeType InputSourceType;
                NumberInstancesInput.GetConnectedSource(&InputSource, &InputSourceName, &InputSourceType);

                NumberInstancesInput = InputSource.GetInput(pxr::TfToken("NumberInstances"));
            }

            int32 NumberInstancesInMaterial = INDEX_NONE;
            if (NumberInstancesInput)
            {
                NumberInstancesInput.Get<int32>(&NumberInstancesInMaterial);
            }

            if (NumberInstancesInMaterial != INDEX_NONE && NumInstances != NumberInstancesInMaterial)
            {
                // if the current number instances bound is different from the exist, we need to create new usd material binding here			
                UsdMaterialName += FString::FromInt(NumInstances);
            }
        }
    }
}

void FSimReadyUSDHelper::ExportAndBindMaterial(const pxr::UsdStageRefPtr& USDStage, 
    UMaterialInterface* MaterialInterface,
    const FNamedParameterTimeSamples* ParameterTimeSamples, 
    const pxr::UsdPrim& Prim,
    const FSimReadyExportSettings& ExportSettings, 
    const FString& MDLBaseAbsolutePath, 
    const FString& MDLBaseRelativePath, 
    TMap<FString, FString>& ExportedMDLs, 
    const FString& CustomMaterialName, 
    const FString& CustomUsdMaterialName,
    FExportMaterialPreCallback PreExportCallback,
    FExportMaterialPostCallback PostExportCallback)
{
    ExportShadowFromMaterial(pxr::UsdGeomXformable(Prim), MaterialInterface);

    if (MaterialInterface && (ExportSettings.bMDL || ExportSettings.bPreviewSurface))
    {
        bool bLandscapeInstance = MaterialInterface->IsA<ULandscapeMaterialInstanceConstant>();

        FString MDLName = CustomMaterialName.IsEmpty() ? MaterialInterface->GetName() : CustomMaterialName;
        FString FullMaterialName = MaterialInterface->GetFullName();
        FString MDLAbsolutePath = MDLBaseAbsolutePath / MDLName + ".mdl";
        FString MDLRelativePath = MDLBaseRelativePath / MDLName + ".mdl";
        FString UsdMaterialName = MDLName;
        FSimReadyExportMaterialResult MaterialResult;
        auto LocalBaseMaterial = USimReadyMDL::GetLocalBaseMaterial(MaterialInterface);
        bool bOmniMaterial = (LocalBaseMaterial != nullptr);
        int32 MappedMaterialIndex = INDEX_NONE;
        if (!bOmniMaterial)
        {
            MappedMaterialIndex = FSimReadyMaterialReparentUtility::FindOmniMaterial(MaterialInterface, MDLName, ESimReadyReparentUsage::Export);
            bOmniMaterial = MappedMaterialIndex != INDEX_NONE;
        }
        bool bIsMaterialExported = false;
        bool bModular = ExportSettings.bModular && !ExportSettings.bForceShared;
        FString ModuleName = MDLName;

        if (bOmniMaterial)
        {
            if (LocalBaseMaterial)
            {
                MDLName = LocalBaseMaterial->GetName();
            }

            if (USimReadyMDL::GetMDLModuleByMaterialName(MDLName, ModuleName))
            {
                MDLRelativePath = ModuleName + TEXT(".mdl");			
            }
            
            if (!ExportedMDLs.Contains(FullMaterialName))
            {
                if (FSimReadyReferenceCollector::Get().FindDependency(MaterialInterface->GetOuter()->GetFName()))
                {
                    FString Text = FString::Printf(TEXT("Exporting Object %s..."), *MaterialInterface->GetName());
                    FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
                }

                if (FSimReadySlowTask::Get().IsProgressCancelled())
                {
                    return;
                }

                bool bShouldCopyTemplate = ExportSettings.bMDL && ExportSettings.MaterialSettings.bCopyTemplate && MappedMaterialIndex == INDEX_NONE;
                FString DestTempPath = MDLBaseAbsolutePath;
                if (bShouldCopyTemplate && !ExportSettings.MaterialSettings.DestTemplatePath.IsEmpty())
                {
                    DestTempPath = ExportSettings.MaterialSettings.DestTemplatePath;
                }

                //// only copying templates if destination was same target (local/server) as the mdl, or it failed.
                if (bShouldCopyTemplate)
                {
                    MDLRelativePath = MDLBaseRelativePath / ModuleName + TEXT(".mdl");

                    TArray<FString> ImportModules = FSimReadyAssetExportHelper::GetImportList(ModuleName);
                    ImportModules.Insert(ModuleName, 0);

                    for (auto Name : ImportModules)
                    {
                        // local templates are copied to server
                        FSimReadyAssetExportHelper::CopyTemplate(Name.StartsWith(TEXT("OmniSurface/")) ? FSimReadyMaterialTemplateType::Surface : FSimReadyMaterialTemplateType::Base, DestTempPath, Name + TEXT(".mdl"));
                    }
                }

                ExportedMDLs.Add(FullMaterialName, MDLAbsolutePath);
            }

            // No need to export Omni Materials
            PreExportCallback = nullptr;
            PostExportCallback = nullptr;
            bIsMaterialExported = true;
        }
        else
        {
            if (!ExportedMDLs.Contains(FullMaterialName))
            {
                if (FSimReadyReferenceCollector::Get().FindDependency(bLandscapeInstance ? Cast<ULandscapeMaterialInstanceConstant>(MaterialInterface)->Parent->GetOuter()->GetFName() : MaterialInterface->GetOuter()->GetFName()))
                {
                    FString Text = FString::Printf(TEXT("Exporting Object %s..."), *MaterialInterface->GetName());
                    FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
                }

                if (FSimReadySlowTask::Get().IsProgressCancelled())
                {
                    return;
                }

                FString RenameMDLAbsolutePath = MDLAbsolutePath;
                while (ExportedMDLs.FindKey(RenameMDLAbsolutePath))
                {
                    RenameMDLAbsolutePath = MDLBaseAbsolutePath / MDLName + TEXT("_") + FString::FromInt(SuffixIndex++) + ".mdl";
                }

                MDLAbsolutePath = RenameMDLAbsolutePath;
                MDLRelativePath = MDLBaseRelativePath / FPaths::GetCleanFilename(MDLAbsolutePath);	

                if (ExportSettings.bMDL)
                {
                    /*MaterialName might be changed to real mdl file name*/
                    if (FSimReadyAssetExportHelper::ExportMaterialToPath(MaterialInterface, MDLAbsolutePath, MDLName, true, bModular, !ExportSettings.bUpYAxis, bLandscapeInstance, ExportSettings.MaterialSettings, &MaterialResult, 
                    [&](const UTexture* Texture)
                    {
                        if (FSimReadyReferenceCollector::Get().FindDependency(Texture->GetOuter()->GetFName()))
                        {
                            FString Text = FString::Printf(TEXT("Exporting Object %s..."), *Texture->GetName());
                            FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
                        }
                    }))
                    {
                        // Get original absolute name as usd material name, NOTE: material name can't be used because it will be changed to the parent name.
                        FString UniqueUsdMaterialName = FPaths::GetBaseFilename(MDLAbsolutePath);
                        // Also cache USD name in the same array, Usd name should be unique here.
                        while (ExportedMDLs.Contains(UniqueUsdMaterialName))
                        {
                            UniqueUsdMaterialName = UsdMaterialName + TEXT("_") + FString::FromInt(SuffixIndex++);
                        }
                        ExportedMDLs.Add(UniqueUsdMaterialName, FullMaterialName);
                        UsdMaterialName = UniqueUsdMaterialName;

                        // Save MDL Path
                        MDLAbsolutePath = MDLBaseAbsolutePath / MDLName + ".mdl";
                        MDLRelativePath = MDLBaseRelativePath / MDLName + ".mdl";
                        ExportedMDLs.Add(FullMaterialName, MDLAbsolutePath);
                    
                        bIsMaterialExported = true;
                    }
                }
                else // if only export preview surface
                {
                    MDLName = FPaths::GetBaseFilename(MDLAbsolutePath);
                    UsdMaterialName = MDLName;

                    ExportedMDLs.Add(FullMaterialName, MDLAbsolutePath);
                    ExportedMDLs.Add(UsdMaterialName, FullMaterialName);
                    bIsMaterialExported = true;
                }
            }
            else
            {
                MDLAbsolutePath = ExportedMDLs[FullMaterialName];
                MDLName = FPaths::GetBaseFilename(MDLAbsolutePath);
                MDLRelativePath = MDLBaseRelativePath / MDLName + ".mdl";
                auto CachedUsdMaterialName = ExportedMDLs.FindKey(FullMaterialName);
                UsdMaterialName = CachedUsdMaterialName ? *CachedUsdMaterialName : MDLName;

                if (ExportSettings.bMDL)
                {
                    // fast get compiled result
                    FSimReadyAssetExportHelper::ExportMaterialToPath(MaterialInterface, MDLAbsolutePath, MDLName, false, bModular, !ExportSettings.bUpYAxis, bLandscapeInstance, ExportSettings.MaterialSettings, &MaterialResult);
                }
                bIsMaterialExported = true;
            }
        }

        if (bIsMaterialExported)
        {
            if (!CustomUsdMaterialName.IsEmpty())
            {
                UsdMaterialName = CustomUsdMaterialName;
            }

            pxr::SdfPath ScopePrimPath;
            if (ExportSettings.bModular)
            {
                ScopePrimPath = Prim.GetPath().GetParentPath();
            }
            else
            {
                ScopePrimPath = USDStage->GetDefaultPrim().GetPath();
            }
            ScopePrimPath = ScopePrimPath.AppendElementString(pxr::UsdUtilsGetMaterialsScopeName().GetString());
    
            bool MaterialOverSublayer = ExportSettings.bMaterialOverSublayer && ExportSettings.bMDL;
            
            FString UsdMaterialFileName = UsdMaterialName + GetUSDExtension(ExportSettings.bAsciiFormat);
            FString AbsoluteUsdMaterialPath = MDLBaseAbsolutePath / UsdMaterialFileName;
            FString RelativeUsdMaterialPath = MDLBaseRelativePath / UsdMaterialFileName;
            auto MaterialStage = MaterialOverSublayer ? CreateUsdMaterialStage(AbsoluteUsdMaterialPath, ExportSettings) : USDStage;
            if (!MaterialStage)
            {
                return;
            }

            if (PreExportCallback)
            {
                FString PreUsdMaterialName = UsdMaterialName;
                auto MaterialPrimPath = MaterialOverSublayer ? pxr::SdfPath::AbsoluteRootPath().AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*UsdMaterialName))) : ScopePrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*UsdMaterialName)));
                PreExportCallback(MaterialStage, MaterialPrimPath, MaterialResult, UsdMaterialName);
                if (MaterialOverSublayer && UsdMaterialName != PreUsdMaterialName)
                {
                    // Need recreate usd
                    UsdMaterialFileName = UsdMaterialName + GetUSDExtension(ExportSettings.bAsciiFormat);
                    AbsoluteUsdMaterialPath = MDLBaseAbsolutePath / UsdMaterialFileName;
                    RelativeUsdMaterialPath = MDLBaseRelativePath / UsdMaterialFileName;
                    MaterialStage = CreateUsdMaterialStage(AbsoluteUsdMaterialPath, ExportSettings);
                }
            }

            if (MaterialOverSublayer)
            {
                MDLRelativePath = (bOmniMaterial ? ModuleName : MDLName) + TEXT(".mdl");
            }

            pxr::UsdShadeMaterial UsdMaterial = MaterialOverSublayer ? DefineUsdMaterial(MaterialStage, UsdMaterialName) : BindMaterial(MaterialStage, Prim, ScopePrimPath, UsdMaterialName);
            auto UsdMaterialPath = UsdMaterial.GetPath();

            if (ExportSettings.bMDL)
            {
                auto MdlShaderPrimPath = UsdMaterialPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*MDLName)));
                auto MdlUsdShader = pxr::UsdShadeShader::Get(MaterialStage, MdlShaderPrimPath);
                if (!MdlUsdShader)
                {
                    CreateMDLSchema(MaterialStage, UsdMaterialPath, true, MDLName, MDLRelativePath);

                    if (bOmniMaterial)
                    {
                        if (MaterialInterface->IsA<UMaterialInstance>())
                        {
                            FString CustomPath = FPaths::GetPath(MDLAbsolutePath) / (bModular ? MaterialInterface->GetName() : TEXT("Textures"));
                            MdlUsdShader = pxr::UsdShadeShader::Get(MaterialStage, MdlShaderPrimPath);
                            FUSDExporter::ExportMdlSchemaParameters(MaterialStage, *Cast<UMaterialInstance>(MaterialInterface), MdlUsdShader, ExportSettings.MaterialSettings.TextureSettings, CustomPath, true);
                        }
                    }
                    else
                    {
                        if (ParameterTimeSamples)
                        {
                            ExportParameterTimeSamplesToMDLSchema(MaterialStage, UsdMaterialPath, MDLName, MaterialInterface, *ParameterTimeSamples, MaterialResult, MDLAbsolutePath, bModular, ExportSettings.MaterialSettings.TextureSettings);
                        }

                        if (MaterialInterface->IsA<UMaterialInstance>())
                        {
                            ExportInstanceParametersToMDLSchema(MaterialStage, UsdMaterialPath, MDLName, Cast<UMaterialInstance>(MaterialInterface), MaterialResult, MDLAbsolutePath, bModular, ExportSettings.MaterialSettings.TextureSettings);
                        }
                    }
                }

                if (PostExportCallback)
                {
                    auto MaterialPrimPath = MaterialOverSublayer ? MaterialStage->GetDefaultPrim().GetPath() : ScopePrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*UsdMaterialName)));
                    PostExportCallback(MaterialStage, MaterialPrimPath, MDLName, MaterialResult, UsdMaterialName);
                }

                if (MaterialOverSublayer)
                {
                    auto RefUsdMaterial = BindMaterial(USDStage, Prim, ScopePrimPath, UsdMaterialName);
                    // recover usd material path, it could be useful for preview surface
                    UsdMaterialPath = RefUsdMaterial.GetPath();
                    RefUsdMaterial.GetPrim().GetReferences().AddReference(TCHAR_TO_UTF8(*RelativeUsdMaterialPath));
                }
            }

            if (MaterialOverSublayer)
            {
                MaterialStage->Save();
            }

            if (ExportSettings.bPreviewSurface && FApp::CanEverRender())
            {
                auto PSShaderPrimPath = UsdMaterialPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*(UsdMaterialName + "PreviewSurface"))));
                auto PSUsdShader = pxr::UsdShadeShader::Get(USDStage, PSShaderPrimPath);
                if (!PSUsdShader)
                {
                    TMap<EMaterialProperty, FString> TextureProperties;
                    // Get ConstantProperties for MaterialInterface, cache ignored.
                    FSimReadyAssetExportHelper::ExportMaterialToPath(
                        MaterialInterface, MDLAbsolutePath, MDLName, false, true, !ExportSettings.bUpYAxis, bLandscapeInstance, ExportSettings.MaterialSettings, &MaterialResult);
                    BakePreviewSurface(MaterialInterface, UsdMaterialName, MDLBaseAbsolutePath, MDLBaseRelativePath, MaterialResult.ConstantProperties, TextureProperties);
                    // hack: saving clip value to shading model property
                    MaterialResult.ConstantProperties.Add(MP_ShadingModel, FLinearColor(MaterialInterface->GetOpacityMaskClipValue(), 0, 0, 0));
                    CreatePSSchema(USDStage, UsdMaterialPath, UsdMaterialName, TextureProperties, MaterialResult.ConstantProperties);
                }
            }
        }
    }
}

// ---------- Methods from UE4 TextRenderComponent.cpp ----------
struct FTextIterator
{
    const TCHAR* SourceString;
    const TCHAR* CurrentPosition;

    FTextIterator(const TCHAR* InSourceString)
        : SourceString(InSourceString)
        , CurrentPosition(InSourceString)
    {
        check(InSourceString);
    }

    bool NextLine()
    {
        check(CurrentPosition);
        return (CurrentPosition[0] != '\0');
    }

    bool NextCharacterInLine(TCHAR& Ch)
    {
        bool bRet = false;
        check(CurrentPosition);

        if (CurrentPosition[0] == '\0')
        {
            // Leave current position on the null terminator
        }
        else if (CurrentPosition[0] == '<' && CurrentPosition[1] == 'b' && CurrentPosition[2] == 'r' && CurrentPosition[3] == '>')
        {
            CurrentPosition += 4;
        }
        else if (CurrentPosition[0] == '\n')
        {
            ++CurrentPosition;
        }
        else
        {
            Ch = *CurrentPosition;
            CurrentPosition++;
            bRet = true;
        }

        return bRet;
    }

    bool Peek(TCHAR& Ch)
    {
        check(CurrentPosition);
        if (CurrentPosition[0] == '\0' || (CurrentPosition[0] == '<' && CurrentPosition[1] == 'b' && CurrentPosition[2] == 'r' && CurrentPosition[3] == '>') || (CurrentPosition[0] == '\n'))
        {
            return false;
        }
        else
        {
            Ch = CurrentPosition[0];
            return true;
        }
    }
};

FVector2D ComputeTextSize(FTextIterator It, class UFont* Font,
    float XScale, float YScale, float HorizSpacingAdjust, float VertSpacingAdjust)
{
    FVector2D Ret(0.f, 0.f);

    if (!Font)
    {
        return Ret;
    }

    const float CharIncrement = ((float)Font->Kerning + HorizSpacingAdjust) * XScale;

    float LineX = 0.f;

    TCHAR Ch = 0;
    while (It.NextCharacterInLine(Ch))
    {
        Ch = Font->RemapChar(Ch);

        if (!Font->Characters.IsValidIndex(Ch))
        {
            continue;
        }

        FFontCharacter& Char = Font->Characters[Ch];

        if (!Font->Textures.IsValidIndex(Char.TextureIndex))
        {
            continue;
        }

        UTexture2D* Tex = Font->Textures[Char.TextureIndex];

        if (Tex)
        {
            FIntPoint ImportedTextureSize = Tex->GetImportedSize();
            FVector2D InvTextureSize(1.0f / (float)ImportedTextureSize.X, 1.0f / (float)ImportedTextureSize.Y);

            const float X = LineX;
            const float Y = Char.VerticalOffset * YScale;
            float SizeX = Char.USize * XScale;
            const float SizeY = (Char.VSize + VertSpacingAdjust) * YScale;
            const float U = Char.StartU * InvTextureSize.X;
            const float V = Char.StartV * InvTextureSize.Y;
            const float SizeU = Char.USize * InvTextureSize.X;
            const float SizeV = Char.VSize * InvTextureSize.Y;

            const float Right = X + SizeX;
            const float Bottom = Y + SizeY;

            Ret.X = FMath::Max(Ret.X, Right);
            Ret.Y = FMath::Max(Ret.Y, Bottom);

            // if we have another non-whitespace character to render, add the font's kerning.
            TCHAR NextCh = 0;
            if (It.Peek(NextCh) && !FChar::IsWhitespace(NextCh))
            {
                SizeX += CharIncrement;
            }

            LineX += SizeX;
        }
    }

    return Ret;
}

// compute the left top depending on the alignment
static float ComputeHorizontalAlignmentOffset(FVector2D Size, EHorizTextAligment HorizontalAlignment)
{
    float Ret = 0.f;

    switch (HorizontalAlignment)
    {
    case EHTA_Left:
    {
        // X is already 0
        break;
    }

    case EHTA_Center:
    {
        Ret = -Size.X * 0.5f;
        break;
    }

    case EHTA_Right:
    {
        Ret = -Size.X;
        break;
    }

    default:
    {
        // internal error
        check(0);
    }
    }

    return Ret;
}

float ComputeVerticalAlignmentOffset(float SizeY, EVerticalTextAligment VerticalAlignment, float LegacyVerticalOffset)
{
    switch (VerticalAlignment)
    {
    case EVRTA_QuadTop:
    {
        return LegacyVerticalOffset;
    }
    case EVRTA_TextBottom:
    {
        return -SizeY;
    }
    case EVRTA_TextTop:
    {
        return 0.f;
    }
    case EVRTA_TextCenter:
    {
        return -SizeY / 2.0f;
    }
    default:
    {
        check(0);
        return 0.f;
    }
    }
}

bool ExportStringMeshToMeshDescription(UTextRenderComponent* TextRenderComponent, FMeshDescription& MeshDescription)
{
    if (!TextRenderComponent->Font || TextRenderComponent->Text.IsEmpty())
    {
        return false;
    }

    TArray<FDynamicMeshVertex> OutVertices;
    TArray<int32> OutIndices;
    float FirstLineHeight = -1.f; // Only kept around for legacy positioning support
    float StartY = 0.f;
    float XScale = TextRenderComponent->WorldSize * TextRenderComponent->XScale * TextRenderComponent->InvDefaultSize;
    float YScale = TextRenderComponent->WorldSize * TextRenderComponent->YScale * TextRenderComponent->InvDefaultSize;
    const float CharIncrement = ((float)TextRenderComponent->Font->Kerning + TextRenderComponent->HorizSpacingAdjust) * XScale;

    float LineX = 0.f;

    int32 PageIndex = INDEX_NONE;


    FTextIterator It(*(TextRenderComponent->Text.ToString()));
    while (It.NextLine())
    {
        FVector2D LineSize = ComputeTextSize(It, TextRenderComponent->Font, XScale, YScale, TextRenderComponent->HorizSpacingAdjust, TextRenderComponent->VertSpacingAdjust);
        float StartX = ComputeHorizontalAlignmentOffset(LineSize, TextRenderComponent->HorizontalAlignment);

        if (FirstLineHeight < 0.f)
        {
            FirstLineHeight = LineSize.Y;
        }

        LineX = 0.f;

        TCHAR Ch = 0;
        while (It.NextCharacterInLine(Ch))
        {
            Ch = TextRenderComponent->Font->RemapChar(Ch);

            if (!TextRenderComponent->Font->Characters.IsValidIndex(Ch))
            {
                continue;
            }

            FFontCharacter& Char = TextRenderComponent->Font->Characters[Ch];

            if (!TextRenderComponent->Font->Textures.IsValidIndex(Char.TextureIndex))
            {
                continue;
            }

            // Need to start a new batch?
            if (PageIndex != Char.TextureIndex)
            {
                if (PageIndex != INDEX_NONE)
                {
                }

                PageIndex = Char.TextureIndex;
            }

            UTexture2D* Tex = TextRenderComponent->Font->Textures[Char.TextureIndex];

            if (Tex)
            {
                FIntPoint ImportedTextureSize = Tex->GetImportedSize();
                FVector2D InvTextureSize(1.0f / (float)ImportedTextureSize.X, 1.0f / (float)ImportedTextureSize.Y);

                const float X = LineX + StartX;
                const float Y = StartY + Char.VerticalOffset * YScale;
                const float SizeX = Char.USize * XScale;
                const float SizeY = Char.VSize * YScale;
                const float U = Char.StartU * InvTextureSize.X;
                const float V = Char.StartV * InvTextureSize.Y;
                const float SizeU = Char.USize * InvTextureSize.X;
                const float SizeV = Char.VSize * InvTextureSize.Y;

                const float Left = X;
                const float Top = Y;
                const float Right = X + SizeX;
                const float Bottom = Y + SizeY;

                // axis choice and sign to get good alignment when placed on surface
                const FVector4 V0(0.f, -Left, -Top, 0.f);
                const FVector4 V1(0.f, -Right, -Top, 0.f);
                const FVector4 V2(0.f, -Left, -Bottom, 0.f);
                const FVector4 V3(0.f, -Right, -Bottom, 0.f);

                const FVector TangentX(0.f, -1.f, 0.f);
                const FVector TangentY(0.f, 0.f, -1.f);
                const FVector TangentZ(1.f, 0.f, 0.f);

                const int32 V00 = OutVertices.Add(FDynamicMeshVertex(V0, TangentX, TangentZ, FVector2D(U, V), TextRenderComponent->TextRenderColor));
                const int32 V10 = OutVertices.Add(FDynamicMeshVertex(V1, TangentX, TangentZ, FVector2D(U + SizeU, V), TextRenderComponent->TextRenderColor));
                const int32 V01 = OutVertices.Add(FDynamicMeshVertex(V2, TangentX, TangentZ, FVector2D(U, V + SizeV), TextRenderComponent->TextRenderColor));
                const int32 V11 = OutVertices.Add(FDynamicMeshVertex(V3, TangentX, TangentZ, FVector2D(U + SizeU, V + SizeV), TextRenderComponent->TextRenderColor));

                check(V00 < 65536);
                check(V10 < 65536);
                check(V01 < 65536);
                check(V11 < 65536);

                OutIndices.Add(V00);
                OutIndices.Add(V11);
                OutIndices.Add(V10);

                OutIndices.Add(V00);
                OutIndices.Add(V01);
                OutIndices.Add(V11);

                LineX += SizeX;

                // if we have another non-whitespace character to render, add the font's kerning.
                TCHAR NextChar = 0;
                if (It.Peek(NextChar) && !FChar::IsWhitespace(NextChar))
                {
                    LineX += CharIncrement;
                }
            }
        }

        // Move Y position down to next line. If the current line is empty, move by max char height in font
        StartY += LineSize.Y > 0.f ? LineSize.Y : TextRenderComponent->Font->GetMaxCharHeight();
    }

    // Avoid initializing RHI resources when no vertices are generated.
    if (OutVertices.Num() > 0)
    {
        const float VerticalAlignmentOffset = -ComputeVerticalAlignmentOffset(StartY, TextRenderComponent->VerticalAlignment, FirstLineHeight);

        MeshDescription.Empty();

        TVertexAttributesRef<FVector> VertexPositions = MeshDescription.VertexAttributes().GetAttributesRef<FVector>(MeshAttribute::Vertex::Position);
        TVertexInstanceAttributesRef<FVector> VertexInstanceNormals = MeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector>(MeshAttribute::VertexInstance::Normal);
        TVertexInstanceAttributesRef<FVector> VertexInstanceTangents = MeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector>(MeshAttribute::VertexInstance::Tangent);
        TVertexInstanceAttributesRef<float> VertexInstanceBinormalSigns = MeshDescription.VertexInstanceAttributes().GetAttributesRef<float>(MeshAttribute::VertexInstance::BinormalSign);
        TVertexInstanceAttributesRef<FVector4> VertexInstanceColors = MeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector4>(MeshAttribute::VertexInstance::Color);
        TVertexInstanceAttributesRef<FVector2D> VertexInstanceUVs = MeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector2D>(MeshAttribute::VertexInstance::TextureCoordinate);

        const int32 NumWedges = OutIndices.Num();
        const int32 NumVertexPositions = OutVertices.Num();
        const int32 NumFaces = NumWedges / 3;

        MeshDescription.ReserveNewVertices(NumVertexPositions);
        MeshDescription.ReserveNewVertexInstances(NumWedges);
        MeshDescription.ReserveNewPolygons(NumFaces);
        MeshDescription.ReserveNewEdges(NumWedges);

        VertexInstanceUVs.SetNumIndices(1);

        FPolygonGroupID PolygonGroupID = FPolygonGroupID::Invalid;
        if (MeshDescription.PolygonGroups().Num() < 1)
        {
            PolygonGroupID = MeshDescription.CreatePolygonGroup();
        }
        else
        {
            PolygonGroupID = MeshDescription.PolygonGroups().GetFirstValidID();
        }

        for (int32 VertexIndex = 0; VertexIndex < NumVertexPositions; ++VertexIndex)
        {
            FVertexID VertexID = MeshDescription.CreateVertex();
            VertexPositions[VertexID] = OutVertices[VertexIndex].Position + FVector(0, 0, VerticalAlignmentOffset);
        }

        //Create the vertex instances
        for (int32 TriangleIndex = 0; TriangleIndex < NumFaces; ++TriangleIndex)
        {
            FVertexID VertexIDs[3];
            TArray<FVertexInstanceID> VertexInstanceIDs;
            VertexInstanceIDs.SetNum(3);

            for (int32 Corner = 0; Corner < 3; ++Corner)
            {
                int32 WedgeIndex = OutIndices[TriangleIndex * 3 + Corner];
                FVertexID VertexID(WedgeIndex);
                FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexID);
                VertexIDs[Corner] = VertexID;
                VertexInstanceIDs[Corner] = VertexInstanceID;

                //NTBs
                const FVector TangentX(0.f, -1.f, 0.f);
                const FVector TangentY(0.f, 0.f, -1.f);
                const FVector TangentZ(1.f, 0.f, 0.f);
                VertexInstanceTangents[VertexInstanceID] = TangentX;
                VertexInstanceBinormalSigns[VertexInstanceID] = GetBasisDeterminantSign(TangentX, TangentY, TangentZ);
                VertexInstanceNormals[VertexInstanceID] = TangentZ;

                // Vertex colors
                // can't use FLinearColor(FColor) here, because it assumed FColor being in sRGB space
                VertexInstanceColors[VertexInstanceID] = TextRenderComponent->TextRenderColor.ReinterpretAsLinear();

                //Tex coord
                VertexInstanceUVs.Set(VertexInstanceID, 0, OutVertices[WedgeIndex].TextureCoordinate[0]);
            }
            //Create a polygon from this triangle
            MeshDescription.CreatePolygon(PolygonGroupID, VertexInstanceIDs);
        }

        return true;
    }

    return false;
}

bool ExportDecalToMeshDescription(UDecalComponent* DecalComponent, FMeshDescription& MeshDescription)
{
    TArray<FDynamicMeshVertex> OutVertices;
    TArray<int32> OutIndices;

    const float Left = -DecalComponent->DecalSize.Y;
    const float Bottom = -DecalComponent->DecalSize.Z;
    const float Right = DecalComponent->DecalSize.Y;
    const float Top = DecalComponent->DecalSize.Z;
    const float Front = -DecalComponent->DecalSize.X;

    const FVector TangentX(0.f, 0.f, 1.f);
    const FVector TangentY(0.f, 1.f, 0.f);
    const FVector TangentZ(-1.f, 0.f, 0.f);

    // axis choice and sign to get good alignment when placed on surface
    const FVector4 V0(Front, Right, Top, 0.f);
    const FVector4 V1(Front, Left, Top, 0.f);
    const FVector4 V2(Front, Right, Bottom, 0.f);
    const FVector4 V3(Front, Left, Bottom, 0.f);

    const int32 V00 = OutVertices.Add(FDynamicMeshVertex(V0, TangentX, TangentZ, FVector2D(1, 1), FColor(255, 255, 255)));
    const int32 V10 = OutVertices.Add(FDynamicMeshVertex(V1, TangentX, TangentZ, FVector2D(1, 0), FColor(255, 255, 255)));
    const int32 V01 = OutVertices.Add(FDynamicMeshVertex(V2, TangentX, TangentZ, FVector2D(0, 1), FColor(255, 255, 255)));
    const int32 V11 = OutVertices.Add(FDynamicMeshVertex(V3, TangentX, TangentZ, FVector2D(0, 0), FColor(255, 255, 255)));

    OutIndices.Add(V00);
    OutIndices.Add(V10);
    OutIndices.Add(V01);

    OutIndices.Add(V01);
    OutIndices.Add(V10);
    OutIndices.Add(V11);

    // Avoid initializing RHI resources when no vertices are generated.
    if (OutVertices.Num() > 0)
    {
        MeshDescription.Empty();

        TVertexAttributesRef<FVector> VertexPositions = MeshDescription.VertexAttributes().GetAttributesRef<FVector>(MeshAttribute::Vertex::Position);
        TVertexInstanceAttributesRef<FVector> VertexInstanceNormals = MeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector>(MeshAttribute::VertexInstance::Normal);
        TVertexInstanceAttributesRef<FVector> VertexInstanceTangents = MeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector>(MeshAttribute::VertexInstance::Tangent);
        TVertexInstanceAttributesRef<float> VertexInstanceBinormalSigns = MeshDescription.VertexInstanceAttributes().GetAttributesRef<float>(MeshAttribute::VertexInstance::BinormalSign);
        TVertexInstanceAttributesRef<FVector4> VertexInstanceColors = MeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector4>(MeshAttribute::VertexInstance::Color);
        TVertexInstanceAttributesRef<FVector2D> VertexInstanceUVs = MeshDescription.VertexInstanceAttributes().GetAttributesRef<FVector2D>(MeshAttribute::VertexInstance::TextureCoordinate);

        const int32 NumWedges = OutIndices.Num();
        const int32 NumVertexPositions = OutVertices.Num();
        const int32 NumFaces = NumWedges / 3;

        MeshDescription.ReserveNewVertices(NumVertexPositions);
        MeshDescription.ReserveNewVertexInstances(NumWedges);
        MeshDescription.ReserveNewPolygons(NumFaces);
        MeshDescription.ReserveNewEdges(NumWedges);

        VertexInstanceUVs.SetNumIndices(1);

        FPolygonGroupID PolygonGroupID = FPolygonGroupID::Invalid;
        if (MeshDescription.PolygonGroups().Num() < 1)
        {
            PolygonGroupID = MeshDescription.CreatePolygonGroup();
        }
        else
        {
            PolygonGroupID = MeshDescription.PolygonGroups().GetFirstValidID();
        }

        for (int32 VertexIndex = 0; VertexIndex < NumVertexPositions; ++VertexIndex)
        {
            FVertexID VertexID = MeshDescription.CreateVertex();
            VertexPositions[VertexID] = OutVertices[VertexIndex].Position;
        }

        //Create the vertex instances
        for (int32 TriangleIndex = 0; TriangleIndex < NumFaces; ++TriangleIndex)
        {
            FVertexID VertexIDs[3];
            TArray<FVertexInstanceID> VertexInstanceIDs;
            VertexInstanceIDs.SetNum(3);

            for (int32 Corner = 0; Corner < 3; ++Corner)
            {
                int32 WedgeIndex = OutIndices[TriangleIndex * 3 + Corner];
                FVertexID VertexID(WedgeIndex);
                FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexID);
                VertexIDs[Corner] = VertexID;
                VertexInstanceIDs[Corner] = VertexInstanceID;

                //NTBs
                VertexInstanceTangents[VertexInstanceID] = TangentX;
                VertexInstanceBinormalSigns[VertexInstanceID] = GetBasisDeterminantSign(TangentX, TangentY, TangentZ);
                VertexInstanceNormals[VertexInstanceID] = TangentZ;

                //Tex coord
                VertexInstanceUVs.Set(VertexInstanceID, 0, OutVertices[WedgeIndex].TextureCoordinate[0]);
            }
            //Create a polygon from this triangle
            MeshDescription.CreatePolygon(PolygonGroupID, VertexInstanceIDs);
        }

        return true;
    }

    return false;
}

pxr::UsdSkelRoot FSimReadyUSDHelper::ExportSkeletalMeshToUSD(class USkeletalMesh* SkeletalMesh, class USkeletalMeshComponent* SkeletalMeshComponent,
    const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& ExportPrimPath, const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
    bool bOverrideMaterial, const FSimReadyExportSettings& ExportSettings,
    TMap<FString, FString>& ExportedMDLs)
{
#if UE_BUILD_DEBUG
    const FString PrimPathText = ExportPrimPath.GetText();
#endif
    pxr::UsdSkelRoot USDSkelRoot;

    if (SkeletalMesh && USDStage)
    {
        if (FSimReadyReferenceCollector::Get().FindDependency(SkeletalMesh->GetOuter()->GetFName()))
        {
            FString Text = FString::Printf(TEXT("Exporting Object %s..."), *SkeletalMesh->GetName());
            FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
        }

        if (FSimReadySlowTask::Get().IsProgressCancelled())
        {
            return USDSkelRoot;
        }

        USDSkelRoot = FUSDExporter::ExportSkeletalMesh(USDStage, ExportPrimPath, *SkeletalMesh, false, ExportSettings.bRootIdentity);
                
        if (USDSkelRoot)
        {
            extern void ExportCarlaTag(const pxr::UsdPrim & Prim, const UObject * Asset, const UActorComponent * Component);
            ExportCarlaTag(USDSkelRoot.GetPrim(), SkeletalMesh, SkeletalMeshComponent);

            TArray<pxr::UsdPrim> MeshPrims;
            GetMeshFromSkelRoot(USDSkelRoot, MeshPrims);
            if (MeshPrims.Num() == 0)
            {
                return USDSkelRoot;
            }

            //export materials
            const FSkeletalMeshRenderData * SkelMeshRenderData = SkeletalMesh->GetResourceForRendering();
            if (SkelMeshRenderData && SkelMeshRenderData->LODRenderData.Num() > 0)
            {
                int32 LODToExport = 0;
                const FSkeletalMeshLODRenderData& LODData = SkelMeshRenderData->LODRenderData[LODToExport];
                const FSkeletalMeshLODInfo& Info = *(SkeletalMesh->GetLODInfo(LODToExport));

                if (MeshPrims.Num() == LODData.RenderSections.Num())
                {
                    for (int SectionIndex = 0; SectionIndex < LODData.RenderSections.Num(); ++SectionIndex)
                    {
                        int32 MaterialIndex = LODData.RenderSections[SectionIndex].MaterialIndex;

                        if (SectionIndex < Info.LODMaterialMap.Num() && SkeletalMesh->Materials.IsValidIndex(Info.LODMaterialMap[SectionIndex]))
                        {
                            MaterialIndex = Info.LODMaterialMap[SectionIndex];
                            MaterialIndex = FMath::Clamp(MaterialIndex, 0, SkeletalMesh->Materials.Num());
                        }

                        UMaterialInterface* MaterialInt = nullptr;
                        const FNamedParameterTimeSamples* ParameterTimeSamples = nullptr;
                        if (bOverrideMaterial && SkeletalMeshComponent)
                        {
                            MaterialInt = SkeletalMeshComponent->GetMaterial(MaterialIndex);
                            ParameterTimeSamples = GetMaterialParameterTimeSample(SkeletalMeshComponent, MaterialIndex);
                        }
                        else
                        {
                            MaterialInt = SkeletalMesh->Materials[MaterialIndex].MaterialInterface;
                        }

                        if (!MaterialInt)
                        {
                            continue;
                        }

                        auto Prim = MeshPrims[SectionIndex];
                        if (Prim)
                        {
                            if (FUSDCARLAVehicleTools::BelongToCarlaVehicle(SkeletalMeshComponent))
                            {
                                FUSDCARLAVehicleTools::ExportSimReadyVehicleComponentType(Prim, MaterialInt);
                            }

                            ExportAndBindMaterial(USDStage, MaterialInt, ParameterTimeSamples, Prim, ExportSettings, ExportMDLBaseAbsolutePath, ExportMDLBaseRelativePath, ExportedMDLs, TEXT(""), ParameterTimeSamples ? SkeletalMeshComponent->GetOwner()->GetName() + SkeletalMeshComponent->GetName() + MaterialInt->GetName() : TEXT(""));

                            extern void ExportSimReadyNonVisMaterialDataForPedestrian(const pxr::UsdPrim & MeshPrim, const UMaterialInterface & IMaterial);
                            ExportSimReadyNonVisMaterialDataForPedestrian(Prim, *MaterialInt);
                        }
                    }
                }
            }
        }
    }

    return USDSkelRoot;
}

bool FSimReadyUSDHelper::ExportSkeletalMeshAsUSDToPathInternal(USkeletalMesh * SkeletalMesh, USkeletalMeshComponent* SkeletalMeshComponent, const FString & ExportMeshPath,
    const FString& ExportMDLBaseAbsolutePath, bool bOverrideMaterial, const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs)
{
    pxr::UsdStageRefPtr USDStage = CreateUSDStageFromPath(ExportMeshPath, false, !ExportSettings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }
    pxr::SdfPath RootPrimPath = InitDefaultPrim(USDStage, pxr::KindTokens->component);

    // MeshPrim name should be filename without extension
    FString MeshPrimName = FPaths::GetBaseFilename(ExportMeshPath);
    while (MeshPrimName != FPaths::GetBaseFilename(MeshPrimName)) // Remove all extension
    {
        MeshPrimName = FPaths::GetBaseFilename(MeshPrimName);
    }
    if (!pxr::TfIsValidIdentifier(TCHAR_TO_ANSI(*MeshPrimName)))
    {
        MeshPrimName = SkeletalMesh->GetName();
    }

    FString ExportMDLBaseRelativePath = FSimReadyPathHelper::ComputeRelativePath(ExportMDLBaseAbsolutePath, ExportMeshPath, true);
    pxr::SdfPath MeshPrimPath = RootPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*MeshPrimName)));
    if (auto SkelRootPrim = ExportSkeletalMeshToUSD(SkeletalMesh, SkeletalMeshComponent, USDStage, MeshPrimPath,
        ExportMDLBaseAbsolutePath, ExportMDLBaseRelativePath, bOverrideMaterial, ExportSettings, ExportedMDLs))
    {
        // Carla vehicles to split the wheels
        if (FUSDCARLAVehicleTools::BelongToCarlaVehicle(SkeletalMeshComponent))
        {
            FUSDCARLAVehicleTools::SplitVehicleSkelByJoints(USDStage, SkelRootPrim);
            FUSDCARLAVehicleTools::ConvertSkelToMesh(USDStage, SkelRootPrim);
        }

        USDStage->Save();

        return true;
    }

    return false;
}

bool FSimReadyUSDHelper::ExportAnimationAssetAsUSDToPathInternal(UAnimationAsset * AnimationAsset, USkeletalMeshComponent* SkeletalMeshComponent, const FString & ExportAnimePath, const FSimReadyExportSettings& ExportSettings)
{
    pxr::UsdStageRefPtr USDStage = CreateUSDStageFromPath(ExportAnimePath, false, !ExportSettings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }
    pxr::SdfPath RootPrimPath = InitDefaultPrim(USDStage, pxr::KindTokens->component);

    if (FSimReadyReferenceCollector::Get().FindDependency(AnimationAsset->GetOuter()->GetFName()))
    {
        FString Text = FString::Printf(TEXT("Exporting Object %s..."), *AnimationAsset->GetName());
        FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
    }

    if (FSimReadySlowTask::Get().IsProgressCancelled())
    {
        return false;
    }

    // AnimePrim name should be filename without extension
    FString AnimePrimName = FPaths::GetBaseFilename(ExportAnimePath);
    while (AnimePrimName != FPaths::GetBaseFilename(AnimePrimName)) // Remove all extension
    {
        AnimePrimName = FPaths::GetBaseFilename(AnimePrimName);
    }
    if (!pxr::TfIsValidIdentifier(TCHAR_TO_ANSI(*AnimePrimName)))
    {
        AnimePrimName = AnimationAsset->GetName();
    }

    pxr::SdfPath AnimePrimPath = RootPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*AnimePrimName)));

    TArray<FTransformTimeSamples> SocketTransformTimeSamples;
    bool Ret = FUSDExporter::ExportSkeletalMeshAnimation(USDStage, AnimePrimPath, AnimationAsset, SkeletalMeshComponent ? SkeletalMeshComponent->SkeletalMesh : nullptr, SocketTransformTimeSamples, ExportSettings.bRootIdentity);
    
    if (SkeletalMeshComponent)
    {
        TArray<FName> SocketNames = SkeletalMeshComponent->GetAllSocketNames();
        for (int32 SocketIndex = 0; SocketIndex < SocketTransformTimeSamples.Num(); ++SocketIndex)
        {
            FSequenceExtractor::Get().SetSocketTimeSamples(SkeletalMeshComponent, SocketNames[SocketIndex], SocketTransformTimeSamples[SocketIndex]);
        }
    }
    else if (ExportSettings.bExportPreviewMesh && AnimationAsset->GetSkeleton() && AnimationAsset->GetSkeleton()->GetPreviewMesh())
    {
        // Export the preview skeletal mesh
        auto PreviewSkeletalMesh = AnimationAsset->GetSkeleton()->GetPreviewMesh();
        TMap<FString, FString> ExportedMDLs;
        FString ExportPath = FPaths::GetPath(ExportAnimePath);
        pxr::SdfPath SkeletalPrimPath = RootPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*PreviewSkeletalMesh->GetName())));
        pxr::UsdSkelRoot USDSkelRoot = ExportSkeletalMeshToUSD(AnimationAsset->GetSkeleton()->GetPreviewMesh(), nullptr, USDStage, SkeletalPrimPath, ExportPath, TEXT(""), false, ExportSettings, ExportedMDLs);
        if (USDSkelRoot)
        {
            pxr::UsdSkelBindingAPI AnimationSourceBinding = pxr::UsdSkelBindingAPI::Apply(USDSkelRoot.GetPrim());
            AnimationSourceBinding.CreateAnimationSourceRel().SetTargets(pxr::SdfPathVector({ AnimePrimPath }));
        }
    }

    USDStage->Save();
    return Ret;
}

bool FSimReadyUSDHelper::ExportAnimationTimeSamplesAsUSDToPathInternal(const TArray<FSkeletalAnimationTimeSamples>* AnimationTimeSamples, const double StartTimeCode, const double EndTimeCode, const double TimeCodesPerSecond, USkeletalMeshComponent* SkeletalMeshComponent, const FString & ExportAnimePath, const FSimReadyExportSettings& ExportSettings)
{
    pxr::UsdStageRefPtr USDStage = CreateUSDStageFromPath(ExportAnimePath, false, !ExportSettings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }
    pxr::SdfPath RootPrimPath = InitDefaultPrim(USDStage, pxr::KindTokens->component);

    // AnimePrim name should be filename without extension
    FString AnimePrimName = FPaths::GetBaseFilename(ExportAnimePath);
    while (AnimePrimName != FPaths::GetBaseFilename(AnimePrimName)) // Remove all extension
    {
        AnimePrimName = FPaths::GetBaseFilename(AnimePrimName);
    }

    pxr::SdfPath AnimePrimPath = RootPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*AnimePrimName)));

    bool Ret = FUSDExporter::ExportSkeletalMeshAnimationTimeSamples(USDStage, AnimePrimPath, *SkeletalMeshComponent, *AnimationTimeSamples, StartTimeCode, EndTimeCode, TimeCodesPerSecond, ExportSettings.bRootIdentity);

    USDStage->Save();
    return Ret;
}

bool FSimReadyUSDHelper::TryExportAndBindMaterialWithAOMask(UStaticMeshComponent* StaticMeshComponent, const FNamedParameterTimeSamples* ParameterTimeSamples, int32 LODToExport, const pxr::UsdStageRefPtr& USDStage, UMaterialInterface* MaterialInt, const pxr::UsdPrim& Prim,
    const FSimReadyExportSettings& ExportSettings, const FString& MDLBaseAbsolutePath, const FString& MDLBaseRelativePath, TMap<FString, FString>& ExportedMDLs)
{
    if (StaticMeshComponent && StaticMeshComponent->GetOwner() && MaterialInt && LODToExport < StaticMeshComponent->LODData.Num() && ExportSettings.bMDL)
    {
        const FStaticMeshComponentLODInfo& LODInfo = StaticMeshComponent->LODData[LODToExport];
        const FMeshMapBuildData* MeshMapBuildData = StaticMeshComponent->GetMeshMapBuildData(LODInfo);
        FLightMap2D* Lightmap = MeshMapBuildData && MeshMapBuildData->LightMap ? MeshMapBuildData->LightMap->GetLightMap2D() : nullptr;
        if (Lightmap)
        {
            auto AOMaterialMaskTexture = Lightmap->GetAOMaterialMaskTexture();

            if (AOMaterialMaskTexture)
            {
                bool bInstancedMesh = StaticMeshComponent && StaticMeshComponent->IsA<UInstancedStaticMeshComponent>();
                int32 NumInstances = bInstancedMesh ? Cast<UInstancedStaticMeshComponent>(StaticMeshComponent)->GetNumRenderInstances() : 0;
                FString UsdMaterialName = StaticMeshComponent->GetOwner()->GetName() + StaticMeshComponent->GetName() + MaterialInt->GetName();
                ExportAndBindMaterial(USDStage, MaterialInt, ParameterTimeSamples, Prim, ExportSettings, MDLBaseAbsolutePath, MDLBaseRelativePath, ExportedMDLs, TEXT(""), UsdMaterialName,
                    [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FSimReadyExportMaterialResult& MaterialResult, FString& UsdMaterialName)
                    {
                        CheckUsdNameByNumTexCoord(InStage, InPrimPath, StaticMeshComponent->GetStaticMesh()->RenderData->LODResources[LODToExport], MaterialResult, UsdMaterialName);
                        if (bInstancedMesh)
                        {
                            CheckUsdNameByInstanceData(InStage, InPrimPath, NumInstances, MaterialResult, UsdMaterialName);
                        }
                    }
                    ,
                    [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FString& MDLName, const FSimReadyExportMaterialResult& MaterialResult, const FString& UsdMaterialName)
                    {
                        BindNumTexCoord(InStage, InPrimPath, StaticMeshComponent->GetStaticMesh(), LODToExport, MaterialResult);
                        if (bInstancedMesh)
                        {
                            BindInstanceData(InStage, InPrimPath, NumInstances, MaterialResult);
                        }

                        if (MaterialResult.IsAOMaterialMaskTextureUsed() && MaterialResult.IsLightMapCoordinateUsed())
                        {
                            FString RelativeAOTexturePath = (ExportSettings.bModular && !ExportSettings.bForceShared ? MDLName : TEXT("AOMaskTextures")) / FSimReadyAssetExportHelper::GetTextureFileName(AOMaterialMaskTexture, ExportSettings.MaterialSettings.TextureSettings.bDDSExport);
                            BindAOMask(InStage, InPrimPath, StaticMeshComponent->GetStaticMesh()->LightMapCoordinateIndex, Lightmap->GetCoordinateScale(), Lightmap->GetCoordinateBias(), ExportSettings.bMaterialOverSublayer ? RelativeAOTexturePath : (MDLBaseRelativePath / RelativeAOTexturePath));				
                            /// Upload AO mask textures
                            FString OmniPath = MDLBaseAbsolutePath / RelativeAOTexturePath;
                            if (!FSimReadyAssetExportHelper::IsTextureExported(AOMaterialMaskTexture))
                            {
                                if (FSimReadyReferenceCollector::Get().FindDependency(AOMaterialMaskTexture->GetOuter()->GetFName()))
                                {
                                    FString Text = FString::Printf(TEXT("Exporting Object %s..."), *AOMaterialMaskTexture->GetName());
                                    FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
                                }

                                if (FSimReadySlowTask::Get().IsProgressCancelled())
                                {
                                    return;
                                }

                                if (FSimReadyAssetExportHelper::ExportTextureToPath(AOMaterialMaskTexture, OmniPath, ExportSettings.MaterialSettings.TextureSettings))
                                {
                                    FSimReadyAssetExportHelper::RegisterExportedTexture(AOMaterialMaskTexture);
                                }
                            }
                        }
                    }
                );

                return true;
            }
        }
    }

    return false;
}

pxr::UsdGeomXformable FSimReadyUSDHelper::ExportStaticMeshToUSD(class UStaticMesh* StaticMesh, class UStaticMeshComponent* StaticMeshComponent,
    const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& ExportPrimPath, const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
    bool bOverrideMaterial, const FSimReadyExportSettings& ExportSettings,
    TMap<FString, FString>& ExportedMDLs)
{
#if UE_BUILD_DEBUG
    const FString PrimPathText = ExportPrimPath.GetText();
#endif
    pxr::UsdGeomXformable USDMesh;
    if (StaticMesh && USDStage)
    {
        if (FSimReadyReferenceCollector::Get().FindDependency(StaticMesh->GetOuter()->GetFName()))
        {
            FString Text = FString::Printf(TEXT("Exporting Object %s..."), *StaticMesh->GetName());
            FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
        }

        if (FSimReadySlowTask::Get().IsProgressCancelled())
        {
            return USDMesh;
        }

        if (StaticMesh->RenderData->LODResources.Num() == 0)
        {
            return USDMesh;
        }

        const int32 SMCurrentMinLOD = StaticMesh->MinLOD.GetValue();
        int32 LODToExport = FMath::Clamp(StaticMeshComponent && StaticMeshComponent->bOverrideMinLOD ? StaticMeshComponent->MinLOD : SMCurrentMinLOD, 0, StaticMesh->RenderData->LODResources.Num() - 1);

        if (StaticMeshComponent && StaticMeshComponent->IsA<USplineMeshComponent>())
        {
            FMeshDescription MeshDescription;
            FStaticMeshAttributes(MeshDescription).Register();
            ExportStaticMeshLOD(StaticMesh->RenderData->LODResources[LODToExport], MeshDescription, StaticMesh->StaticMaterials);
            //
            PropagateSplineDeformationToRawMesh(Cast<USplineMeshComponent>(StaticMeshComponent), MeshDescription);
            USDMesh = FUSDExporter::ExportMeshDescription(USDStage, ExportPrimPath, MeshDescription);

            if (USDMesh)
            {
                ExportShadowFromPrimitiveComponent(USDMesh, StaticMeshComponent);
            }
        }
        else
        {
            USDMesh = FUSDExporter::ExportStaticMesh(USDStage, ExportPrimPath, StaticMesh->RenderData->LODResources[LODToExport]);
        
            if (USDMesh)
            {
                if (StaticMeshComponent)
                {
                    ExportShadowFromPrimitiveComponent(USDMesh, StaticMeshComponent);
                }
                else
                {
                    ExportShadowFromStaticMesh(USDMesh, StaticMesh);
                }
            }
        }
        
        if (USDMesh)
        {
            //export materials
            int32 NumSections = StaticMesh->RenderData->LODResources[LODToExport].Sections.Num();
            for (int SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
            {
                const FStaticMeshSection& Section = StaticMesh->RenderData->LODResources[LODToExport].Sections[SectionIndex];
                UMaterialInterface* MaterialInt = nullptr;
                const FNamedParameterTimeSamples* ParameterTimeSamples = nullptr;
                if (bOverrideMaterial && StaticMeshComponent)
                {
                    MaterialInt = StaticMeshComponent->GetMaterial(Section.MaterialIndex);
                    ParameterTimeSamples = GetMaterialParameterTimeSample(StaticMeshComponent, Section.MaterialIndex);
                }
                else
                {
                    MaterialInt = StaticMesh->GetMaterial(Section.MaterialIndex);
                }

                if (!MaterialInt)
                {
                    continue;
                }

                auto Prim = NumSections == 1 ? USDMesh.GetPrim() : GetSectionPrimByIndex(USDStage, USDMesh.GetPath(), SectionIndex);
                if (Prim)
                {
                    bool bAOMaskExported = bOverrideMaterial && TryExportAndBindMaterialWithAOMask(StaticMeshComponent, ParameterTimeSamples, LODToExport, USDStage, MaterialInt, Prim, ExportSettings, ExportMDLBaseAbsolutePath, ExportMDLBaseRelativePath, ExportedMDLs);
                    if (!bAOMaskExported)
                    {
                        bool bInstancedMesh = StaticMeshComponent && StaticMeshComponent->IsA<UInstancedStaticMeshComponent>();
                        int32 NumInstances = bInstancedMesh ? Cast<UInstancedStaticMeshComponent>(StaticMeshComponent)->GetNumRenderInstances() : 0;
                        ExportAndBindMaterial(USDStage, MaterialInt, ParameterTimeSamples, Prim, ExportSettings, ExportMDLBaseAbsolutePath, ExportMDLBaseRelativePath, ExportedMDLs, TEXT(""), ParameterTimeSamples ? StaticMeshComponent->GetOwner()->GetName() + StaticMeshComponent->GetName() + MaterialInt->GetName() : TEXT(""),
                            [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FSimReadyExportMaterialResult& MaterialResult, FString& UsdMaterialName)
                            {
                                CheckUsdNameByNumTexCoord(InStage, InPrimPath, StaticMesh->RenderData->LODResources[LODToExport], MaterialResult, UsdMaterialName);
                                if (bInstancedMesh)
                                {
                                    CheckUsdNameByInstanceData(InStage, InPrimPath, NumInstances, MaterialResult, UsdMaterialName);
                                }
                            },
                            [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FString& MDLName, const FSimReadyExportMaterialResult& MaterialResult, const FString& UsdMaterialName)
                            {
                                BindNumTexCoord(InStage, InPrimPath, StaticMesh, LODToExport, MaterialResult);
                                if (bInstancedMesh)
                                {
                                    BindInstanceData(InStage, InPrimPath, NumInstances, MaterialResult);
                                }
                            }
                        );

                        if (StaticMeshComponent)
                        {
                            extern void ExportSimReadyNonVisMaterialDataForGeneralObject(const pxr::UsdPrim& MeshPrim, const UPrimitiveComponent& Component, const UMaterialInterface& IMaterial);
                            ExportSimReadyNonVisMaterialDataForGeneralObject(Prim, *StaticMeshComponent, *MaterialInt);
                        }
                    }
                }
            }
        }
    }

    if (USDMesh)
    {
        extern void ExportCarlaTag(const pxr::UsdPrim& Prim, const UObject* Asset, const UActorComponent* Component);
        ExportCarlaTag(USDMesh.GetPrim(), StaticMesh, StaticMeshComponent);
    }

    return USDMesh;
}

bool FSimReadyUSDHelper::ExportOverrideMeshAsUSDToPath(UStaticMeshComponent* StaticMeshComponent, const FString& USDAbsolutePath, const FString& USDRelativePathToRoot, const FString& MeshUSDPath, int32 LODToExport,
    const FString& MDLAbsoluteBasePath, const FString& MDLRelativeBasePathToRoot,
    const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs)
{
    pxr::UsdStageRefPtr USDStage = CreateUSDStageFromPath(USDAbsolutePath, false, !ExportSettings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }

    pxr::SdfPath RootPrimPath = InitDefaultPrim(USDStage, pxr::KindTokens->component);

    FString MeshPrimName = FPaths::GetBaseFilename(MeshUSDPath);
    while (MeshPrimName != FPaths::GetBaseFilename(MeshPrimName)) // Remove all extension
    {
        MeshPrimName = FPaths::GetBaseFilename(MeshPrimName);
    }
    pxr::SdfPath PrimPath = RootPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*MeshPrimName)));
    auto ReferencedPrim = pxr::UsdGeomXform::Define(USDStage, PrimPath).GetPrim();
    AddReferenceOrPayload(ExportSettings.bPayloads, ReferencedPrim, USDRelativePathToRoot);
    auto MeshPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*MeshPrimName)));
    pxr::UsdPrim USDMeshPrim = USDStage->GetPrimAtPath(MeshPrimPath);
    if (USDMeshPrim)
    {
        TMap<FString, FString> TempExportedMDLs;
        UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
        int32 NumSections = StaticMesh->GetSectionInfoMap().GetSectionNumber(LODToExport);

        if (LODToExport < StaticMeshComponent->LODData.Num())
        {
            auto ColorVertexBuffer = StaticMeshComponent->LODData[LODToExport].OverrideVertexColors;
            if (ColorVertexBuffer)
            {
                FUSDExporter::ExportOverrideVertexColor(USDStage, MeshPrimPath, StaticMesh->RenderData->LODResources[LODToExport], ColorVertexBuffer);
            }
        }

        for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
        {
            FMeshSectionInfo Info = StaticMesh->GetSectionInfoMap().Get(LODToExport, SectionIndex);
            auto AssetMaterialInterface = StaticMesh->GetMaterial(Info.MaterialIndex);
            auto ComponentMaterialInterface = StaticMeshComponent->GetMaterial(Info.MaterialIndex);
            auto ParameterTimeSamples = GetMaterialParameterTimeSample(StaticMeshComponent, Info.MaterialIndex);
            if (!ComponentMaterialInterface)
            {
                continue;
            }

            auto Prim = NumSections == 1 ? USDMeshPrim : GetSectionPrimByIndex(USDStage, USDMeshPrim.GetPath(), SectionIndex);
            if (Prim)
            {
                //Binding materials for component, skipping the ones that
                //component does not override the materials from static mesh.
                if (!AssetMaterialInterface ||
                    AssetMaterialInterface->GetPathName() != ComponentMaterialInterface->GetPathName() ||
                    ParameterTimeSamples != nullptr)
                {
                    ExportAndBindMaterial(USDStage, ComponentMaterialInterface, ParameterTimeSamples, Prim, ExportSettings, MDLAbsoluteBasePath, MDLRelativeBasePathToRoot, ExportSettings.bModular ? TempExportedMDLs : ExportedMDLs, TEXT(""), TEXT(""),
                    [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FSimReadyExportMaterialResult& MaterialResult, FString& UsdMaterialName)
                    {
                        CheckUsdNameByNumTexCoord(InStage, InPrimPath, StaticMesh->RenderData->LODResources[LODToExport], MaterialResult, UsdMaterialName);
                    },
                    [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FString& MDLName, const FSimReadyExportMaterialResult& MaterialResult, const FString& UsdMaterialName)
                    {
                        BindNumTexCoord(InStage, InPrimPath, StaticMesh, LODToExport, MaterialResult);
                    }
                    );
                }
            }
        }
    }

    USDStage->Save();
    return true;
}

bool FSimReadyUSDHelper::ExportStaticMeshAsUSDToPathInternal(UStaticMesh * StaticMesh, UStaticMeshComponent* StaticMeshComponent, const FString & ExportMeshPath,
    const FString& ExportMDLBaseAbsolutePath, bool bOverrideMaterial, const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs)
{
    pxr::UsdStageRefPtr USDStage = CreateUSDStageFromPath(ExportMeshPath, false, !ExportSettings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }
    pxr::SdfPath RootPrimPath = InitDefaultPrim(USDStage, pxr::KindTokens->component);

    // MeshPrim name should be filename without extension
    FString MeshPrimName = FPaths::GetBaseFilename(ExportMeshPath);
    while (MeshPrimName != FPaths::GetBaseFilename(MeshPrimName)) // Remove all extension
    {
        MeshPrimName = FPaths::GetBaseFilename(MeshPrimName);
    }

    FString ExportMDLBaseRelativePath = FSimReadyPathHelper::ComputeRelativePath(ExportMDLBaseAbsolutePath, ExportMeshPath, true);
    pxr::SdfPath MeshPrimPath = RootPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*MeshPrimName)));
    if (auto UsdMeshPrim = ExportStaticMeshToUSD(StaticMesh, StaticMeshComponent, USDStage, MeshPrimPath,
        ExportMDLBaseAbsolutePath, ExportMDLBaseRelativePath, bOverrideMaterial, ExportSettings, ExportedMDLs))
    {
        // Carla vehicles to split the lights
        if (FUSDCARLAVehicleTools::IsCarlaVehicleLight(StaticMeshComponent))
        {
            bool bIsMesh = UsdMeshPrim.GetPrim().IsA<pxr::UsdGeomMesh>();
            FUSDCARLAVehicleTools::SplitVehicleLightByTexcoords(USDStage, UsdMeshPrim.GetPrim());
            if (FUSDCARLAVehicleTools::IsCarlaVehicleLightMaterial(StaticMesh->GetMaterial(0)))
            {
                TArray<pxr::UsdPrim> USDMeshPrims;
                auto LightsPath = (bIsMesh ? RootPrimPath : MeshPrimPath).AppendElementString("Lights");
                FindAllPrims<pxr::UsdGeomMesh>(USDStage->GetPrimAtPath(LightsPath), USDMeshPrims);
                FUSDCARLAVehicleTools::BindMaterialsToSplitVehicleLight(USDStage, USDMeshPrims, StaticMesh->GetMaterial(0));
            }
        }

        USDStage->Save();
        return true;
    }

    return false;
}

bool FSimReadyUSDHelper::ExportObjectAsUSDToPathInternal(UObject* Object, const FString & ExportPath, const FSimReadyExportSettings& ExportSettings)
{
    pxr::UsdStageRefPtr USDStage = CreateUSDStageFromPath(ExportPath, false, !ExportSettings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }
    pxr::SdfPath RootPrimPath = InitDefaultPrim(USDStage, pxr::KindTokens->component);

    // USDPrim name should be filename without extension
    FString PrimName = FPaths::GetBaseFilename(ExportPath);
    while (PrimName != FPaths::GetBaseFilename(PrimName)) // Remove all extension
    {
        PrimName = FPaths::GetBaseFilename(PrimName);
    }
    if (!pxr::TfIsValidIdentifier(TCHAR_TO_ANSI(*PrimName)))
    {
        PrimName = Object->GetName();
    }

    pxr::SdfPath PrimPath = RootPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*PrimName)));

    auto USDModule = FModuleManager::GetModulePtr<ISimReadyUSDModule>("SimReadyUSD");
    if (USDModule && USDModule->HasUSDTranslator())
    {
        for(auto USDTranslator : USDModule->GetUSDTranslators())
        {
            if (USDTranslator->IsObjectSupported(Object))
            {
                auto USDPrim = USDTranslator->TranslateObjectToUSD(Object, USDStage, PrimPath);
                if (USDPrim)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool FSimReadyUSDHelper::ExportBlueprintAsUSDToPathInternal(UBlueprint* Blueprint, const FString& ExportPath, const FSimReadyExportSettings& ExportSettings)
{
    bool bSuccess = false;
    if (Blueprint)
    {
        AActor* Actor = nullptr;
        UClass* AssetClass = Blueprint->GeneratedClass;
        if (AssetClass)
        {
            if (GWorld && AssetClass->IsChildOf<AActor>())
            {
                UPackage* Package = GWorld->GetOutermost();
                bool bDirty = false;
                if (Package)
                {
                    bDirty = Package->IsDirty();
                }

                Actor = GWorld->SpawnActor(Blueprint->GeneratedClass);
                if (Actor)
                {
                    Actor->PostEditChange();
                    TArray<AActor*> Actors;
                    Actors.Add(Actor);
                    FSimReadyExportSettings BlueprintExportSettings = ExportSettings;
                    BlueprintExportSettings.bCustomLayer = false;
                    BlueprintExportSettings.bExportXodr = false;
                    BlueprintExportSettings.bEditLayer = false;
                    ExportActorsAsUSDToPath(Actors, ExportPath, BlueprintExportSettings);
                    GWorld->DestroyActor(Actor);
                    bSuccess = true;
                }

                if (!bDirty)
                {
                    Package->ClearDirtyFlag();
                }
            }
        }
    }

    return bSuccess;
}

bool FSimReadyUSDHelper::ExportObjectAsUSDToPath(class UObject* Object, const FString& InExportPath, const FSimReadyExportSettings& ExportSettings)
{
    if (Object == nullptr)
    {
        return false;
    }

    auto ExportPath = GetLayerPathBySuffix(InExportPath, BaseLayerSuffix, ExportSettings.bEditLayer);

    UExporter::CurrentFilename = ExportPath;

    FSimReadyAssetExportHelper::ResetGlobalCaches();

    FSimReadyExportSettings NewExportSettings = ExportSettings;
    // Correct extension
    NewExportSettings.bAsciiFormat = (FPaths::GetExtension(ExportPath).ToLower() == TEXT("usda"));

    TMap<FString, FString> ExportedMDLs;
    FString ExportMDLBasePath = FPaths::GetPath(ExportPath);

    bool bSuccess = false;
    if (Object->IsA<UStaticMesh>())
    {
        bSuccess = ExportStaticMeshAsUSDToPathInternal(Cast<UStaticMesh>(Object), nullptr, ExportPath, ExportMDLBasePath, false, NewExportSettings, ExportedMDLs);
    }
    else if (Object->IsA<USkeletalMesh>())
    {
        bSuccess = ExportSkeletalMeshAsUSDToPathInternal(Cast<USkeletalMesh>(Object), nullptr, ExportPath, ExportMDLBasePath, false, NewExportSettings, ExportedMDLs);
    }
    else if (Object->IsA<UBlueprint>())
    {
        bSuccess = ExportBlueprintAsUSDToPathInternal(Cast<UBlueprint>(Object), ExportPath, NewExportSettings);
    }
    else if (Object->IsA<UAnimSequence>() || Object->IsA<UAnimMontage>())
    {
        bSuccess = ExportAnimationAssetAsUSDToPathInternal(Cast<UAnimSequenceBase>(Object), nullptr, ExportPath, NewExportSettings);
    }
    else
    {
        bSuccess = ExportObjectAsUSDToPathInternal(Object, ExportPath, NewExportSettings);
    }

    if (ExportSettings.bEditLayer)
    {
        CreateEditLayerIfNotExist(InExportPath, ExportSettings.bUpYAxis);
    }

    return bSuccess;
}

void RestoreSequencerAnimatedState(ULevelSequence* LevelSequence)
{
    if (LevelSequence)
    {
        IAssetEditorInstance* AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(LevelSequence, false);
        ILevelSequenceEditorToolkit* LevelSequenceEditor = static_cast<ILevelSequenceEditorToolkit*>(AssetEditor);
        TWeakPtr<ISequencer> WeakSequencer = LevelSequenceEditor ? LevelSequenceEditor->GetSequencer() : nullptr;

        if (WeakSequencer.IsValid())
        {
            WeakSequencer.Pin()->RestorePreAnimatedState();
        }
    }
}

void FSimReadyUSDHelper::GetActorsToExportFromWorld(class UWorld* World, bool bSelectedOnly, const FSimReadyExportSettings& ExportSettings, TArray<AActor*>& ActorsToExport, TArray<ALevelSequenceActor*>& LevelSequenceActors)
{
    ULevel* Level = World->PersistentLevel;

    if (!ExportSettings.bExportInvisibleLevel && !FLevelUtils::IsLevelVisible(Level))
    {
        return;
    }

    for (int32 ActorIndex = 0; ActorIndex < Level->Actors.Num(); ++ActorIndex)
    {
        AActor* Actor = Level->Actors[ActorIndex];

        if (Actor != NULL && (!bSelectedOnly || (bSelectedOnly && Actor->IsSelected())))
        {
            if (IsActorSupported(Actor, ExportSettings))
            {
                ActorsToExport.Add(Actor);
            }	

            if (Actor->IsA<ALevelSequenceActor>())
            {
                auto LevelSequenceActor = Cast<ALevelSequenceActor>(Actor);
                RestoreSequencerAnimatedState(LevelSequenceActor->GetSequence());
                LevelSequenceActors.Add(LevelSequenceActor);
            }
        }
    }

    // Export streaming levels and actors
    for (int32 CurLevelIndex = 0; CurLevelIndex < World->GetNumLevels(); ++CurLevelIndex)
    {
        ULevel* CurLevel = World->GetLevel(CurLevelIndex);
        if (CurLevel != NULL && CurLevel != Level)
        {
            if (!ExportSettings.bExportInvisibleLevel && !FLevelUtils::IsLevelVisible(CurLevel))
            {
                continue;
            }

            for (int32 ActorIndex = 0; ActorIndex < CurLevel->Actors.Num(); ++ActorIndex)
            {
                AActor* Actor = CurLevel->Actors[ActorIndex];

                if (Actor != NULL && (!bSelectedOnly || (bSelectedOnly && Actor->IsSelected())))
                {
                    if (IsActorSupported(Actor, ExportSettings))
                    {
                        ActorsToExport.Add(Actor);
                    }			

                    if (Actor->IsA<ALevelSequenceActor>())
                    {
                        auto LevelSequenceActor = Cast<ALevelSequenceActor>(Actor);
                        RestoreSequencerAnimatedState(LevelSequenceActor->GetSequence());
                        LevelSequenceActors.Add(LevelSequenceActor);
                    }
                }
            }
        }
    }
}

void SetCameraToCustomLayerData(const pxr::UsdStageRefPtr& USDStage, UWorld* World)
{
    if (World == nullptr)
    {
        return;
    }

    // Just export perspective now
    const FLevelViewportInfo& ViewportInfo = World->EditorViews[ELevelViewportType::LVT_Perspective];
    auto CamPosition = ViewportInfo.CamPosition;
    auto CamRotation = ViewportInfo.CamRotation;
    // It's an unsaved empty map
    if (CamPosition.IsZero() && CamRotation.IsZero())
    {
        FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
        SLevelViewport* LevelViewport = LevelEditor.GetFirstActiveLevelViewport().Get();
        FLevelEditorViewportClient& LevelViewportClient = LevelViewport->GetLevelViewportClient();

        CamPosition = LevelViewportClient.GetViewLocation();
        CamRotation = LevelViewportClient.GetViewRotation();
    }

    auto CamTarget = CamPosition + FTransform(CamRotation).TransformVector(FVector(1, 0, 0)) * 1000.0f;
    
    FUSDCustomLayerData::SetEditorCamera(USDStage, CamPosition, CamTarget);
}

FORCEINLINE float LuminanceToEV100(float Luminance)
{
    // if Luminance was 0, reset EV100 to 0
    return Luminance > 0.0f ? FMath::Log2(Luminance / 1.2f) : 0.0f;
}

//FORCEINLINE float EV100ToLuminance(float EV100)
//{
//	return 1.2 * FMath::Pow(2.0f, EV100);
//}

void FetchSettingsFromPostProcessVolumes(UWorld* World, FPostProcessSettings& PostProcessSettings)
{
    const FVector ViewLocation = World->EditorViews[ELevelViewportType::LVT_Perspective].CamPosition;
    for (IInterface_PostProcessVolume* Volume : World->PostProcessVolumes)
    {
        const FPostProcessVolumeProperties VolumeProperties = Volume->GetProperties();
        if (!VolumeProperties.bIsEnabled)
        {
            continue;
        }

        float DistanceToPoint = 0.0f;
        float LocalWeight = FMath::Clamp(VolumeProperties.BlendWeight, 0.0f, 1.0f);

        if (!VolumeProperties.bIsUnbound)
        {
            float SquaredBlendRadius = VolumeProperties.BlendRadius * VolumeProperties.BlendRadius;
            Volume->EncompassesPoint(ViewLocation, 0.0f, &DistanceToPoint);

            if (DistanceToPoint >= 0)
            {
                if (DistanceToPoint > VolumeProperties.BlendRadius)
                {
                    // outside
                    LocalWeight = 0.0f;
                }
                else
                {
                    // to avoid div by 0
                    if (VolumeProperties.BlendRadius >= 1.0f)
                    {
                        LocalWeight *= 1.0f - DistanceToPoint / VolumeProperties.BlendRadius;

                        check(LocalWeight >= 0 && LocalWeight <= 1.0f);
                    }
                }
            }
            else
            {
                LocalWeight = 0;
            }
        }

        if (LocalWeight > 0)
        {
            OverridePostProcessSettings(*VolumeProperties.Settings, LocalWeight, PostProcessSettings);
        }
    }
}

void SetExposureToCustomLayerData(const pxr::UsdStageRefPtr& USDStage, UWorld* World)
{
    if (World == nullptr)
    {
        return;
    }

    FPostProcessSettings PostProcessSettings;
    PostProcessSettings.SetBaseValues();

    static const auto VarDefaultAutoExposure = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.DefaultFeature.AutoExposure"));
    static const auto VarDefaultAutoExposureMethod = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.DefaultFeature.AutoExposure.Method"));
    static const auto VarDefaultAutoExposureExtendDefaultLuminanceRange = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange"));
    const bool bExtendedLuminanceRange = VarDefaultAutoExposureExtendDefaultLuminanceRange->GetValueOnGameThread() == 1;

    if (!VarDefaultAutoExposure->GetValueOnGameThread())
    {
        PostProcessSettings.AutoExposureMinBrightness = 1;
        PostProcessSettings.AutoExposureMaxBrightness = 1;
        if (bExtendedLuminanceRange)
        {
            PostProcessSettings.AutoExposureMinBrightness = LuminanceToEV100(PostProcessSettings.AutoExposureMinBrightness);
            PostProcessSettings.AutoExposureMaxBrightness = LuminanceToEV100(PostProcessSettings.AutoExposureMaxBrightness);
        }
    }
    else
    {
        int32 Value = VarDefaultAutoExposureMethod->GetValueOnGameThread();
        if (Value >= 0 && Value < AEM_MAX)
        {
            PostProcessSettings.AutoExposureMethod = (EAutoExposureMethod)Value;
        }
    }

    FetchSettingsFromPostProcessVolumes(World, PostProcessSettings);

    FUSDCustomLayerData::SetRenderSettings(USDStage->GetRootLayer(), PostProcessSettings);
}

FSimReadyExportSettings SetRayTracingTranslucencySettings(UWorld * World, const FSimReadyExportSettings& ExportSettings)
{
    if (World == nullptr)
    {
        return ExportSettings;
    }

    FSimReadyExportSettings OutExportSettings = ExportSettings;
    FPostProcessSettings PostProcessSettings;
    PostProcessSettings.SetBaseValues();
    FetchSettingsFromPostProcessVolumes(World, PostProcessSettings);
    OutExportSettings.MaterialSettings.bRayTracingTranslucency = (PostProcessSettings.TranslucencyType == ETranslucencyType::RayTracing);
    OutExportSettings.MaterialSettings.bRayTracingRefraction = PostProcessSettings.RayTracingTranslucencyRefraction;

    return OutExportSettings;
}

bool FSimReadyUSDHelper::ExportUWorldAsUSDToPath(UWorld * World, const FString & InExportWorldStagePath, const FSimReadyExportSettings& ExportSettings)
{
    if (World == nullptr)
    {
        return false;
    }

    auto ExportWorldStagePath = GetLayerPathBySuffix(InExportWorldStagePath, BaseLayerSuffix, ExportSettings.bEditLayer);

    UExporter::CurrentFilename = ExportWorldStagePath;

    // New export settings with ray tracing translucency
    FSimReadyExportSettings NewExportSettings = SetRayTracingTranslucencySettings(World, ExportSettings);
    // Correct extension
    NewExportSettings.bAsciiFormat = (FPaths::GetExtension(ExportWorldStagePath).ToLower() == TEXT("usda"));


    // Export world to USD
    pxr::UsdStageRefPtr USDStage = FSimReadyUSDHelper::CreateUSDStageFromPath(ExportWorldStagePath, IsRunningCommandlet(), !NewExportSettings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }

    SimReadyCarlaWrapper::SpawnSignalsAndTrafficLights(*World);

    FSimReadyAssetExportHelper::ResetGlobalCaches();
    auto RootLayer = USDStage->GetRootLayer();
    pxr::SdfPath RootPrimPath = InitDefaultPrim(USDStage);

    TMap<FString, FString> ExportMeshUSD;
    TMap<FString, FString> ExportMDLs;
    TMap<FString, FString> ExportAnimeUSD;
    TArray<FString> ExportSubstages;

    TArray<AActor*> ActorsToExport;
    TArray<ALevelSequenceActor*> LevelSequenceActors;
    GetActorsToExportFromWorld(World, false, NewExportSettings, ActorsToExport, LevelSequenceActors);

    if (FSequenceExtractor::Get().SetData(World, LevelSequenceActors))
    {
        RootLayer->SetStartTimeCode(FSequenceExtractor::Get().GetMasterStartTimeCode());
        RootLayer->SetEndTimeCode(FSequenceExtractor::Get().GetMasterEndTimeCode());
        RootLayer->SetTimeCodesPerSecond(FSequenceExtractor::Get().GetMasterTimeCodesPerSecond());
        TArray<UObject*> Spawnables;
        FSequenceExtractor::Get().GetSpawnableObjects(Spawnables);
        for(auto SpawnableObject : Spawnables)
        {
            if (SpawnableObject->IsA<AActor>() && IsActorSupported(Cast<AActor>(SpawnableObject), NewExportSettings))
            {
                ActorsToExport.Add(Cast<AActor>(SpawnableObject));
            }
        }
    }

    int32 NumDependencies = FSimReadyReferenceCollector::Get().GetAllDependencies(ActorsToExport, NewExportSettings);
    FSimReadySlowTask::Get().BeginProgress(NumDependencies + ActorsToExport.Num() + World->GetLevels().Num() + 1 /*usd saving*/ + 1/*checkpoint*/, FText::FromString(FString::Printf(TEXT("Start Exporting Map %s..."), *World->GetName())));

    pxr::UsdStageRefPtr MaterialOversStage = nullptr;
    FString MaterialOverSublayerPath;
    if (NewExportSettings.bMaterialOverSublayer && NewExportSettings.bMDL)
    {
        // create material overrides sublayer
        FString MaterialOverSublayerName = FPaths::GetBaseFilename(ExportWorldStagePath) + TEXT("_MaterialOvers") + GetUSDExtension(NewExportSettings.bAsciiFormat);
        MaterialOverSublayerPath = FPaths::GetPath(ExportWorldStagePath) / MaterialOverSublayerName;
        
        MaterialOversStage = CreateUSDStageFromPath(MaterialOverSublayerPath, true, !NewExportSettings.bUpYAxis);
        if (MaterialOversStage)
        {
            RootLayer->InsertSubLayerPath(TCHAR_TO_UTF8(*MaterialOverSublayerName));
        }
    }

    SimReadyCarlaWrapper::SimReadyLoadXodr(*World);

    TMap<ULevel*, pxr::UsdStageRefPtr> SubStages;
    ULevel* LastLevel = nullptr;
    for (auto Actor : ActorsToExport)
    {
        FString Text = FString::Printf(TEXT("Exporting Actor %s..."), *Actor->GetName());
        FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));

        if (FSimReadySlowTask::Get().IsProgressCancelled())
        {
            break;
        }

        if (Actor->IsHidden())
        {
            if (FSequenceExtractor::Get().GetVisibilityTimeSamples(Actor) == nullptr)
            {
                continue;
            }
        }

        auto ExportPrimPath = RootPrimPath;
        auto ExportUSDStage = USDStage;

        // Actors from sublevel should be attached to scope
        ULevel* ActorLevel = Cast<ULevel>(Actor->GetOuter());
        if (NewExportSettings.bExportSublayers && ActorLevel != World->PersistentLevel)
        {
            if (ActorLevel != LastLevel)
            {
                FString SubLayerName = Cast<UWorld>(ActorLevel->GetOuter())->GetMapName() + TEXT(".") + FPaths::GetExtension(ExportWorldStagePath);
                FString SubLayerPath = FPaths::GetPath(ExportWorldStagePath) / SubLayerName;
                
                ExportUSDStage = CreateUSDStageFromPath(SubLayerPath, false, !NewExportSettings.bUpYAxis);
                if (ExportUSDStage)
                {
                    RootLayer->InsertSubLayerPath(TCHAR_TO_UTF8(*SubLayerName));
                    SubStages.Add(ActorLevel, ExportUSDStage);
                    ExportSubstages.Add(SubLayerPath);
                    LastLevel = ActorLevel;
                }
            }
            else
            {
                ExportUSDStage = *SubStages.Find(ActorLevel);
            }

            UWorld* SubLevel = Cast<UWorld>(ActorLevel->GetOuter());
            check(SubLevel);
            ExportPrimPath = ExportPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_UTF8(*SubLevel->GetMapName())));
            auto Xform = pxr::UsdGeomXform::Define(ExportUSDStage, ExportPrimPath);
            pxr::UsdModelAPI(Xform).SetKind(pxr::KindTokens->group);
        }

        // Create Scope by Folder Path
        auto FolderPath = Actor->GetFolderPath();
        if (!FolderPath.IsNone())
        {
            TArray<FString> Folders;
            FolderPath.ToString().ParseIntoArray(Folders, TEXT("/"));

            for (auto Folder : Folders)
            {
                ExportPrimPath = ExportPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_UTF8(*Folder)));
                auto Scope = pxr::UsdGeomScope::Define(ExportUSDStage, ExportPrimPath);
                pxr::UsdModelAPI(Scope).SetKind(pxr::KindTokens->group);
            }
        }

        USceneComponent* SceneComp = Cast<USceneComponent>(Actor->GetRootComponent());
        const bool bIsRootComp = SceneComp && !SceneComp->GetAttachParent();
        if (bIsRootComp)
        {	
            FSimReadyUSDHelper::ExportComponentToUSD(SceneComp, ExportUSDStage, MaterialOversStage, ExportPrimPath, NewExportSettings, ExportMeshUSD, ExportMDLs, ExportAnimeUSD);
        }

        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        if (StaticMeshComponents.Num() > 0)
        {			
            for (auto StaticMeshComponent : StaticMeshComponents)
            {
                if (StaticMeshComponent == Actor->GetRootComponent() || StaticMeshComponent->GetAttachParent())
                {
                    continue;
                }

                FSimReadyUSDHelper::ExportComponentToUSD(StaticMeshComponent, ExportUSDStage, MaterialOversStage, ExportPrimPath, NewExportSettings, ExportMeshUSD, ExportMDLs, ExportAnimeUSD);
            }
        }
    }

    // Export BSP
    for(auto Level : World->GetLevels())
    {
        FString Text = FString::Printf(TEXT("Exporting BSP %s..."), *Level->Model->GetName());
        FSimReadySlowTask::Get().UpdateProgress(1, FText::FromString(Text));
        if (FSimReadySlowTask::Get().IsProgressCancelled())
        {
            break;
        }
        ExportModelToUSD(Level->Model, USDStage, MaterialOversStage, NewExportSettings, ExportMeshUSD, ExportMDLs);
    }

    auto SetViewportCallback = [&](UObject* CameraObject, const TArray<FTimeSampleRange>& TimeSamples)
    {
        FUSDExporter Exporter;
        Exporter.Stage = USDStage;
        Exporter.RootPrim = USDStage->GetPrimAtPath(RootPrimPath);
        Exporter.ExportViewportCameraShot(Cast<AActor>(CameraObject), TimeSamples);
    };

    FSimReadySlowTask::Get().UpdateProgress(1, FText::FromString(TEXT("Saving USD...")));

    FSequenceExtractor::Get().SetViewportShotsTimeSamples(SetViewportCallback);

    if (NewExportSettings.bCustomLayer)
    {
        SetCameraToCustomLayerData(USDStage, World);
        //SetExposureToCustomLayerData(USDStage, World);
    }

    if (MaterialOversStage)
    {
        MaterialOversStage->Save();
    }

    for (auto Pair : SubStages)
    {
        // Export the same custom layer to sublayer
        if (NewExportSettings.bCustomLayer)
        {
            SetCameraToCustomLayerData(Pair.Value, World);
            //SetExposureToCustomLayerData(Pair.Value, World);
        }

        Pair.Value->Save();
    }

    PostExportWorld(*World, *USDStage, NewExportSettings.bExportXodr);

    SimReadyCarlaWrapper::DestroySignalsAndTrafficLights(*World);

    USDStage->Save();

    FSimReadySlowTask::Get().EndProgress();
    FSimReadyReferenceCollector::Get().Reset();

    FSequenceExtractor::Get().ClearData();

    if (ExportSettings.bEditLayer)
    {
        CreateEditLayerIfNotExist(InExportWorldStagePath, ExportSettings.bUpYAxis);
    }

    return true;
}

void ExportSimReadyTrafficLight(const pxr::UsdPrim& Prim, const AActor& Actor)
{
    if (!SimReadyCarlaWrapper::IsTrafficLight(Actor))
    {
        return;
    }

    UE_LOG(LogSimReadyUsd, Log, TEXT("Found TrafficLightBase: %s"), *Actor.GetFullName());

    // Add XODR traffic signal ID
    auto IdStr = SimReadyCarlaWrapper::SimReadyFindXodrTrafficSignalID(Actor.GetActorLocation());
    if (IdStr)
    {
        int64 Id = 0;
        LexFromString(Id, ANSI_TO_TCHAR(IdStr));
        auto Attr = Prim.CreateAttribute(SimReadyTokens::SignalId, pxr::SdfValueTypeNames->Int64, true);
        Attr.Set(static_cast<int64_t>(Id));
    }
    else
    {
        UE_LOG(LogSimReadyUsd, Error, TEXT("Can't find XODR ID for traffic signal %s"), *Actor.GetFullName());
    }

    // Add signal type
    auto SignalTypeAttr = Prim.CreateAttribute(SimReadyTokens::SignalType, pxr::SdfValueTypeNames->Token, true);
    SignalTypeAttr.Set(SimReadyTokens::TrafficLight);

    // Set order
    auto SignalOrderAttr = Prim.CreateAttribute(SimReadyTokens::SignalOrder, pxr::SdfValueTypeNames->TokenArray, true);
    SignalOrderAttr.Set(pxr::VtTokenArray{ SimReadyTokens::Red, SimReadyTokens::Amber, SimReadyTokens::Green });

    // Warn if no traffic lights are found
    bool FoundSignal = false;
    for (auto SubPrim : pxr::UsdPrimRange(Prim))
    {
        if (pxr::UsdGeomMesh(SubPrim))
        {
            auto SignalAttr = SubPrim.GetAttribute(SimReadyTokens::Signal);
            if (SignalAttr)
            {
                FoundSignal = true;
                break;
            }
        }
    }

    if (!FoundSignal)
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("Couldn't find any signal light meshes for traffic light actor %s"), *Actor.GetFullName());
    }
}

bool FSimReadyUSDHelper::ExportActorsAsUSDToPath(const TArray<class AActor*>& Actors, const FString & InExportUSDPath, const FSimReadyExportSettings& ExportSettings)
{
    auto ExportUSDPath = GetLayerPathBySuffix(InExportUSDPath, BaseLayerSuffix, ExportSettings.bEditLayer);

    UExporter::CurrentFilename = ExportUSDPath;

    FSimReadyExportSettings NewExportSettings = ExportSettings;
    // Correct extension
    NewExportSettings.bAsciiFormat = (FPaths::GetExtension(ExportUSDPath).ToLower() == TEXT("usda"));

    // export world to USD
    pxr::UsdStageRefPtr USDStage = FSimReadyUSDHelper::CreateUSDStageFromPath(ExportUSDPath, IsRunningCommandlet(), !NewExportSettings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }

    FSimReadyAssetExportHelper::ResetGlobalCaches();

    pxr::SdfPath RootPrimPath = InitDefaultPrim(USDStage);

    TArray<AActor*> ActorsToExport;
    TArray<ALevelSequenceActor*> LevelSequenceActors;
    for (auto Actor : Actors)
    {
        if (Actor->IsA<ALevelSequenceActor>())
        {
            LevelSequenceActors.Add(Cast<ALevelSequenceActor>(Actor));
        }

        if (IsActorSupported(Actor, NewExportSettings))
        {
            ActorsToExport.Add(Actor);
        }
    }

    if (FSequenceExtractor::Get().SetData(nullptr, LevelSequenceActors))
    {
        auto RootLayerHandle = USDStage->GetRootLayer();
        RootLayerHandle->SetStartTimeCode(FSequenceExtractor::Get().GetMasterStartTimeCode());
        RootLayerHandle->SetEndTimeCode(FSequenceExtractor::Get().GetMasterEndTimeCode());
        RootLayerHandle->SetTimeCodesPerSecond(FSequenceExtractor::Get().GetMasterTimeCodesPerSecond());

        TArray<UObject*> Spawnables;
        FSequenceExtractor::Get().GetSpawnableObjects(Spawnables);
        for(auto SpawnableObject : Spawnables)
        {
            if (SpawnableObject->IsA<AActor>() && IsActorSupported(Cast<AActor>(SpawnableObject), NewExportSettings))
            {
                ActorsToExport.Add(Cast<AActor>(SpawnableObject));
            }
        }
    }

    int32 NumDependencies = FSimReadyReferenceCollector::Get().GetAllDependencies(ActorsToExport, NewExportSettings);
    FSimReadySlowTask::Get().BeginProgress(NumDependencies + ActorsToExport.Num() + 1 /*usd saving*/ + 1/*checkpoint*/, FText::FromString(TEXT("Start Exporting Actors ...")));

    if (ActorsToExport.Num() > 0)
    {
        if (ActorsToExport[0]->GetWorld())
        {
            SimReadyCarlaWrapper::SimReadyLoadXodr(*ActorsToExport[0]->GetWorld());
        }
    }

    TMap<FString, FString> ExportMeshUSD;
    TMap<FString, FString> ExportMDLs;
    TMap<FString, FString> ExportAnimeUSD;
    for (auto Actor : ActorsToExport)
    {
        FString Text = FString::Printf(TEXT("Exporting Actor %s..."), *Actor->GetName());
        FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));

        if (FSimReadySlowTask::Get().IsProgressCancelled())
        {
            break;
        }

        if (Actor->IsHidden())
        {
            if (FSequenceExtractor::Get().GetVisibilityTimeSamples(Actor) == nullptr)
            {
                continue;
            }
        }

        USceneComponent* SceneComp = Cast<USceneComponent>(Actor->GetRootComponent());
        const bool bIsRootComp = SceneComp && !SceneComp->GetAttachParent();
        if (bIsRootComp)
        {
            FSimReadyUSDHelper::ExportComponentToUSD(SceneComp, USDStage, nullptr, RootPrimPath, NewExportSettings, ExportMeshUSD, ExportMDLs, ExportAnimeUSD);
        }

        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

        if (StaticMeshComponents.Num() > 0)
        {
            for (auto StaticMeshComponent : StaticMeshComponents)
            {
                if (StaticMeshComponent == Actor->GetRootComponent() || StaticMeshComponent->GetAttachParent())
                {
                    continue;
                }

                FSimReadyUSDHelper::ExportComponentToUSD(StaticMeshComponent, USDStage, nullptr, RootPrimPath, NewExportSettings, ExportMeshUSD, ExportMDLs, ExportAnimeUSD);
            }
        }
    }

    auto SetViewportCallback = [&](UObject* CameraObject, const TArray<FTimeSampleRange>& TimeSamples)
    {
        if (ActorsToExport.Find(Cast<AActor>(CameraObject)) != INDEX_NONE)
        {
            FUSDExporter Exporter;
            Exporter.Stage = USDStage;
            Exporter.RootPrim = USDStage->GetPrimAtPath(RootPrimPath);
            Exporter.ExportViewportCameraShot(Cast<AActor>(CameraObject), TimeSamples);
        }
    };

    FSimReadySlowTask::Get().UpdateProgress(1, FText::FromString(TEXT("Saving USD...")));

    FSequenceExtractor::Get().SetViewportShotsTimeSamples(SetViewportCallback);

    if (NewExportSettings.bCustomLayer && ActorsToExport.Num() > 0)
    {
        SetCameraToCustomLayerData(USDStage, ActorsToExport[0]->GetWorld());
        //SetExposureToCustomLayerData(USDStage, ActorsToExport[0]->GetWorld());
    }

    if (ActorsToExport.Num() > 0 && ActorsToExport[0]->GetWorld())
    {
        PostExportWorld(*ActorsToExport[0]->GetWorld(), *USDStage, NewExportSettings.bExportXodr);
    }

    USDStage->Save();

    FSimReadySlowTask::Get().EndProgress();
    FSimReadyReferenceCollector::Get().Reset();

    FSequenceExtractor::Get().ClearData();

    if (ExportSettings.bEditLayer)
    {
        CreateEditLayerIfNotExist(InExportUSDPath, ExportSettings.bUpYAxis);
    }

    return true;
}

bool ExportLandscapeToMeshDescription(ALandscapeProxy* Landscape, FMeshDescription& OutRawMesh)
{
    TInlineComponentArray<ULandscapeComponent*> RegisteredLandscapeComponents;
    Landscape->GetComponents<ULandscapeComponent>(RegisteredLandscapeComponents);

    const FIntRect LandscapeSectionRect = Landscape->GetBoundingRect();
    const FVector2D LandscapeUVScale = FVector2D(1.0f, 1.0f) / FVector2D(LandscapeSectionRect.Size());

    TVertexAttributesRef<FVector> VertexPositions = OutRawMesh.VertexAttributes().GetAttributesRef<FVector>(MeshAttribute::Vertex::Position);
    TEdgeAttributesRef<bool> EdgeHardnesses = OutRawMesh.EdgeAttributes().GetAttributesRef<bool>(MeshAttribute::Edge::IsHard);
    TEdgeAttributesRef<float> EdgeCreaseSharpnesses = OutRawMesh.EdgeAttributes().GetAttributesRef<float>(MeshAttribute::Edge::CreaseSharpness);
    TPolygonGroupAttributesRef<FName> PolygonGroupImportedMaterialSlotNames = OutRawMesh.PolygonGroupAttributes().GetAttributesRef<FName>(MeshAttribute::PolygonGroup::ImportedMaterialSlotName);
    TVertexInstanceAttributesRef<FVector> VertexInstanceNormals = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<FVector>(MeshAttribute::VertexInstance::Normal);
    TVertexInstanceAttributesRef<FVector> VertexInstanceTangents = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<FVector>(MeshAttribute::VertexInstance::Tangent);
    TVertexInstanceAttributesRef<float> VertexInstanceBinormalSigns = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<float>(MeshAttribute::VertexInstance::BinormalSign);
    TVertexInstanceAttributesRef<FVector4> VertexInstanceColors = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<FVector4>(MeshAttribute::VertexInstance::Color);
    TVertexInstanceAttributesRef<FVector2D> VertexInstanceUVs = OutRawMesh.VertexInstanceAttributes().GetAttributesRef<FVector2D>(MeshAttribute::VertexInstance::TextureCoordinate);

    if (VertexInstanceUVs.GetNumIndices() < 2)
    {
        VertexInstanceUVs.SetNumIndices(2);
    }

    // User specified LOD to export
    int32 LandscapeLODToExport = Landscape->GetLandscapeActor() ? Landscape->GetLandscapeActor()->ExportLOD : 0;
    float AmountOfWork = RegisteredLandscapeComponents.Num() * 2 + 1;
    FString Text = FString::Printf(TEXT("Converting Landscape %s..."), *Landscape->GetName());
    FScopedSlowTask ConvertLandscapeTask(AmountOfWork, FText::FromString(Text));
    ConvertLandscapeTask.Visibility = ESlowTaskVisibility::ForceVisible;
    ConvertLandscapeTask.MakeDialog(false);
    ConvertLandscapeTask.EnterProgressFrame();

    // Export data for each component
    TMap<FVector, FVertexID> PositionToVertexID;
    for (auto It = RegisteredLandscapeComponents.CreateConstIterator(); It; ++It)
    {
        ULandscapeComponent* Component = (*It);

        FLandscapeComponentDataInterface CDI(Component, LandscapeLODToExport);
        const int32 ComponentSizeQuadsLOD = ((Component->ComponentSizeQuads + 1) >> LandscapeLODToExport) - 1;
        const int32 SubsectionSizeQuadsLOD = ((Component->SubsectionSizeQuads + 1) >> LandscapeLODToExport) - 1;
        const FIntPoint ComponentOffsetQuads = Component->GetSectionBase() - Landscape->LandscapeSectionOffset - LandscapeSectionRect.Min;
        const FVector2D ComponentUVOffsetLOD = FVector2D(ComponentOffsetQuads)*((float)ComponentSizeQuadsLOD / Component->ComponentSizeQuads);
        const FVector2D ComponentUVScaleLOD = LandscapeUVScale*((float)Landscape->ComponentSizeQuads / ComponentSizeQuadsLOD);
        const FTransform& RelativeTransform = Component->GetRelativeTransform();

        const int32 NumFaces = FMath::Square(ComponentSizeQuadsLOD) * 2;
        const int32 NumVertices = FMath::Square(ComponentSizeQuadsLOD + 1);
        const int32 NumIndices = NumFaces * 3;

        OutRawMesh.ReserveNewVertices(NumVertices);
        OutRawMesh.ReserveNewPolygons(NumFaces);
        OutRawMesh.ReserveNewVertexInstances(NumIndices);
        OutRawMesh.ReserveNewEdges(NumIndices);

        FPolygonGroupID PolygonGroupID = OutRawMesh.CreatePolygonGroup();
        const FName ImportedSlotName(*FString::Printf(TEXT("LandscapeMat_%d"), PolygonGroupID.GetValue()));
        PolygonGroupImportedMaterialSlotNames[PolygonGroupID] = ImportedSlotName;

        auto FindVertexID = [&PositionToVertexID](const FVector& Position)->FVertexID
        {
            auto VertexID = PositionToVertexID.Find(Position);
            if(VertexID)
            {
                return *VertexID;
            }

            return FVertexID();
        };

        for (int32 y = 0; y < ComponentSizeQuadsLOD + 1; y++)
        {
            for (int32 x = 0; x < ComponentSizeQuadsLOD + 1; x++)
            {
                FVector Position = RelativeTransform.TransformPosition(CDI.GetLocalVertex(x, y));
                if (FindVertexID(Position) == FVertexID::Invalid)
                {
                    FVertexID VertexID = OutRawMesh.CreateVertex();
                    VertexPositions[VertexID] = Position;
                    PositionToVertexID.Add(Position, VertexID);
                }
            }
        }

        ConvertLandscapeTask.EnterProgressFrame();

        if (ConvertLandscapeTask.ShouldCancel())
        {
            break;
        }

        const int QuadIndices[6] = 
        {
            0,
            1,
            2,
            0,
            2,
            3,
        };

        const FIntPoint QuadVertices[4] =
        {
            FIntPoint(0, 0),
            FIntPoint(0, 1),
            FIntPoint(1, 1),
            FIntPoint(1, 0),
        };

        // Export to MeshDescription
        for (int32 y = 0; y < ComponentSizeQuadsLOD; y++)
        {
            for (int32 x = 0; x < ComponentSizeQuadsLOD; x++)
            {
                //Fill the vertexID we need
                TArray<FVertexID> VertexIDs;
                VertexIDs.Reserve(UE_ARRAY_COUNT(QuadIndices));
                TArray<FVertexInstanceID> VertexInstanceIDs;
                VertexInstanceIDs.Reserve(UE_ARRAY_COUNT(QuadIndices));
                // Fill positions
                for (int32 i = 0; i < UE_ARRAY_COUNT(QuadIndices); i++)
                {
                    int32 VertexX = x + QuadVertices[QuadIndices[i]].X;
                    int32 VertexY = y + QuadVertices[QuadIndices[i]].Y;
                    FVector Positions = RelativeTransform.TransformPosition(CDI.GetLocalVertex(VertexX, VertexY));

                    FVertexID VertexID = FindVertexID(Positions);
                    if (VertexID == FVertexID::Invalid)
                    {
                        UE_LOG(LogSimReadyUsd, Display, TEXT("Convert Landscape: Invalid VertexID at %d %d"), VertexX, VertexY);
                    }
                    VertexIDs.Add(VertexID);
                }

                // Create triangle
                {
                    // Whether this vertex is in hole
                    bool bInvisible = false;
#if 0
                    if (VisDataMap.Num())
                    {
                        int32 TexelX, TexelY;
                        CDI.VertexXYToTexelXY(x, y, TexelX, TexelY);
                        bInvisible = (VisDataMap[CDI.TexelXYToIndex(TexelX, TexelY)] >= VisThreshold);
                    }
#endif
                    //Add vertexInstance and polygon only if we are visible
                    if (!bInvisible)
                    {
                        VertexInstanceIDs.Add(OutRawMesh.CreateVertexInstance(VertexIDs[0]));
                        VertexInstanceIDs.Add(OutRawMesh.CreateVertexInstance(VertexIDs[1]));
                        VertexInstanceIDs.Add(OutRawMesh.CreateVertexInstance(VertexIDs[2]));

                        VertexInstanceIDs.Add(OutRawMesh.CreateVertexInstance(VertexIDs[3]));
                        VertexInstanceIDs.Add(OutRawMesh.CreateVertexInstance(VertexIDs[4]));
                        VertexInstanceIDs.Add(OutRawMesh.CreateVertexInstance(VertexIDs[5]));

                        // Fill other vertex data
                        for (int32 i = 0; i < UE_ARRAY_COUNT(QuadIndices); i++)
                        {
                            int32 VertexX = x + QuadVertices[QuadIndices[i]].X;
                            int32 VertexY = y + QuadVertices[QuadIndices[i]].Y;

                            FVector LocalTangentX, LocalTangentY, LocalTangentZ;
                            CDI.GetLocalTangentVectors(VertexX, VertexY, LocalTangentX, LocalTangentY, LocalTangentZ);

                            VertexInstanceTangents[VertexInstanceIDs[i]] = LocalTangentX;
                            VertexInstanceBinormalSigns[VertexInstanceIDs[i]] = GetBasisDeterminantSign(LocalTangentX, LocalTangentY, LocalTangentZ);
                            VertexInstanceNormals[VertexInstanceIDs[i]] = LocalTangentZ;

                            FVector2D UV = (ComponentUVOffsetLOD + FVector2D(VertexX, VertexY));
                            VertexInstanceUVs.Set(VertexInstanceIDs[i], 0, UV);
                            UV *= ComponentUVScaleLOD;
                            VertexInstanceUVs.Set(VertexInstanceIDs[i], 1, UV);

                        }
                        auto AddTriangle = [&OutRawMesh, &EdgeHardnesses, &EdgeCreaseSharpnesses, &PolygonGroupID, &VertexIDs, &VertexInstanceIDs](int32 BaseIndex)
                        {
                            //Create a polygon from this triangle
                            TArray<FVertexInstanceID> PerimeterVertexInstances;
                            PerimeterVertexInstances.SetNum(3);
                            for (int32 Corner = 0; Corner < 3; ++Corner)
                            {
                                PerimeterVertexInstances[Corner] = VertexInstanceIDs[BaseIndex + Corner];
                            }
                            // Insert a polygon into the mesh
                            TArray<FEdgeID> NewEdgeIDs;
                            const FPolygonID NewPolygonID = OutRawMesh.CreatePolygon(PolygonGroupID, PerimeterVertexInstances, &NewEdgeIDs);
                            for (const FEdgeID NewEdgeID : NewEdgeIDs)
                            {
                                EdgeHardnesses[NewEdgeID] = false;
                                EdgeCreaseSharpnesses[NewEdgeID] = 0.0f;
                            }
                        };
                        AddTriangle(0);
                        AddTriangle(3);
                    }
                }
            }
        }

        ConvertLandscapeTask.EnterProgressFrame();

        if (ConvertLandscapeTask.ShouldCancel())
        {
            break;
        }
    }

    //Compact the MeshDescription, if there was visibility mask or some bounding box clip, it need to be compacted so the sparse array are from 0 to n with no invalid data in between. 
    FElementIDRemappings ElementIDRemappings;
    OutRawMesh.Compact(ElementIDRemappings);
    return OutRawMesh.Polygons().Num() > 0;
}

void ExportLayerWeightmap(ULandscapeInfo* LandscapeInfo, ULandscapeLayerInfoObject* LandscapeLayerInfoObject, const FString& OmniPath)
{
    int32 MinX = MAX_int32;
    int32 MinY = MAX_int32;
    int32 MaxX = -MAX_int32;
    int32 MaxY = -MAX_int32;

    if (!LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY))
    {
        return;
    }

    TArray<uint8> WeightData;
    WeightData.AddZeroed((MaxX - MinX + 1) * (MaxY - MinY + 1));

    FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);
    LandscapeEdit.GetWeightDataFast(LandscapeLayerInfoObject, MinX, MinY, MaxX, MaxY, WeightData.GetData(), 0);
    
    FSimReadyAssetExportHelper::ExportRawDataToPath(WeightData.GetData(), WeightData.Num(), OmniPath, (uint32)(MaxX - MinX + 1), (uint32)(MaxY - MinY + 1), ERGBFormat::Gray, 8);
}

void ExportHeightNormalmap(ULandscapeInfo* LandscapeInfo, const FString& OmniPath)
{
    int32 MinX = MAX_int32;
    int32 MinY = MAX_int32;
    int32 MaxX = -MAX_int32;
    int32 MaxY = -MAX_int32;

    if (!LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY))
    {
        return;
    }

    TArray<uint16> HeightData;
    HeightData.AddZeroed((MaxX - MinX + 1) * (MaxY - MinY + 1));

    TArray<uint16> NormalData;
    NormalData.AddZeroed((MaxX - MinX + 1) * (MaxY - MinY + 1));
    FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);
    LandscapeEdit.GetHeightDataFast(MinX, MinY, MaxX, MaxY, HeightData.GetData(), 0, NormalData.GetData());

    TArray<uint8> Normalmap;
    Normalmap.AddZeroed((MaxX - MinX + 1) * (MaxY - MinY + 1) * 4);

    for(int32 Index = 0; Index < NormalData.Num(); ++Index)
    {
        float X = 2.f * (float)((NormalData[Index] >> 8) & 0x00FF) / 255.f - 1.f;
        float Y = 2.f * (float)(NormalData[Index] & 0x00FF) / 255.f - 1.f;
        float Z = FMath::Sqrt(1.f - (FMath::Square(X)+FMath::Square(Y)));

        Normalmap[Index * 4] = (uint8)((Z * 0.5f + 0.5f) * 255.f);
        Normalmap[Index * 4 + 1] = (uint8)((Y * 0.5f + 0.5f) * 255.f);
        Normalmap[Index * 4 + 2] = (uint8)((X * 0.5f + 0.5f) * 255.f);
        Normalmap[Index * 4 + 3] = 255;
    }

    FSimReadyAssetExportHelper::ExportRawDataToPath(Normalmap.GetData(), Normalmap.Num(), OmniPath, (uint32)(MaxX - MinX + 1), (uint32)(MaxY - MinY + 1), ERGBFormat::BGRA, 8);
}

bool FSimReadyUSDHelper::ExportConvertedMeshToUSDInternal(USceneComponent* SceneComponent,
    const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
    const FSimReadyExportSettings& ExportSettings,
    TMap<FString, FString>& ExportedMDLs, const FString& ExportMeshPath)
{
    if (SceneComponent == nullptr)
    {
        return false;
    }

    auto TextRenderComponent = Cast<UTextRenderComponent>(SceneComponent);
    auto DecalComponent = Cast<UDecalComponent>(SceneComponent);
    if (TextRenderComponent == nullptr && DecalComponent == nullptr)
    {
        return false;
    }

    pxr::UsdStageRefPtr USDStage = CreateUSDStageFromPath(ExportMeshPath, false, !ExportSettings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }
    pxr::SdfPath RootPrimPath = InitDefaultPrim(USDStage, pxr::KindTokens->component);
    auto PrimPath = RootPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SceneComponent->GetName())));

    auto Mesh = FSimReadyUSDHelper::ExportConvertedMeshToUSD(SceneComponent, USDStage, PrimPath, ExportMDLBaseAbsolutePath, ExportMDLBaseRelativePath, ExportSettings, ExportedMDLs);
    if (Mesh)
    {
        USDStage->Save();
        return true;
    }

    return false;
}

pxr::UsdGeomXformable FSimReadyUSDHelper::ExportConvertedMeshToUSD(USceneComponent* SceneComponent, const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& ExportPrimPath,
    const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
    const FSimReadyExportSettings& ExportSettings,
    TMap<FString, FString>& ExportedMDLs)
{
    pxr::UsdGeomXformable USDMesh;

    if (SceneComponent == nullptr)
    {
        return USDMesh;
    }

    auto TextRenderComponent = Cast<UTextRenderComponent>(SceneComponent);
    auto DecalComponent = Cast<UDecalComponent>(SceneComponent);
    if (TextRenderComponent == nullptr && DecalComponent == nullptr)
    {
        return USDMesh;
    }

    if (USDStage)
    {
        FMeshDescription MeshDescription;
        FStaticMeshAttributes(MeshDescription).Register();
        if (TextRenderComponent)
        {
            if (ExportStringMeshToMeshDescription(TextRenderComponent, MeshDescription))
            {
                USDMesh = FUSDExporter::ExportMeshDescription(USDStage, ExportPrimPath, MeshDescription);
            }
        }
        else
        {
            if (ExportDecalToMeshDescription(DecalComponent, MeshDescription))
            {
                USDMesh = FUSDExporter::ExportMeshDescription(USDStage, ExportPrimPath, MeshDescription);
            }
        }

        if (USDMesh)
        {
            auto Material = TextRenderComponent ? TextRenderComponent->TextMaterial : DecalComponent->GetDecalMaterial();
            if (Material)
            {
                ExportAndBindMaterial(USDStage, Material, nullptr, USDMesh.GetPrim(), ExportSettings, ExportMDLBaseAbsolutePath, ExportMDLBaseRelativePath, ExportedMDLs, TEXT(""), TEXT(""),
                    nullptr,			
                    [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FString& MDLName, const FSimReadyExportMaterialResult& MaterialResult, const FString& UsdMaterialName)
                    {
                        BindNumTexCoord(InStage, InPrimPath, MeshDescription, MaterialResult);
                    }
                );	
            }
        }
    }

    return USDMesh;
}

bool FSimReadyUSDHelper::ExportLandscapeToUSDInternal(class ALandscapeProxy* LandscapeProxy,
    const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
    const FSimReadyExportSettings& ExportSettings,
    TMap<FString, FString>& ExportedMDLs, TMap<FString, FString>& ExportedMeshUSD, const FString& ExportMeshPath)
{
    if (!LandscapeProxy)
    {
        return false;
    }

    pxr::UsdStageRefPtr USDStage = CreateUSDStageFromPath(ExportMeshPath, false, !ExportSettings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }
    pxr::SdfPath RootPrimPath = InitDefaultPrim(USDStage, pxr::KindTokens->component);
    auto PrimPath = RootPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*LandscapeProxy->GetName())));

    auto Mesh = FSimReadyUSDHelper::ExportLandscapeToUSD(LandscapeProxy, USDStage, PrimPath, ExportMDLBaseAbsolutePath, ExportMDLBaseRelativePath, ExportSettings, ExportedMDLs, ExportedMeshUSD, true, ExportMeshPath);
    if (Mesh)
    {
        USDStage->Save();
        return true;
    }

    return false;
}

pxr::UsdGeomXformable FSimReadyUSDHelper::ExportLandscapeToUSD(class ALandscapeProxy* LandscapeProxy, const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& ExportPrimPath,
            const FString& ExportMDLBaseAbsolutePath, const FString& ExportMDLBaseRelativePath,
            const FSimReadyExportSettings& ExportSettings,
            TMap<FString, FString>& ExportedMDLs, TMap<FString, FString>& ExportedMeshUSD, bool bStandaloneUSD, const FString& ExportMeshPath)
{
    pxr::UsdGeomXformable USDMesh;

    if (!LandscapeProxy)
    {
        return USDMesh;
    }

    if (USDStage)
    {
        FMeshDescription MeshDescription;
        FStaticMeshAttributes(MeshDescription).Register();
        if (ExportLandscapeToMeshDescription(LandscapeProxy, MeshDescription))
        {
            FBox WorldBounds(ForceInit);
            TInlineComponentArray<ULandscapeComponent*> RegisteredLandscapeComponents;
            LandscapeProxy->GetComponents<ULandscapeComponent>(RegisteredLandscapeComponents);

            USDMesh = FUSDExporter::ExportMeshDescription(USDStage, ExportPrimPath, MeshDescription, false, true);
            if (USDMesh)
            {
                auto GeomSubsets = pxr::UsdGeomSubset::GetAllGeomSubsets(pxr::UsdGeomImageable(USDMesh.GetPrim()));

                for (int32 CompIndex = 0; CompIndex < RegisteredLandscapeComponents.Num(); ++CompIndex)
                {
                    WorldBounds += RegisteredLandscapeComponents[CompIndex]->CalcBounds(RegisteredLandscapeComponents[CompIndex]->GetComponentTransform()).GetBox();
                    if (RegisteredLandscapeComponents[CompIndex]->MaterialInstances.Num() > 0)
                    {
                        auto LandscapeMaterialInst = RegisteredLandscapeComponents[CompIndex]->MaterialInstances[0];
                        auto LandscapeMaterial = LandscapeProxy->LandscapeMaterial;
                        if (LandscapeMaterialInst && LandscapeMaterial)
                        {
                            auto SubsetPrim = GeomSubsets[CompIndex].GetPrim();
                            GeomSubsets[CompIndex].GetFamilyNameAttr().Set(pxr::UsdShadeTokens->materialBind);

                            TCHAR* End = nullptr;
                            FString CustomMaterialName(LandscapeMaterial->GetName() + FString::FromInt(FCString::Strtoi(*LandscapeMaterialInst->GetName(), &End, 10)));

                            ExportAndBindMaterial(USDStage, 
                                LandscapeMaterialInst, 
                                nullptr, 
                                SubsetPrim, 
                                ExportSettings, 
                                ExportMDLBaseAbsolutePath, 
                                ExportMDLBaseRelativePath, 
                                ExportedMDLs,
                                CustomMaterialName,
                                TEXT(""),
                                nullptr,
                                [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FString& MDLName, const FSimReadyExportMaterialResult& MaterialResult, const FString& UsdMaterialName)
                            {
                                ULandscapeInfo* LandscapeInfo = LandscapeProxy->GetLandscapeInfo();
                                if (LandscapeInfo)
                                {
                                    auto UsdMaterial = pxr::UsdShadeMaterial::Get(InStage, InPrimPath);
                                    // UsdMaterial might be failed
                                    if (UsdMaterial)
                                    {
                                        pxr::UsdShadeConnectableAPI Source;
                                        pxr::TfToken SourceName;
                                        pxr::UsdShadeAttributeType SourceType;
                                        UsdMaterial.GetSurfaceOutput(USDTokens.mdl).GetConnectedSource(&Source, &SourceName, &SourceType);
                                        auto MdlSurfaceShader = pxr::UsdShadeShader(Source);
                                        bool bModular = ExportSettings.bModular && !ExportSettings.bForceShared;
                                        TArray<FName> LayerNames = ALandscapeProxy::GetLayersFromMaterial(LandscapeProxy->LandscapeMaterial);
                                        for (auto Name : LayerNames)
                                        {
                                            ULandscapeLayerInfoObject* LayerInfoObject = LandscapeInfo->GetLayerInfoByName(Name);

                                            if (LayerInfoObject)
                                            {
                                                FString FileName = (bModular ? MDLName : TEXT("Textures")) / LandscapeProxy->GetName() + TEXT("Weightmap") + LayerInfoObject->LayerName.ToString() + TEXT(".png");
                                                FString OmniPath = ExportMDLBaseAbsolutePath / FileName;
                                                FString AssetPath = ExportSettings.bMaterialOverSublayer ? FileName : (ExportMDLBaseRelativePath / FileName);
                                                FString LayerParamName = TEXT("Weightmap") + LayerInfoObject->LayerName.ToString();
                                                ExportLayerWeightmap(LandscapeInfo, LayerInfoObject, OmniPath);
#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
                                                auto WeightTextureInput = MdlSurfaceShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*LayerParamName)), pxr::SdfValueTypeNames->Asset);
#else // Create input to UsdMaterial and connect to shader
                                                auto WeightTextureInput = UsdMaterial.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*LayerParamName)), pxr::SdfValueTypeNames->Asset);
                                                MdlSurfaceShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*LayerParamName)), pxr::SdfValueTypeNames->Asset).ConnectToSource(WeightTextureInput);
#endif
                                                WeightTextureInput.Set(pxr::SdfAssetPath(TCHAR_TO_ANSI(*AssetPath)));
                                                WeightTextureInput.GetAttr().SetColorSpace(USDTokens.raw);
                                            }
                                        }

                                        // export visibility layer
                                        for (auto It = LandscapeInfo->Layers.CreateIterator(); It; It++)
                                        {
                                            FLandscapeInfoLayerSettings& LayerSettings = *It;
                                            if (LayerSettings.LayerInfoObj == ALandscapeProxy::VisibilityLayer)
                                            {
                                                FString FileName = (bModular ? MDLName : TEXT("Textures")) / LandscapeProxy->GetName() + TEXT("Weightmap") + UMaterialExpressionLandscapeVisibilityMask::ParameterName.ToString() + TEXT(".png");
                                                FString OmniPath = ExportMDLBaseAbsolutePath / FileName;
                                                FString AssetPath = ExportSettings.bMaterialOverSublayer ? FileName : (ExportMDLBaseRelativePath / FileName);
                                                FString LayerParamName = TEXT("Weightmap") + UMaterialExpressionLandscapeVisibilityMask::ParameterName.ToString();
                                                ExportLayerWeightmap(LandscapeInfo, LayerSettings.LayerInfoObj, OmniPath);
#if USD_DIRECT_SHADER_INPUT // Create input to UsdShader, KIT not supported for now
                                                auto WeightTextureInput = MdlSurfaceShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*LayerParamName)), pxr::SdfValueTypeNames->Asset);
#else // Create input to UsdMaterial and connect to shader
                                                auto WeightTextureInput = UsdMaterial.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*LayerParamName)), pxr::SdfValueTypeNames->Asset);
                                                MdlSurfaceShader.CreateInput(pxr::TfToken(TCHAR_TO_ANSI(*LayerParamName)), pxr::SdfValueTypeNames->Asset).ConnectToSource(WeightTextureInput);
#endif
                                                WeightTextureInput.Set(pxr::SdfAssetPath(TCHAR_TO_ANSI(*AssetPath)));
                                                WeightTextureInput.GetAttr().SetColorSpace(USDTokens.raw);
                                            }
                                        }
                                    }

                                    BindNumTexCoord(InStage, InPrimPath, MeshDescription, MaterialResult);
                                }
                            });
                        }
                    }
                }
            }
            else
            {
                for (int32 CompIndex = 0; CompIndex < RegisteredLandscapeComponents.Num(); ++CompIndex)
                {
                    WorldBounds += RegisteredLandscapeComponents[CompIndex]->CalcBounds(RegisteredLandscapeComponents[CompIndex]->GetComponentTransform()).GetBox();
                }
            }

            if (ExportSettings.bExportLandscapeGrass)
            {
                IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("grass.CullDistanceScale"));
                float GrassCullDistanceScale = CVar->GetFloat();
                CVar->Set(WorldBounds.GetExtent().GetMax());

                TArray<FVector> Cameras;
                Cameras.Add(WorldBounds.GetCenter());
                LandscapeProxy->UpdateGrass(Cameras, true);

                if (LandscapeProxy->FoliageComponents.Num() > 0)
                {
                    auto FoliagePrimName = LandscapeProxy->GetName() + TEXT("_Foliage");
                    auto FoliagePrimPath = ExportPrimPath.GetParentPath().AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*FoliagePrimName)));
                    if (bStandaloneUSD)
                    {
                        auto FoliageFileName = FPaths::GetBaseFilename(ExportMeshPath) + TEXT("_Foliage.") + FPaths::GetExtension(ExportMeshPath);
                        auto ExportFoliagePath = FPaths::GetPath(ExportMeshPath) / FoliageFileName;
                        if (FSimReadyUSDHelper::ExportInstancedMeshesToPointInstancerInternal(LandscapeProxy->FoliageComponents, ExportSettings, ExportedMDLs, ExportedMeshUSD, true, ExportFoliagePath))
                        {
                            // Reference to landscape prim
                            auto FoliageXform = pxr::UsdGeomXform::Define(USDStage, FoliagePrimPath);
                            AddReferenceOrPayload(ExportSettings.bPayloads, FoliageXform.GetPrim(), FoliageFileName);
                        }
                    }
                    else
                    {
                        FSimReadyUSDHelper::ExportInstancedMeshesToPointInstancer(LandscapeProxy->FoliageComponents, USDStage, FoliagePrimPath, ExportSettings, ExportedMDLs, ExportedMeshUSD, true);
                    }
                }

                // Restore
                CVar->Set(GrassCullDistanceScale);
            }
        }
    }

    return USDMesh;
}

bool FSimReadyUSDHelper::ExportComponentToUSD(USceneComponent* SceneComp, const pxr::UsdStageRefPtr& USDStage, const pxr::UsdStageRefPtr& MaterialOversStage, const pxr::SdfPath& RootPath,
    const FSimReadyExportSettings& ExportSettings, 
    TMap<FString, FString>& ExportedMeshUSD, TMap<FString, FString>& ExportedMDLs, TMap<FString, FString>& ExportedAnimeUSD)
{
    if (!USDStage || !SceneComp)
        return false;

    auto ParentPath = RootPath;

    FUSDExporter Exporter;
    Exporter.Stage = USDStage;
    Exporter.PathToObject.Add(FSimReadyPathHelper::PrimPathToKey(ParentPath.GetText()), (UObject*)SceneComp->GetAttachParent());
    Exporter.RootPrim = USDStage->GetPrimAtPath(ParentPath);
    TArray<UInstancedStaticMeshComponent*> ExportedInstancedStaticMeshComponents;

    auto CustomExport = [&](USceneComponent& SceneComp, bool& bContinue)
    {
        pxr::UsdGeomXformable NewPrim;

        if (SceneComp.IsA<ULandscapeComponent>() 
        || SceneComp.IsA<ULandscapeHeightfieldCollisionComponent>())
        {
            bContinue = false;
            return pxr::UsdGeomXformable();
        }

        if (SceneComp.IsA<UInstancedStaticMeshComponent>() && ExportedInstancedStaticMeshComponents.Find(Cast<UInstancedStaticMeshComponent>(&SceneComp)) != INDEX_NONE)
        {
            bContinue = false;
            return pxr::UsdGeomXformable();
        }

        if (FSimReadySlowTask::Get().IsProgressCancelled())
        {
            bContinue = false;
            return pxr::UsdGeomXformable();
        }

        auto SetupPrim = [](FUSDExporter& Exporter, const pxr::UsdGeomXformable& NewPrim, USceneComponent* SceneComp)
        {
            if(!NewPrim)
            {
                return;
            }

            Exporter.PathToObject.Add(FSimReadyPathHelper::PrimPathToKey(NewPrim.GetPath().GetText()), (UObject*)SceneComp);

            auto PrimSpec = NewPrim.GetPrim().GetStage()->GetEditTarget().GetLayer()->GetPrimAtPath(NewPrim.GetPath());
            auto References = PrimSpec->GetField(pxr::SdfFieldKeys->References);

            if (SceneComp->IsA<UPrimitiveComponent>())
            {
                ExportShadowFromPrimitiveComponent(NewPrim, Cast<UPrimitiveComponent>(SceneComp));
            }

            Exporter.ExportSceneComponent(NewPrim, *SceneComp);

            PrimSpec->SetField(pxr::SdfFieldKeys->References, References);
        };

        const bool bRootComponent = SceneComp.GetOwner() && SceneComp.GetOwner()->GetRootComponent() == &SceneComp;
        const bool bHasBlueprintComponents = SceneComp.GetOwner() && SceneComp.GetOwner()->BlueprintCreatedComponents.Num() > 0;
        auto Parent = SceneComp.GetAttachParent();
        auto PrimName = bRootComponent ? SceneComp.GetOwner()->GetName() : SceneComp.GetName();

        pxr::SdfPath PrimPath;		
        if(Parent)
        {
            auto ParentObjPathKey = Exporter.PathToObject.FindKey((UObject*)Parent);
            if(!ParentObjPathKey)
            {
                return pxr::UsdGeomXformable();
            }

            PrimPath = pxr::SdfPath(TCHAR_TO_ANSI(*FSimReadyPathHelper::KeyToPrimPath(*ParentObjPathKey)));
        }
        else
        {
            PrimPath = ParentPath;
        }

        // Check if the Prim path is already in the usd
        {
            pxr::SdfPath NewPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*PrimName)));
            
            while (pxr::UsdGeomXformable::Get(USDStage, NewPrimPath))
            {
                NewPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*(PrimName + TEXT("_") + FString::FromInt(SuffixIndex++)))));
            }

            PrimPath = NewPrimPath;
        }

#if UE_BUILD_DEBUG
        const FString PrimPathText = PrimPath.GetText();
#endif

        FString RootLayerFile = USDStage->GetRootLayer()->GetIdentifier().c_str();
        FString RootLayerPath = FPaths::GetPath(RootLayerFile);	

        UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(&SceneComp);
        USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(&SceneComp);
        
        UAnimationAsset* AnimationAsset = nullptr;
        const TArray<FSkeletalAnimationTimeSamples>* AnimationTimeSamples = nullptr;
        bool bUsingCopyPoseFromMesh = false;
        if (SkeletalMeshComponent)
        {
            bUsingCopyPoseFromMesh = IsUsingCopyPoseFromMesh(SkeletalMeshComponent);
            if (!bUsingCopyPoseFromMesh)
            {
                AnimationTimeSamples = FSequenceExtractor::Get().GetSkeletalAnimationTimeSamples(SkeletalMeshComponent);
                if (AnimationTimeSamples == nullptr)
                {
                    AnimationTimeSamples = FSequenceExtractor::Get().GetSkeletalAnimationTimeSamples(SkeletalMeshComponent->GetOwner());
                }
                // only export animation when there's no animation sequence
                if (AnimationTimeSamples == nullptr)
                {
                    UAnimSingleNodeInstance* SingleNodeInstance = SkeletalMeshComponent->GetSingleNodeInstance();
                    if (SingleNodeInstance)
                    {
                        AnimationAsset = SingleNodeInstance->GetAnimationAsset();
                    }
                }
            }
        }

        auto IsComponentHidden = [](const USceneComponent& Comp, bool bExportInvisibleComponent)
        {
            // Sky light is hidden by default
            if (Comp.IsA<USkyLightComponent>())
            {
                return false;
            }

            return (!bExportInvisibleComponent && (!Comp.IsVisible() && (FSequenceExtractor::Get().GetVisibilityTimeSamples(&Comp) == nullptr)))
            || (Comp.GetOwner() && Comp.GetOwner()->IsHidden() && (FSequenceExtractor::Get().GetVisibilityTimeSamples(Comp.GetOwner()) == nullptr));
        };

        bool bAnimationExport = bUsingCopyPoseFromMesh || AnimationTimeSamples || AnimationAsset;
        // Used for umap(separate usd) exporting
        FString MeshName;
        FString MeshFullName;
        FString MDLAbsoluteBasePath;
        FString MDLRelativeBasePathToRoot; // Relative path for saving mdl to the main USD, can't be used for the separated USD
        FString MeshAbsolutePath;
        FString MeshRelativePathToRoot;
        FString MeshAbsoluteBasePath;
        FString MeshRelativeBasePathToRoot;

        FString AnimeName;
        FString AnimeFullName;
        FString AnimeAbsolutePath;
        FString AnimeRelativePathToRoot;
        FString AnimeAbsoluteBasePath;
        FString AnimeRelativeBasePathToRoot;
        FString Extension;


        //export mesh to separate USD		
        ALandscapeProxy* LandscapeProxy = Cast<ALandscapeProxy>(SceneComp.GetOwner());
        TArray<UInstancedStaticMeshComponent*> InstancedStaticMeshComponents;
        
        const TArray<USceneComponent*>& AttachedChildren = SceneComp.GetAttachChildren();
        for(auto Comp : AttachedChildren)
        {
            // Verify that Comp is valid... somtimes it is not (TODO: dig deeper?)
            if (Comp && Comp->IsA<UInstancedStaticMeshComponent>())
            {
                if (!IsComponentHidden(*Comp, ExportSettings.bExportInvisibleComponent))
                {
                    InstancedStaticMeshComponents.Add(Cast<UInstancedStaticMeshComponent>(Comp));
                }
                ExportedInstancedStaticMeshComponents.Add(Cast<UInstancedStaticMeshComponent>(Comp));
            }
        }

        if (SceneComp.IsA<UInstancedStaticMeshComponent>())
        {
            if (!IsComponentHidden(SceneComp, ExportSettings.bExportInvisibleComponent))
            {
                InstancedStaticMeshComponents.Add(Cast<UInstancedStaticMeshComponent>(&SceneComp));
            }
            ExportedInstancedStaticMeshComponents.Add(Cast<UInstancedStaticMeshComponent>(&SceneComp));
        }

        if(ExportSettings.bExportAssetToSeperateUSD)
        {
            if ((LandscapeProxy && bRootComponent)
                || (StaticMeshComponent && StaticMeshComponent->GetStaticMesh())
                || (SkeletalMeshComponent && SkeletalMeshComponent->SkeletalMesh))
            {

                MeshName = LandscapeProxy && bRootComponent ? LandscapeProxy->GetName() : StaticMeshComponent ? StaticMeshComponent->GetStaticMesh()->GetName() : SkeletalMeshComponent->SkeletalMesh->GetName();
                MeshFullName = LandscapeProxy && bRootComponent ? LandscapeProxy->GetFullName() : StaticMeshComponent ? StaticMeshComponent->GetStaticMesh()->GetFullName() : SkeletalMeshComponent->SkeletalMesh->GetFullName();				
                if (SceneComp.IsA<USplineMeshComponent>())
                {
                    // can't use static mesh name
                    MeshName = SceneComp.GetOwner() ? (SceneComp.GetOwner()->GetName() + TEXT("_") + SceneComp.GetName()) : SceneComp.GetName();
                    MeshFullName = SceneComp.GetFullName();
                }
                else if (SceneComp.IsA<UInstancedStaticMeshComponent>())
                {
                    // Instanced Static Mesh can't use mesh name as exporting name
                    MeshName = SceneComp.GetOwner() ? (SceneComp.GetOwner()->GetName() + TEXT("_PointInstancer")) : SceneComp.GetName();
                    MeshFullName = SceneComp.GetFullName();
                }

                if (AnimationTimeSamples || bUsingCopyPoseFromMesh)
                {
                    AnimeName = SkeletalMeshComponent->GetName() + TEXT("_") + SkeletalMeshComponent->SkeletalMesh->GetName();
                    AnimeFullName = SkeletalMeshComponent->GetFullName() + TEXT("_") + SkeletalMeshComponent->SkeletalMesh->GetFullName();
                }
                else if (AnimationAsset)
                {
                    AnimeName = AnimationAsset->GetName();
                    AnimeFullName = AnimationAsset->GetFullName();
                }
            }
            else
            {
                auto USDModule = FModuleManager::GetModulePtr<ISimReadyUSDModule>("SimReadyUSD");
                if (USDModule && USDModule->HasUSDTranslator())
                {
                    for (auto USDTranslator : USDModule->GetUSDTranslators())
                    {
                        if (USDTranslator->IsComponentSupported(&SceneComp))
                        {
                            USDTranslator->GetExportNameAndFullName(&SceneComp, MeshName, MeshFullName);
                            break;
                        }
                    }
                }
                
                if (MeshName.IsEmpty() || MeshFullName.IsEmpty())
                {
                    MeshName = SceneComp.GetOwner() ? (SceneComp.GetOwner()->GetName() + TEXT("_") + SceneComp.GetName()) : SceneComp.GetName();
                    MeshFullName = SceneComp.GetFullName();
                }
            }

            MeshName = pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*MeshName)).c_str();
            AnimeName = pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*AnimeName)).c_str();

            {
                // If all props should be exported to their own directory
                if(ExportSettings.bModular)
                {
                    FString CheckedMeshName = SceneComp.IsA<USplineMeshComponent>() ? SceneComp.GetOwner()->GetName() : FSimReadyAssetExportHelper::GetLegalMDLName(MeshName);
                    MeshAbsoluteBasePath = RootLayerPath / "Props" / CheckedMeshName;
                    MeshRelativeBasePathToRoot = "./Props" / CheckedMeshName;
                    if (bAnimationExport)
                    {
                        FString CheckedAnimeName = FSimReadyAssetExportHelper::GetLegalMDLName(AnimeName);
                        AnimeAbsoluteBasePath = RootLayerPath / "Animations" / CheckedAnimeName;
                        AnimeRelativeBasePathToRoot = "./Animations" / CheckedAnimeName;
                    }
                    MDLAbsoluteBasePath = MeshAbsoluteBasePath;
                    MDLRelativeBasePathToRoot = MeshRelativeBasePathToRoot;
                }
                else
                {
                    MeshAbsoluteBasePath = RootLayerPath / "Props";
                    MeshRelativeBasePathToRoot = "./Props";
                    if (bAnimationExport)
                    {
                        AnimeAbsoluteBasePath = RootLayerPath / "Animations";
                        AnimeRelativeBasePathToRoot = "./Animations";
                    }
                    MDLAbsoluteBasePath = RootLayerPath / "Materials";
                    MDLRelativeBasePathToRoot = "./Materials";
                }

                if(ExportSettings.bAddExtraExtension)
                {
                    Extension = ExportSettings.bAsciiFormat ? ".prop.usda" : ".prop.usd";
                }
                else
                {
                    Extension = GetUSDExtension(ExportSettings.bAsciiFormat);
                }

                MeshAbsolutePath = MeshAbsoluteBasePath / MeshName + Extension;
                MeshRelativePathToRoot = MeshRelativeBasePathToRoot / MeshName + Extension;
                // There is already an static mesh that has the same name
                if(!ExportedMeshUSD.Contains(MeshFullName))
                {
                    FString RenameMeshAbsolutePath = MeshAbsolutePath;
                    while(ExportedMeshUSD.FindKey(RenameMeshAbsolutePath))
                    {
                        RenameMeshAbsolutePath = MeshAbsoluteBasePath / MeshName + TEXT("_") + FString::FromInt(SuffixIndex++) + Extension;
                    }

                    MeshAbsolutePath = RenameMeshAbsolutePath;
                    MeshRelativePathToRoot = MeshRelativeBasePathToRoot / FPaths::GetCleanFilename(MeshAbsolutePath);
                }

                if (bAnimationExport)
                {
                    AnimeAbsolutePath = AnimeAbsoluteBasePath / AnimeName + Extension;
                    AnimeRelativePathToRoot = AnimeRelativeBasePathToRoot / AnimeName + Extension;
                    // There is already an animation that has the same name
                    if (!ExportedAnimeUSD.Contains(AnimeFullName))
                    {
                        FString RenameAnimeAbsolutePath = AnimeAbsolutePath;
                        while (ExportedAnimeUSD.FindKey(RenameAnimeAbsolutePath))
                        {
                            RenameAnimeAbsolutePath = AnimeAbsoluteBasePath / AnimeName + TEXT("_") + FString::FromInt(SuffixIndex++) + Extension;
                        }

                        AnimeAbsolutePath = RenameAnimeAbsolutePath;
                        AnimeRelativePathToRoot = AnimeRelativeBasePathToRoot / FPaths::GetCleanFilename(AnimeAbsolutePath);
                    }
                }
            }
        }

        auto ShouldInstanceable = [&](bool bInstanceable)
        {
            return ExportSettings.bMeshInstanced && bInstanceable;
        };

        auto BindAndRegisterPrim = [&](bool bInstanceable)
        {
            if(!ExportedMeshUSD.Contains(MeshFullName))
            {
                ExportedMeshUSD.Add(MeshFullName, MeshAbsolutePath);
            }
            else
            {
                MeshAbsolutePath = *ExportedMeshUSD.Find(MeshFullName);
                MeshRelativePathToRoot = FPaths::GetPath(MeshRelativePathToRoot) / FPaths::GetCleanFilename(MeshAbsolutePath);
            }

            NewPrim = pxr::UsdGeomXform::Define(USDStage, PrimPath);
            auto ReferencedPrim = NewPrim;
            // NOTE: Prim with the children can't be set instanceable, or the children will be lost. Having to add new prim to hold the instanced reference.
            if (bInstanceable && SceneComp.GetAttachChildren().Num() > 0)
            {
                pxr::SdfPath InstancedReferencedPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*(TEXT("Instanced") + PrimName))));
                ReferencedPrim = pxr::UsdGeomXform::Define(USDStage, InstancedReferencedPrimPath);
            }

            auto RootPrim = ReferencedPrim.GetPrim();
            AddReferenceOrPayload(ExportSettings.bPayloads, RootPrim, MeshRelativePathToRoot);
            if (bInstanceable)
            {
                RootPrim.SetInstanceable(true);
            }
        };

        auto BindAnimationPrim = [&](const pxr::UsdSkelRoot& USDSkelRoot, const pxr::SdfPath& SkelAnimationPrimPath, float Offset = 0.0f, float Scale = 1.0f)
        {
            if (!ExportedAnimeUSD.Contains(AnimeFullName))
            {
                ExportedAnimeUSD.Add(AnimeFullName, AnimeAbsolutePath);
            }
            else
            {
                AnimeAbsolutePath = *ExportedAnimeUSD.Find(AnimeFullName);
                AnimeRelativePathToRoot = FPaths::GetPath(AnimeRelativePathToRoot) / FPaths::GetCleanFilename(AnimeAbsolutePath);
            }

            if (NewPrim)
            {
                if (ExportSettings.bPayloads)
                {
                    NewPrim.GetPrim().GetPayloads().AddPayload(TCHAR_TO_ANSI(*AnimeRelativePathToRoot), pxr::SdfLayerOffset(Offset, Scale));
                }
                else
                {
                    NewPrim.GetPrim().GetReferences().AddReference(TCHAR_TO_ANSI(*AnimeRelativePathToRoot), pxr::SdfLayerOffset(Offset, Scale));
                }
            }

            // bind animation
            pxr::UsdSkelBindingAPI AnimationSourceBinding = pxr::UsdSkelBindingAPI::Apply(USDSkelRoot.GetPrim());
            AnimationSourceBinding.CreateAnimationSourceRel().SetTargets(pxr::SdfPathVector({ SkelAnimationPrimPath }));
        };

        auto DefineEmptyXform = [&]()
        {
            NewPrim = pxr::UsdGeomXform::Define(USDStage, PrimPath);
            pxr::UsdModelAPI(NewPrim).SetKind(pxr::KindTokens->group);
            FixPrimKind(NewPrim.GetPrim());
            SetupPrim(Exporter, NewPrim, &SceneComp);
            return NewPrim;
        };

        bContinue = false;
        if (IsComponentHidden(SceneComp, ExportSettings.bExportInvisibleComponent))
        {
            if (!LookupSupportedComponent(&SceneComp, ExportSettings))
            {
                return pxr::UsdGeomXformable();
            }
            else
            {
                // Hidden component is replaced with GeomXform if the children should be exported
                return DefineEmptyXform();
            }
        }
        else if (LandscapeProxy && bRootComponent)
        {
            if(ExportSettings.bExportAssetToSeperateUSD)
            {
                auto NewExportSettings = ExportSettings;
                NewExportSettings.bForceShared = true;
                if (NewExportSettings.bModular)
                {
                    ExportedMDLs.Reset();
                    FSimReadyAssetExportHelper::ResetGlobalCaches();
                }
                FString MDLRelativeBasePath = FSimReadyPathHelper::ComputeRelativePath(MDLAbsoluteBasePath, MeshAbsolutePath, true);
                if (FSimReadyUSDHelper::ExportLandscapeToUSDInternal(LandscapeProxy, MDLAbsoluteBasePath, MDLRelativeBasePath, NewExportSettings, ExportedMDLs, ExportedMeshUSD, MeshAbsolutePath))
                {
                    BindAndRegisterPrim(false);
                }

                auto MeshPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_UTF8(*LandscapeProxy->GetName())));
                pxr::UsdPrim USDMeshPrim = USDStage->GetPrimAtPath(MeshPrimPath);
                if (USDMeshPrim)
                {
                    auto GeomSubsets = pxr::UsdGeomSubset::GetAllGeomSubsets(pxr::UsdGeomImageable(USDMeshPrim));
                    TInlineComponentArray<ULandscapeComponent*> RegisteredLandscapeComponents;
                    LandscapeProxy->GetComponents<ULandscapeComponent>(RegisteredLandscapeComponents);
                    for (int32 CompIndex = 0; CompIndex < RegisteredLandscapeComponents.Num(); ++CompIndex)
                    {
                        if (RegisteredLandscapeComponents[CompIndex]->MaterialInstances.Num() > 0)
                        {
                            auto LandscapeMaterialInst = RegisteredLandscapeComponents[CompIndex]->MaterialInstances[0];
                            BindMaterialOverrides(MaterialOversStage, GeomSubsets[CompIndex].GetPrim(), LandscapeMaterialInst, MDLRelativeBasePathToRoot, ExportedMDLs, NewExportSettings);
                        }
                    }
                }
            }
            else
            {
                NewPrim = FSimReadyUSDHelper::ExportLandscapeToUSD(LandscapeProxy, USDStage, PrimPath, RootLayerPath, ".", ExportSettings, ExportedMDLs, ExportedMeshUSD);
            }

            SetupPrim(Exporter, NewPrim, &SceneComp);

            return NewPrim;
        }		
        else if (InstancedStaticMeshComponents.Num() > 0)
        {
            if(ExportSettings.bExportAssetToSeperateUSD)
            {
                TMap<FString, FString> TempExportedMDLs;
                FString MDLRelativeBasePath = FSimReadyPathHelper::ComputeRelativePath(MDLAbsoluteBasePath, MeshAbsolutePath, true);
                if (FSimReadyUSDHelper::ExportInstancedMeshesToPointInstancerInternal(InstancedStaticMeshComponents, ExportSettings, ExportSettings.bModular ? TempExportedMDLs : ExportedMDLs, ExportedMeshUSD, false, MeshAbsolutePath))
                {
                    BindAndRegisterPrim(false);
                }

                // Bind to material override
                auto PrototypesRootPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*FPaths::GetBaseFilename(MeshAbsolutePath)))).AppendElementString(pxr::TfMakeValidIdentifier("Prototypes"));
                for (int32 Index = 0; Index < InstancedStaticMeshComponents.Num(); ++Index)
                {
                    auto InstancedMeshComp = InstancedStaticMeshComponents[Index];
                    if (InstancedMeshComp->GetInstanceCount() > 0 && InstancedMeshComp->GetStaticMesh())
                    {
                        auto InstancedStaticMesh = InstancedMeshComp->GetStaticMesh();
                        auto PrototypesPrimPath = PrototypesRootPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*InstancedMeshComp->GetName())));
                        auto MeshPrimPath = PrototypesPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*InstancedStaticMesh->GetName())));
                        pxr::UsdPrim USDMeshPrim = USDStage->GetPrimAtPath(MeshPrimPath);
                        if (USDMeshPrim)
                        {
                            const int32 SMCurrentMinLOD = InstancedStaticMesh->MinLOD.GetValue();
                            int32 LODToExport = FMath::Clamp(SMCurrentMinLOD, 0, InstancedStaticMesh->RenderData->LODResources.Num() - 1);
                            int32 NumSections = InstancedStaticMesh->GetSectionInfoMap().GetSectionNumber(LODToExport);

                            for(int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
                            {
                                FMeshSectionInfo Info = InstancedStaticMesh->GetSectionInfoMap().Get(LODToExport, SectionIndex);
                                auto MaterialInterface = InstancedStaticMesh->GetMaterial(Info.MaterialIndex);
                                auto ComponentMaterialInterface = InstancedMeshComp->GetMaterial(Info.MaterialIndex);
                                auto ParameterTimeSamples = GetMaterialParameterTimeSample(InstancedMeshComp, Info.MaterialIndex);

                                auto Prim = NumSections == 1 ? USDMeshPrim : GetSectionPrimByIndex(USDStage, USDMeshPrim.GetPath(), SectionIndex);
                                if (Prim)
                                {
                                    bool bAOMaskExported = TryExportAndBindMaterialWithAOMask(InstancedMeshComp, ParameterTimeSamples, LODToExport, MaterialOversStage ? MaterialOversStage : USDStage, ComponentMaterialInterface, Prim, ExportSettings, MDLAbsoluteBasePath, MDLRelativeBasePathToRoot, ExportSettings.bModular ? TempExportedMDLs : ExportedMDLs);
                                    //Binding materials for component, skipping the ones that
                                    //component does not override the materials from static mesh.
                                    if (((!bAOMaskExported) && 
                                        ((!MaterialInterface) || (MaterialInterface->GetPathName() != ComponentMaterialInterface->GetPathName()))) ||
                                        (ParameterTimeSamples != nullptr))
                                    {
                                        ExportAndBindMaterial(MaterialOversStage ? MaterialOversStage : USDStage, ComponentMaterialInterface, ParameterTimeSamples, Prim, ExportSettings, MDLAbsoluteBasePath, MDLRelativeBasePathToRoot, ExportSettings.bModular ? TempExportedMDLs : ExportedMDLs, TEXT(""), TEXT(""),
                                            [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FSimReadyExportMaterialResult& MaterialResult, FString& UsdMaterialName)
                                            {
                                                CheckUsdNameByNumTexCoord(InStage, InPrimPath, InstancedStaticMesh->RenderData->LODResources[LODToExport], MaterialResult, UsdMaterialName);
                                            },
                                            [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FString& MDLName, const FSimReadyExportMaterialResult& MaterialResult, const FString& UsdMaterialName)
                                            {
                                                BindNumTexCoord(InStage, InPrimPath, InstancedStaticMesh, LODToExport, MaterialResult);
                                            }
                                        );
                                    }
                                    else if (!bAOMaskExported)
                                    {
                                        BindMaterialOverrides(MaterialOversStage, Prim, MaterialInterface, MDLRelativeBasePathToRoot, ExportedMDLs, ExportSettings);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                pxr::UsdGeomPointInstancer USDPointInstancer = FSimReadyUSDHelper::ExportInstancedMeshesToPointInstancer(InstancedStaticMeshComponents, USDStage, PrimPath, ExportSettings, ExportedMDLs, ExportedMeshUSD);
                if(USDPointInstancer)
                {
                    NewPrim = pxr::UsdGeomXformable(USDPointInstancer);
                }
            }

            SetupPrim(Exporter, NewPrim, &SceneComp);

            return NewPrim;
        }
        else if (SceneComp.IsA<UTextRenderComponent>() || (ExportSettings.bExportDecalActors && SceneComp.IsA<UDecalComponent>()))
        {
            if (ExportSettings.bExportAssetToSeperateUSD)
            {
                TMap<FString, FString> TempExportedMDLs;
                FString MDLRelativeBasePath = FSimReadyPathHelper::ComputeRelativePath(MDLAbsoluteBasePath, MeshAbsolutePath, true);
                if (FSimReadyUSDHelper::ExportConvertedMeshToUSDInternal(&SceneComp, MDLAbsoluteBasePath, MDLRelativeBasePath, ExportSettings, ExportSettings.bModular ? TempExportedMDLs : ExportedMDLs, MeshAbsolutePath))
                {
                    BindAndRegisterPrim(false);
                }

                // Bind to material override
                auto MeshPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_UTF8(*SceneComp.GetName())));
                pxr::UsdPrim USDMeshPrim = USDStage->GetPrimAtPath(MeshPrimPath);
                if (USDMeshPrim)
                {
                    auto TextRenderComponent = Cast<UTextRenderComponent>(&SceneComp);
                    auto DecalComponent = Cast<UDecalComponent>(&SceneComp);
                    BindMaterialOverrides(MaterialOversStage, USDMeshPrim, TextRenderComponent ? TextRenderComponent->TextMaterial : DecalComponent->GetDecalMaterial(), MDLRelativeBasePathToRoot, ExportedMDLs, ExportSettings);
                }
            }
            else
            {
                NewPrim = FSimReadyUSDHelper::ExportConvertedMeshToUSD(&SceneComp, USDStage, PrimPath, RootLayerPath, ".", ExportSettings, ExportedMDLs);
            }

            SetupPrim(Exporter, NewPrim, &SceneComp);

            return NewPrim;
        }
        else if (SkeletalMeshComponent)
        {
            if (SkeletalMeshComponent->SkeletalMesh == nullptr)
            {
                return DefineEmptyXform();
            }

            if(ExportSettings.bExportAssetToSeperateUSD)
            {
                // For modular mode, materials of mesh will be exported separately.
                // So materials will not be reuse across meshes.
                TMap<FString, FString> TempExportedMDLs;
                int32 LODToExport = 0;
                if(ExportedMeshUSD.Contains(MeshFullName) || ExportSkeletalMeshAsUSDToPathInternal(SkeletalMeshComponent->SkeletalMesh, SkeletalMeshComponent, MeshAbsolutePath,
                    MDLAbsoluteBasePath, false, ExportSettings, ExportSettings.bModular ? TempExportedMDLs : ExportedMDLs))
                {
                    BindAndRegisterPrim(false);

                    // Export animation must be after skel mesh exporting
                    FString SkelRootPrimName = FPaths::GetBaseFilename(MeshAbsolutePath);
                    while (SkelRootPrimName != FPaths::GetBaseFilename(SkelRootPrimName)) // Remove all extension
                    {
                        SkelRootPrimName = FPaths::GetBaseFilename(SkelRootPrimName);
                    }

                    auto SkelRootPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SkelRootPrimName)));
                    pxr::UsdSkelRoot USDSkelRoot = pxr::UsdSkelRoot(USDStage->GetPrimAtPath(SkelRootPrimPath));				
                    if (bUsingCopyPoseFromMesh)
                    {
                        TFunction<bool(USkeletalMeshComponent*, FString&, FString&)> GetSourceAnimeFullName;
                        GetSourceAnimeFullName = [&](USkeletalMeshComponent* SkeletalMeshComp, FString& OutFullAnimeName, FString& OutPrimPath)
                        {
                            if (SkeletalMeshComp)
                            {
                                bool bUsingCopyPoseFromMesh = IsUsingCopyPoseFromMesh(SkeletalMeshComp);
                                if (bUsingCopyPoseFromMesh)
                                {
                                    return GetSourceAnimeFullName(GetCopyPoseSourceComponent(SkeletalMeshComp), OutFullAnimeName, OutPrimPath);
                                }
                                else
                                {
                                    if (SkeletalMeshComp->SkeletalMesh == nullptr)
                                    {
                                        return false;
                                    }

                                    auto FoundPathKey = Exporter.PathToObject.FindKey((UObject*)SkeletalMeshComp);
                                    if (FoundPathKey)
                                    {
                                        OutPrimPath = FSimReadyPathHelper::KeyToPrimPath(*FoundPathKey);
                                    }

                                    const TArray<FSkeletalAnimationTimeSamples>* AnimationTimeSamples = FSequenceExtractor::Get().GetSkeletalAnimationTimeSamples(SkeletalMeshComp);
                                    if (AnimationTimeSamples == nullptr)
                                    {
                                        AnimationTimeSamples = FSequenceExtractor::Get().GetSkeletalAnimationTimeSamples(SkeletalMeshComp->GetOwner());
                                    }
                                    // only export animation when there's no animation sequence
                                    if (AnimationTimeSamples)
                                    {
                                        OutFullAnimeName = SkeletalMeshComp->GetFullName() + TEXT("_") + SkeletalMeshComp->SkeletalMesh->GetFullName();
                                        return true;
                                    }
                                    else
                                    {
                                        UAnimSingleNodeInstance* SingleNodeInstance = SkeletalMeshComp->GetSingleNodeInstance();
                                        if (SingleNodeInstance)
                                        {
                                            if (SingleNodeInstance->GetAnimationAsset())
                                            {
                                                OutFullAnimeName = SingleNodeInstance->GetAnimationAsset()->GetFullName();
                                                return true;
                                            }
                                        }
                                    }
                                }
                            }

                            return false;
                        };
                    
                        FString SourcePrimPath;
                        if (GetSourceAnimeFullName(GetCopyPoseSourceComponent(SkeletalMeshComponent), AnimeFullName, SourcePrimPath))
                        {
                            if (ExportedAnimeUSD.Contains(AnimeFullName))
                            {
                                FString SourcePrimName = FPaths::GetBaseFilename(*ExportedAnimeUSD.Find(AnimeFullName));
                                auto SkelAnimationPrimPath = pxr::SdfPath(TCHAR_TO_ANSI(*SourcePrimPath)).AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SourcePrimName)));
                                BindAnimationPrim(USDSkelRoot, SkelAnimationPrimPath);
                            }
                        }
                    }
                    else if (AnimationTimeSamples)
                    {
                        auto RootLayerHandle = USDStage->GetRootLayer();
                        double StartTimeCode = RootLayerHandle->HasStartTimeCode() ? RootLayerHandle->GetStartTimeCode() : 0.0;
                        double EndTimeCode = RootLayerHandle->HasEndTimeCode() ? RootLayerHandle->GetEndTimeCode() : 0.0;
                        double TimeCodesPerSecond = RootLayerHandle->HasTimeCodesPerSecond() ? RootLayerHandle->GetTimeCodesPerSecond() : DEFAULT_TIMECODESPERSECOND;
                        if (ExportedAnimeUSD.Contains(AnimeFullName) || ExportAnimationTimeSamplesAsUSDToPathInternal(AnimationTimeSamples, StartTimeCode, EndTimeCode, TimeCodesPerSecond, SkeletalMeshComponent, AnimeAbsolutePath, ExportSettings))
                        {
                            // bind to main usd
                            FString SkelAnimationPrimName = FPaths::GetBaseFilename(AnimeAbsolutePath);
                            while (SkelAnimationPrimName != FPaths::GetBaseFilename(SkelAnimationPrimName)) // Remove all extension
                            {
                                SkelAnimationPrimName = FPaths::GetBaseFilename(SkelAnimationPrimName);
                            }

                            auto SkelAnimationPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SkelAnimationPrimName)));
                            BindAnimationPrim(USDSkelRoot, SkelAnimationPrimPath);
                        }
                    }
                    else
                    {
                        UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(AnimationAsset);
                        if (AnimSequence)
                        {
                            if (ExportedAnimeUSD.Contains(AnimeFullName) || ExportAnimationAssetAsUSDToPathInternal(AnimSequence, SkeletalMeshComponent, AnimeAbsolutePath, ExportSettings))
                            {
                                // bind to main usd
                                FString SkelAnimationPrimName = FPaths::GetBaseFilename(AnimeAbsolutePath);
                                while (SkelAnimationPrimName != FPaths::GetBaseFilename(SkelAnimationPrimName)) // Remove all extension
                                {
                                    SkelAnimationPrimName = FPaths::GetBaseFilename(SkelAnimationPrimName);
                                }

                                auto SkelAnimationPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*SkelAnimationPrimName)));
                                const double TimeCodesPerSecond = FMath::TruncToFloat(((AnimSequence->GetNumberOfFrames() - 1) / AnimSequence->SequenceLength) + 0.5f);
                                float Scale = 1.0f / SkeletalMeshComponent->GetPlayRate();
                                float Offset = -SkeletalMeshComponent->GetPosition() * TimeCodesPerSecond * Scale;
                                BindAnimationPrim(USDSkelRoot, SkelAnimationPrimPath, Offset, Scale);

                                // update usd time codes						
                                auto RootLayerHandle = USDStage->GetRootLayer();
                                const double StartTimeCode = 0.0;
                                const double EndTimeCode = (AnimSequence->GetNumberOfFrames() - 1) * Scale + Offset;

                                if (!RootLayerHandle->HasStartTimeCode() || StartTimeCode < RootLayerHandle->GetStartTimeCode())
                                {
                                    RootLayerHandle->SetStartTimeCode(StartTimeCode);
                                }

                                if (!RootLayerHandle->HasEndTimeCode() || EndTimeCode > RootLayerHandle->GetEndTimeCode())
                                {
                                    RootLayerHandle->SetEndTimeCode(EndTimeCode);
                                }
                                RootLayerHandle->SetTimeCodesPerSecond(TimeCodesPerSecond);
                            }
                        }
                    }

                    TArray<pxr::UsdPrim> MeshPrims;
                    bool bBelongToCarlaVehicle = FUSDCARLAVehicleTools::BelongToCarlaVehicle(SkeletalMeshComponent);
                    if (bBelongToCarlaVehicle)
                    {
                        auto RootPrim = USDStage->GetPrimAtPath(SkelRootPrimPath);
                        FindAllPrims<pxr::UsdGeomMesh>(RootPrim, MeshPrims);
                    }
                    else
                    {
                        GetMeshFromSkelRoot(USDSkelRoot, MeshPrims);
                    }

                    if (MeshPrims.Num() > 0)
                    {
                        //export materials
                        const FSkeletalMeshRenderData * SkelMeshRenderData = SkeletalMeshComponent->SkeletalMesh->GetResourceForRendering();
                        if (SkelMeshRenderData && SkelMeshRenderData->LODRenderData.Num() > 0)
                        {
                            const FSkeletalMeshLODRenderData& LODData = SkelMeshRenderData->LODRenderData[LODToExport];
                            const FSkeletalMeshLODInfo& Info = *(SkeletalMeshComponent->SkeletalMesh->GetLODInfo(LODToExport));

                            if (MeshPrims.Num() == LODData.RenderSections.Num() || bBelongToCarlaVehicle)
                            {							
                                for (int32 SectionIndex = 0; SectionIndex < LODData.RenderSections.Num(); SectionIndex++)
                                {
                                    int32 MaterialIndex = LODData.RenderSections[SectionIndex].MaterialIndex;
                                    if (SectionIndex < Info.LODMaterialMap.Num() && SkeletalMeshComponent->SkeletalMesh->Materials.IsValidIndex(Info.LODMaterialMap[SectionIndex]))
                                    {
                                        MaterialIndex = Info.LODMaterialMap[SectionIndex];
                                        MaterialIndex = FMath::Clamp(MaterialIndex, 0, SkeletalMeshComponent->SkeletalMesh->Materials.Num());
                                    }
                                    auto AssetMaterialInterface = SkeletalMeshComponent->SkeletalMesh->Materials[MaterialIndex].MaterialInterface;
                                    auto ComponentMaterialInterface = SkeletalMeshComponent->GetMaterial(MaterialIndex);
                                    auto ParameterTimeSamples = GetMaterialParameterTimeSample(SkeletalMeshComponent, MaterialIndex);

                                    FString SectionName = FString::Printf(TEXT("Section%d"), SectionIndex);

                                    int32 MeshIndex = 0;
                                    while (MeshIndex < MeshPrims.Num())
                                    {
                                        auto MeshPrim = MeshPrims[MeshIndex];
                                        FString MeshPrimName = MeshPrim.GetName().GetText();
                                        bool bSection = (MeshPrimName == SectionName);
                                        bool bVehicleSubMesh = FUSDCARLAVehicleTools::IsCarlaVehicleSubMesh(MeshPrimName, SectionIndex);
                                        if (!bSection && !bVehicleSubMesh)
                                        {
                                            ++MeshIndex;
                                            continue;
                                        }
                                        
                                        //Binding materials for component, skipping the ones that
                                        //component does not override the materials from static mesh.
                                        if ((ComponentMaterialInterface &&
                                            ((!AssetMaterialInterface) || (AssetMaterialInterface->GetPathName() != ComponentMaterialInterface->GetPathName()))) ||
                                            (ParameterTimeSamples != nullptr))
                                        {
                                            ExportAndBindMaterial(MaterialOversStage ? MaterialOversStage : USDStage, ComponentMaterialInterface, ParameterTimeSamples, MeshPrim, ExportSettings, MDLAbsoluteBasePath, MDLRelativeBasePathToRoot, ExportSettings.bModular ? TempExportedMDLs : ExportedMDLs, TEXT(""), ParameterTimeSamples ? SkeletalMeshComponent->GetOwner()->GetName() + SkeletalMeshComponent->GetName() + ComponentMaterialInterface->GetName() : TEXT(""));
                                        }
                                        else
                                        {
                                            BindMaterialOverrides(MaterialOversStage, MeshPrim, ComponentMaterialInterface, MDLRelativeBasePathToRoot, ExportedMDLs, ExportSettings);
                                        }
                                    
                                        MeshPrims.RemoveAt(MeshIndex);

                                        if (bSection)
                                        {
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                pxr::UsdSkelRoot USDSkelRoot = ExportSkeletalMeshToUSD(SkeletalMeshComponent->SkeletalMesh, SkeletalMeshComponent, USDStage, PrimPath, RootLayerPath, ".", true, ExportSettings, ExportedMDLs);
                if(USDSkelRoot)
                {
                    NewPrim = pxr::UsdGeomXformable(USDSkelRoot);
                }
            }

            SetupPrim(Exporter, NewPrim, &SceneComp);

            if (FUSDCARLAVehicleTools::BelongToCarlaVehicle(&SceneComp))
            {
                FUSDCARLAVehicleTools::ExportSimReadyVehicle(NewPrim.GetPrim(), *SkeletalMeshComponent);
            }

            return NewPrim;
        }
        else if(StaticMeshComponent)
        {
            if (StaticMeshComponent->GetStaticMesh() == nullptr)
            {
                return DefineEmptyXform();
            }

            auto BlueprintClass = Cast<UBlueprintGeneratedClass>(StaticMeshComponent->GetOwner()->GetClass());
            if (BlueprintClass && BlueprintClass->GetName() == TEXT("BP_Sky_C"))
            {
                return DefineEmptyXform();
            }

            auto IsInstanced = [ExportSettings](UStaticMeshComponent* StaticMeshComponent, int32 LODToExport)
            {
                auto StaticMesh = StaticMeshComponent->GetStaticMesh();

                // Mesh with the override vertex color can't be instanced.
                if (LODToExport < StaticMeshComponent->LODData.Num())
                {
                    if (StaticMeshComponent->LODData[LODToExport].OverrideVertexColors)
                    {
                        return false;
                    }
                }

                // No material exported
                if (!ExportSettings.bMDL && !ExportSettings.bPreviewSurface)
                {
                    return true;
                }

                if (StaticMeshComponent->GetNumOverrideMaterials() == 0)
                {
                    return true;
                }

                int32 NumSections = StaticMesh->GetSectionInfoMap().GetSectionNumber(LODToExport);

                for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
                {
                    FMeshSectionInfo Info = StaticMesh->GetSectionInfoMap().Get(LODToExport, SectionIndex);
                    if (StaticMesh->GetMaterial(Info.MaterialIndex) != StaticMeshComponent->GetMaterial(Info.MaterialIndex))
                    {
                        return false;
                    }
                }

                return true;
            };

            UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
            //export mesh to separate USD		
            if(ExportSettings.bExportAssetToSeperateUSD)
            {
                auto NewExportSettings = ExportSettings;

                if (StaticMeshComponent->IsA<USplineMeshComponent>())
                {
                    NewExportSettings.bForceShared = true;

                    if (NewExportSettings.bModular)
                    {
                        if (!ExportedMeshUSD.Contains(StaticMeshComponent->GetOwner()->GetFullName()))
                        {
                            ExportedMDLs.Reset();
                            FSimReadyAssetExportHelper::ResetGlobalCaches();
                            ExportedMeshUSD.Add(StaticMeshComponent->GetOwner()->GetFullName(), TEXT(""));
                        }
                    }
                }		

                // For modular mode, materials of mesh will be exported separately.
                // So materials will not be reuse across meshes.
                TMap<FString, FString> TempExportedMDLs;
                bool bUseTempMDLCache = NewExportSettings.bModular && !NewExportSettings.bForceShared;
                if(ExportedMeshUSD.Contains(MeshFullName) || ExportStaticMeshAsUSDToPathInternal(StaticMesh, StaticMeshComponent, MeshAbsolutePath,
                    MDLAbsoluteBasePath, false, NewExportSettings, bUseTempMDLCache ? TempExportedMDLs : ExportedMDLs))
                {
                    const int32 SMCurrentMinLOD = StaticMesh->MinLOD.GetValue();
                    int32 LODToExport = FMath::Clamp(StaticMeshComponent && StaticMeshComponent->bOverrideMinLOD ? StaticMeshComponent->MinLOD : SMCurrentMinLOD, 0, StaticMesh->RenderData->LODResources.Num() - 1);
                    int32 NumSections = StaticMesh->GetSectionInfoMap().GetSectionNumber(LODToExport);

                    if (NewExportSettings.bMeshInstanced)
                    {
                        // Can be instanced with Mesh USD directly
                        if (ShouldInstanceable(IsInstanced(StaticMeshComponent, LODToExport)))
                        {
                            BindAndRegisterPrim(true);
                        }
                        else // Override materials can't be instanced, we need to make new usd reference the mesh and override the material, then it can be instanced
                        {
                            // Get Mesh USD
                            if (!ExportedMeshUSD.Contains(MeshFullName))
                            {
                                ExportedMeshUSD.Add(MeshFullName, MeshAbsolutePath);
                            }
                            else
                            {
                                MeshAbsolutePath = *ExportedMeshUSD.Find(MeshFullName);
                            }

                            FString MaterialsFullName;
                            for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
                            {
                                FMeshSectionInfo Info = StaticMesh->GetSectionInfoMap().Get(LODToExport, SectionIndex);
                                auto ComponentMaterialInterface = StaticMeshComponent->GetMaterial(Info.MaterialIndex);

                                if (ComponentMaterialInterface)
                                {
                                    MaterialsFullName += ComponentMaterialInterface->GetFullName();
                                }
                                else
                                {
                                    MaterialsFullName += TEXT("Null_Material");
                                }
                            }

                            FString OverrideMeshAbsolutePath;						
                            FString SearchName = MeshFullName + MaterialsFullName;
                            if (!ExportedMeshUSD.Contains(SearchName))
                            {
                                FString RenameMeshAbsolutePath = MeshAbsolutePath;
                                while (ExportedMeshUSD.FindKey(RenameMeshAbsolutePath))
                                {
                                    RenameMeshAbsolutePath = MeshAbsoluteBasePath / MeshName + TEXT("_") + FString::FromInt(SuffixIndex++) + Extension;
                                }

                                OverrideMeshAbsolutePath = RenameMeshAbsolutePath;
                                // create new USD stage
                                MeshRelativePathToRoot = FSimReadyPathHelper::ComputeRelativePath(MeshAbsolutePath, OverrideMeshAbsolutePath);
                                MDLRelativeBasePathToRoot = FSimReadyPathHelper::ComputeRelativePath(MDLAbsoluteBasePath, OverrideMeshAbsolutePath, true);

                                ExportOverrideMeshAsUSDToPath(StaticMeshComponent, OverrideMeshAbsolutePath, MeshRelativePathToRoot, MeshAbsolutePath, LODToExport, MDLAbsoluteBasePath, MDLRelativeBasePathToRoot, NewExportSettings, ExportedMDLs);

                                // create new usd referenced mesh usd and override the materials
                                ExportedMeshUSD.Add(SearchName, OverrideMeshAbsolutePath);
                            }
                            else
                            {
                                OverrideMeshAbsolutePath = *ExportedMeshUSD.Find(SearchName);
                            }

                            FString OverrideMeshRelativePathToRoot = MeshRelativeBasePathToRoot / FPaths::GetCleanFilename(OverrideMeshAbsolutePath);

                            NewPrim = pxr::UsdGeomXform::Define(USDStage, PrimPath);
                            
                            auto ReferencedPrim = NewPrim;
                            // NOTE: Prim with the children can't be set instanceable, or the children will be lost. Having to add new prim to hold the instanced reference.
                            if (SceneComp.GetAttachChildren().Num() > 0)
                            {
                                pxr::SdfPath InstancedReferencedPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*(TEXT("Instanced") + PrimName))));
                                ReferencedPrim = pxr::UsdGeomXform::Define(USDStage, InstancedReferencedPrimPath);
                            }

                            auto RootPrim = ReferencedPrim.GetPrim();
                            AddReferenceOrPayload(NewExportSettings.bPayloads, RootPrim, OverrideMeshRelativePathToRoot);
                            RootPrim.SetInstanceable(true);
                        }
                    }
                    else
                    {
                        BindAndRegisterPrim(false);

                        FString MeshPrimName = FPaths::GetBaseFilename(MeshAbsolutePath);
                        while (MeshPrimName != FPaths::GetBaseFilename(MeshPrimName)) // Remove all extension
                        {
                            MeshPrimName = FPaths::GetBaseFilename(MeshPrimName);
                        }

                        auto MeshPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*MeshPrimName)));
                        pxr::UsdPrim USDMeshPrim = USDStage->GetPrimAtPath(MeshPrimPath);
                        bool IsCarlaVehicleLight = FUSDCARLAVehicleTools::IsCarlaVehicleLight(StaticMeshComponent);
                        if(USDMeshPrim || IsCarlaVehicleLight)
                        {
                            if (LODToExport < StaticMeshComponent->LODData.Num())
                            {
                                auto ColorVertexBuffer = StaticMeshComponent->LODData[LODToExport].OverrideVertexColors;
                                if (ColorVertexBuffer)
                                {
                                    FUSDExporter::ExportOverrideVertexColor(USDStage, MeshPrimPath, StaticMesh->RenderData->LODResources[LODToExport], ColorVertexBuffer);
                                }
                            }

                            for(int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
                            {
                                FMeshSectionInfo Info = StaticMesh->GetSectionInfoMap().Get(LODToExport, SectionIndex);
                                auto AssetMaterialInterface = StaticMesh->GetMaterial(Info.MaterialIndex);
                                auto ComponentMaterialInterface = StaticMeshComponent->GetMaterial(Info.MaterialIndex);
                                auto ParameterTimeSamples = GetMaterialParameterTimeSample(StaticMeshComponent, Info.MaterialIndex);

                                if (!ComponentMaterialInterface)
                                {
                                    continue;
                                }

                                TArray<pxr::UsdPrim> USDMeshPrims;
                                if (IsCarlaVehicleLight && SectionIndex == 0)
                                {
                                    auto LightsPath = (NumSections > 1 ? MeshPrimPath : PrimPath).AppendElementString("Lights");
                                    FindAllPrims<pxr::UsdGeomMesh>(USDStage->GetPrimAtPath(LightsPath), USDMeshPrims);
                                }
                                else
                                {
                                    auto Prim = NumSections == 1 ? USDMeshPrim : GetSectionPrimByIndex(USDStage, USDMeshPrim.GetPath(), SectionIndex);
                                    USDMeshPrims.Add(Prim);
                                }
                                int32 PrimIndex = 0;
                                while (PrimIndex < USDMeshPrims.Num())
                                {
                                    auto Prim = USDMeshPrims[PrimIndex];
                                    bool bAOMaskExported = TryExportAndBindMaterialWithAOMask(StaticMeshComponent, ParameterTimeSamples, LODToExport, MaterialOversStage ? MaterialOversStage : USDStage, ComponentMaterialInterface, Prim, NewExportSettings, MDLAbsoluteBasePath, MDLRelativeBasePathToRoot, bUseTempMDLCache ? TempExportedMDLs : ExportedMDLs);
                                    //Binding materials for component, skipping the ones that
                                    //component does not override the materials from static mesh.
                                    if (((!bAOMaskExported) && 
                                        ((!AssetMaterialInterface) || (AssetMaterialInterface->GetPathName() != ComponentMaterialInterface->GetPathName()))) ||
                                        (ParameterTimeSamples != nullptr))
                                    {
                                        ExportAndBindMaterial(MaterialOversStage ? MaterialOversStage : USDStage, ComponentMaterialInterface, ParameterTimeSamples, Prim, NewExportSettings, MDLAbsoluteBasePath, MDLRelativeBasePathToRoot, bUseTempMDLCache ? TempExportedMDLs : ExportedMDLs, TEXT(""), TEXT(""),
                                            [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FSimReadyExportMaterialResult& MaterialResult, FString& UsdMaterialName)
                                            {
                                                CheckUsdNameByNumTexCoord(InStage, InPrimPath, StaticMesh->RenderData->LODResources[LODToExport], MaterialResult, UsdMaterialName);
                                            },
                                            [&](const pxr::UsdStageRefPtr& InStage, const pxr::SdfPath& InPrimPath, const FString& MDLName, const FSimReadyExportMaterialResult& MaterialResult, const FString& UsdMaterialName)
                                            {
                                                BindNumTexCoord(InStage, InPrimPath, StaticMesh, LODToExport, MaterialResult);
                                            }
                                        );
                                        extern void ExportSimReadyNonVisMaterialDataForGeneralObject(const pxr::UsdPrim& MeshPrim, const UPrimitiveComponent& Component, const UMaterialInterface& IMaterial);
                                        ExportSimReadyNonVisMaterialDataForGeneralObject(Prim, *StaticMeshComponent, *ComponentMaterialInterface);
                                    }
                                    else if (!bAOMaskExported)
                                    {
                                        BindMaterialOverrides(MaterialOversStage, Prim, ComponentMaterialInterface, MDLRelativeBasePathToRoot, ExportedMDLs, NewExportSettings);
                                    }
                                    ++PrimIndex;
                                }

                                if (IsCarlaVehicleLight && SectionIndex == 0 && !FUSDCARLAVehicleTools::IsCarlaVehicleLightMaterial(AssetMaterialInterface))
                                {
                                    FUSDCARLAVehicleTools::BindMaterialsToSplitVehicleLight(MaterialOversStage ? MaterialOversStage : USDStage, USDMeshPrims, ComponentMaterialInterface);
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                NewPrim = ExportStaticMeshToUSD(StaticMesh, StaticMeshComponent, USDStage, PrimPath, RootLayerPath, ".", true, ExportSettings, ExportedMDLs);
            }

            SetupPrim(Exporter, NewPrim, &SceneComp);

            if (FUSDCARLAVehicleTools::BelongToCarlaVehicle(&SceneComp))
            {
                FUSDCARLAVehicleTools::ExportSimReadyVehicleComponentType(NewPrim.GetPrim(), StaticMeshComponent);
            }

            return NewPrim;
        }
        else if (bRootComponent && bHasBlueprintComponents)
        {
            return DefineEmptyXform();
        }
        else // check external object translator
        {
            auto USDModule = FModuleManager::GetModulePtr<ISimReadyUSDModule>("SimReadyUSD");
            if (USDModule && USDModule->HasUSDTranslator())
            {
                for(auto USDTranslator : USDModule->GetUSDTranslators())
                {
                    if(ExportSettings.bExportAssetToSeperateUSD)
                    {
                        if (USDTranslator->IsComponentSupported(&SceneComp))
                        {
                            if (!ExportedMeshUSD.Contains(MeshFullName))
                            {
                                pxr::UsdStageRefPtr NewUSDStage = CreateUSDStageFromPath(MeshAbsolutePath, false, !ExportSettings.bUpYAxis);
                                if (!NewUSDStage)
                                {
                                    break;
                                }

                                pxr::SdfPath RootPrimPath = InitDefaultPrim(NewUSDStage, pxr::KindTokens->component);
                                pxr::SdfPath NewPrimPath = RootPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*MeshName)));
                                USDTranslator->TranslateComponentToUSD(&SceneComp, NewUSDStage, NewPrimPath);

                                NewUSDStage->Save();
                            }

                            BindAndRegisterPrim(false);
                            break;
                        }
                    }
                    else
                    {
                        NewPrim = USDTranslator->TranslateComponentToUSD(&SceneComp, USDStage, PrimPath);
                        if (NewPrim)
                        {
                            break; // only translate the first valid component
                        }
                    }
                }
            }

            if (NewPrim)
            {
                SetupPrim(Exporter, NewPrim, &SceneComp);
                return NewPrim;
            }
        }
        
        bContinue = true;
        return pxr::UsdGeomXformable();
    };

    Exporter.CustomExport = CustomExport;

    auto Imageable = Exporter.ExportSceneObject(*SceneComp);

    // Export XODR ID for traffic signals
    if (Imageable && SceneComp->GetOwner()->GetRootComponent() == SceneComp)
    {
        ExportSimReadyTrafficLight(Imageable.GetPrim(), *SceneComp->GetOwner());
    }

    return (bool)Imageable;
}

void FSimReadyUSDHelper::SetTransfromForXForm(pxr::UsdGeomXformable & XForm, const pxr::GfMatrix4d & Transform)
{	
    bool bResetXFormStack = false;
    bool FoundTransformOp = false;
    
    auto XFormOps = XForm.GetOrderedXformOps(&bResetXFormStack);
    for (auto XFormOp : XFormOps)
    {
        // Found transform op, trying to set its value
        if (XFormOp.GetOpType() == pxr::UsdGeomXformOp::TypeTransform)
        {
            FoundTransformOp = true;
            XFormOp.Set(Transform);
            break;
        }
    }

    // If transformOp is not found, make a new one
    if (!FoundTransformOp)
    {
        pxr::UsdGeomXformOp MatrixXform = XForm.MakeMatrixXform();
        if (MatrixXform)
            MatrixXform.Set(Transform);
    }	
}

template<typename InstancedComponent>
bool FSimReadyUSDHelper::ExportInstancedMeshesToPointInstancerInternal(const TArray<InstancedComponent*>& InstancedStaticMeshComponents,
    const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs, TMap<FString, FString>& ExportedMeshUSD,
    bool bRenderData, const FString& ExportMeshPath)
{
    if (InstancedStaticMeshComponents.Num() == 0)
    {
        return false;
    }

    int32 InstanceCount = 0;
    for (int32 Index = 0; Index < InstancedStaticMeshComponents.Num(); ++Index)
    {
        auto InstancedComp = InstancedStaticMeshComponents[Index];
        if (bRenderData)
        {
            if (InstancedComp->PerInstanceRenderData.IsValid())
            {
                InstanceCount += InstancedComp->PerInstanceRenderData->InstanceBuffer_GameThread->GetNumInstances();
            }
        }
        else
        {
            InstanceCount += InstancedComp->GetInstanceCount();
        }
    }

    if (InstanceCount == 0)
    {
        return false;
    }

    pxr::UsdStageRefPtr USDStage = CreateUSDStageFromPath(ExportMeshPath, false, !ExportSettings.bUpYAxis);
    if (!USDStage)
    {
        return false;
    }
    pxr::SdfPath RootPrimPath = InitDefaultPrim(USDStage, pxr::KindTokens->assembly);
    auto PrimPath = RootPrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*FPaths::GetBaseFilename(ExportMeshPath))));

    auto PointInstancer = FSimReadyUSDHelper::ExportInstancedMeshesToPointInstancer(InstancedStaticMeshComponents, USDStage, PrimPath, ExportSettings, ExportedMDLs, ExportedMeshUSD, bRenderData, ExportMeshPath);

    if (PointInstancer)
    {
        USDStage->Save();
        return true;
    }
    

    return false;
}

template<typename InstancedComponent>
pxr::UsdGeomPointInstancer FSimReadyUSDHelper::ExportInstancedMeshesToPointInstancer(const TArray<InstancedComponent*>& InstancedStaticMeshComponents, const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& ExportPrimPath,
    const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMDLs, TMap<FString, FString>& ExportedMeshUSD, bool bRenderData, const FString& ExportMeshPath)
{
    pxr::UsdGeomPointInstancer PointInstancer;

    if (InstancedStaticMeshComponents.Num() == 0)
    {
        return PointInstancer;
    }

    if (USDStage)
    {
        int32 InstanceCount = 0;
        for (int32 Index = 0; Index < InstancedStaticMeshComponents.Num(); ++Index)
        {
            auto InstancedComp = InstancedStaticMeshComponents[Index];
            if (bRenderData)
            {
                if (InstancedComp->PerInstanceRenderData.IsValid())
                {
                    InstanceCount += InstancedComp->PerInstanceRenderData->InstanceBuffer_GameThread->GetNumInstances();
                }
            }
            else
            {
                InstanceCount += InstancedComp->GetInstanceCount();
            }
        }

        if (InstanceCount == 0)
        {
            return PointInstancer;
        }

        FScopedSlowTask ConvertTask(InstancedStaticMeshComponents.Num() + InstanceCount + 2, FText::FromString(TEXT("Converting Instanced Mesh...")));
        ConvertTask.Visibility = ESlowTaskVisibility::ForceVisible;
        ConvertTask.MakeDialog(false);
        ConvertTask.EnterProgressFrame();

        PointInstancer = pxr::UsdGeomPointInstancer::Define(USDStage, ExportPrimPath);
        pxr::UsdModelAPI(PointInstancer).SetKind(pxr::KindTokens->group);
        auto PrototypesRootPath = ExportPrimPath.AppendElementString(pxr::TfMakeValidIdentifier("Prototypes"));
        pxr::UsdGeomXform PrototypesPrim = pxr::UsdGeomXform::Define(USDStage, PrototypesRootPath);
        pxr::UsdModelAPI(PrototypesPrim).SetKind(pxr::KindTokens->group);

        pxr::VtArray<pxr::GfQuath> Orientations;
        pxr::VtArray<pxr::GfVec3f> Positions;
        pxr::VtArray<pxr::GfVec3f> Scales;
        pxr::VtArray<int> ProtoIndices;
        pxr::SdfPathVector PrototypesPaths;

        Orientations.resize(InstanceCount);
        Positions.resize(InstanceCount);
        Scales.resize(InstanceCount);
        ProtoIndices.resize(InstanceCount);

        int32 PointIndex = 0;
        for (int32 Index = 0; Index < InstancedStaticMeshComponents.Num(); ++Index)
        {
            ConvertTask.EnterProgressFrame();
            if (ConvertTask.ShouldCancel())
            {
                break;
            }
            auto InstancedMeshComp = InstancedStaticMeshComponents[Index];
            bool bValidRenderData = bRenderData && InstancedMeshComp->PerInstanceRenderData.IsValid();
            int32 NumInstance = bValidRenderData
                ? InstancedMeshComp->PerInstanceRenderData->InstanceBuffer_GameThread->GetNumInstances()
                : InstancedMeshComp->GetInstanceCount();

            if (NumInstance > 0 && InstancedMeshComp->GetStaticMesh())
            {
                auto PrototypesPrimPath = PrototypesRootPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_ANSI(*InstancedMeshComp->GetName())));
                auto MeshName = InstancedMeshComp->GetStaticMesh()->GetName();
                auto MeshFullName = InstancedMeshComp->GetStaticMesh()->GetFullName();
                FString MeshAbsoluteBasePath;
                FString MeshRelativeBasePathToRoot;
                FString MDLAbsoluteBasePath;

                // If all props should be exported to their own directory
                if(ExportSettings.bModular)
                {
                    FString CheckedName = FSimReadyAssetExportHelper::GetLegalMDLName(MeshName);
                    MeshAbsoluteBasePath = FPaths::GetPath(FPaths::GetPath(ExportMeshPath)) / CheckedName;
                    MeshRelativeBasePathToRoot = "../" + CheckedName;
                    MDLAbsoluteBasePath = MeshAbsoluteBasePath;
                }
                else
                {
                    MeshAbsoluteBasePath = FPaths::GetPath(ExportMeshPath);
                    MeshRelativeBasePathToRoot = ".";
                    MDLAbsoluteBasePath = FPaths::GetPath(MeshAbsoluteBasePath) / "Materials";
                }

                FString Extension;
                if(ExportSettings.bAddExtraExtension)
                {
                    Extension = ExportSettings.bAsciiFormat ? ".prop.usda" : ".prop.usd";
                }
                else
                {
                    Extension = GetUSDExtension(ExportSettings.bAsciiFormat);
                }

                FString MeshAbsolutePath = MeshAbsoluteBasePath / MeshName + Extension;
                FString MeshRelativePathToRoot = MeshRelativeBasePathToRoot / MeshName + Extension;
    
                if(!ExportedMeshUSD.Contains(MeshFullName))
                {
                    FString RenameMeshAbsolutePath = MeshAbsolutePath;
                    while(ExportedMeshUSD.FindKey(RenameMeshAbsolutePath))
                    {
                        RenameMeshAbsolutePath = MeshAbsoluteBasePath / MeshName + TEXT("_") + FString::FromInt(SuffixIndex++) + Extension;
                    }

                    MeshAbsolutePath = RenameMeshAbsolutePath;
                    MeshRelativePathToRoot = MeshRelativeBasePathToRoot / FPaths::GetCleanFilename(MeshAbsolutePath);
                }			
    
                TMap<FString, FString> TempExportedMDLs;
                if(ExportedMeshUSD.Contains(MeshFullName) || ExportStaticMeshAsUSDToPathInternal(InstancedMeshComp->GetStaticMesh(), InstancedMeshComp, MeshAbsolutePath,
                    MDLAbsoluteBasePath, false, ExportSettings, ExportSettings.bModular ? TempExportedMDLs : ExportedMDLs))
                {
                    if(!ExportedMeshUSD.Contains(MeshFullName))
                    {
                        ExportedMeshUSD.Add(MeshFullName, MeshAbsolutePath);
                    }
                    else
                    {
                        MeshAbsolutePath = *ExportedMeshUSD.Find(MeshFullName);
                        MeshRelativePathToRoot = FPaths::GetPath(MeshRelativePathToRoot) / FPaths::GetCleanFilename(MeshAbsolutePath);
                    }

                    auto NewPrim = pxr::UsdGeomXform::Define(USDStage, PrototypesPrimPath);

                    auto RootPrim = NewPrim.GetPrim();
                    AddReferenceOrPayload(ExportSettings.bPayloads, RootPrim, MeshRelativePathToRoot);
                    
                    int32 ProtoIndex = PrototypesPaths.size();
                    PrototypesPaths.push_back(PrototypesPrimPath);

                    for (int32 InstanceIndex = 0; InstanceIndex < NumInstance; ++InstanceIndex)
                    {
                        ConvertTask.EnterProgressFrame();
                        if (ConvertTask.ShouldCancel())
                        {
                            break;
                        }
                        FTransform InstanceTransform;
                        if (bValidRenderData)
                        {
                            FMatrix TransformMat;
                            InstancedMeshComp->PerInstanceRenderData->InstanceBuffer_GameThread->GetInstanceTransform(InstanceIndex, TransformMat);
                            InstanceTransform = FTransform(TransformMat);
                        }
                        else
                        {
                        InstancedMeshComp->GetInstanceTransform(InstanceIndex, InstanceTransform);
                        }
                        InstanceTransform *= InstancedMeshComp->GetRelativeTransform();
                        InstanceTransform = RHSTransformConvert(USDStage, InstanceTransform, false);

                        ProtoIndices[PointIndex] = ProtoIndex;
                        Scales[PointIndex] = USDConvert(InstanceTransform.GetScale3D());
                        Positions[PointIndex] = USDConvert(InstanceTransform.GetTranslation());
                        Orientations[PointIndex] = pxr::GfQuath(USDConvert(InstanceTransform.GetRotation()));
                        ++PointIndex;
                    }
                }
            }
        }

        ConvertTask.EnterProgressFrame();
        PointInstancer.CreatePositionsAttr(pxr::VtValue(Positions));
        PointInstancer.CreateOrientationsAttr(pxr::VtValue(Orientations));
        PointInstancer.CreateProtoIndicesAttr(pxr::VtValue(ProtoIndices));
        PointInstancer.CreateScalesAttr(pxr::VtValue(Scales));
        PointInstancer.CreatePrototypesRel().SetTargets(PrototypesPaths);
        pxr::VtVec3fArray Extent;
        pxr::UsdGeomBoundable Boundable(PointInstancer.GetPrim());
        pxr::UsdGeomBoundable::ComputeExtentFromPlugins(Boundable, pxr::UsdTimeCode::Default(), &Extent);
        PointInstancer.CreateExtentAttr().Set(Extent);
    }

    return PointInstancer;
}

void FSimReadyUSDHelper::ExportModelToUSD(UModel* Model, const pxr::UsdStageRefPtr& USDStage, const pxr::UsdStageRefPtr& MaterialOversStage, const FSimReadyExportSettings& ExportSettings, TMap<FString, FString>& ExportedMeshUSD, TMap<FString, FString>& ExportedMDLs)
{
    if (!Model)
    {
        return;
    }

    if (Model->VertexBuffer.Vertices.Num() == 0)
    {
        return;
    }

    FString RootLayerPath = FPaths::GetPath(USDStage->GetRootLayer()->GetIdentifier().c_str());	
    FString MeshName = Model->GetName();
    FString MeshFullName = Model->GetFullName();
    FString MDLAbsoluteBasePath;
    FString MeshAbsoluteBasePath;
    FString MeshRelativeBasePathToRoot;

    // If all props should be exported to their own directory
    if(ExportSettings.bModular)
    {
        FString CheckedName = FSimReadyAssetExportHelper::GetLegalMDLName(MeshName);
        MeshAbsoluteBasePath = RootLayerPath / "Props" / CheckedName;
        MeshRelativeBasePathToRoot = "./Props/" + CheckedName;
        MDLAbsoluteBasePath = MeshAbsoluteBasePath;
    }
    else
    {
        MeshAbsoluteBasePath = RootLayerPath / "Props";
        MeshRelativeBasePathToRoot = "./Props";
        MDLAbsoluteBasePath = RootLayerPath / "Materials";
    }

    FString Extension;
    if(ExportSettings.bAddExtraExtension)
    {
        Extension = ExportSettings.bAsciiFormat ? ".prop.usda" : ".prop.usd";
    }
    else
    {
        Extension = GetUSDExtension(ExportSettings.bAsciiFormat);
    }

    FString MeshAbsolutePath = MeshAbsoluteBasePath / MeshName + Extension;
    FString MeshRelativePathToRoot = MeshRelativeBasePathToRoot / MeshName + Extension;
    
    if(!ExportedMeshUSD.Contains(MeshFullName))
    {
        FString RenameMeshAbsolutePath = MeshAbsolutePath;
        while(ExportedMeshUSD.FindKey(RenameMeshAbsolutePath))
        {
            RenameMeshAbsolutePath = MeshAbsoluteBasePath / MeshName + TEXT("_") + FString::FromInt(SuffixIndex++) + Extension;
        }

        MeshAbsolutePath = RenameMeshAbsolutePath;
        MeshRelativePathToRoot = MeshRelativeBasePathToRoot / FPaths::GetCleanFilename(MeshAbsolutePath);
    }

    // Model is only valid in Level, always using separated USD to export
    pxr::UsdStageRefPtr NewUSDStage = CreateUSDStageFromPath(MeshAbsolutePath, false, !ExportSettings.bUpYAxis);
    if (!NewUSDStage)
    {
        return;
    }

    pxr::SdfPath RootPrimPath = InitDefaultPrim(NewUSDStage, pxr::KindTokens->component);

    auto USDMesh = FUSDExporter::ExportBSP(NewUSDStage, *Model, MeshName);
    if (USDMesh)
    {
        if(!ExportedMeshUSD.Contains(MeshFullName))
        {
            ExportedMeshUSD.Add(MeshFullName, MeshAbsolutePath);
        }

        TMap<FString, FString> TempExportedMDLs;
        FString MDLRelativeBasePath = FSimReadyPathHelper::ComputeRelativePath(MDLAbsoluteBasePath, MeshAbsolutePath, true);

        // Binding and exporting material
        TArray<UMaterialInterface*> Materials;
        Model->MaterialIndexBuffers.GetKeys(Materials);
        if (Materials.Num() > 0)
        {
            UMaterialInterface* MaterialInt = Materials[0];
            if (MaterialInt)
            {
                ExportAndBindMaterial(NewUSDStage, MaterialInt, nullptr, USDMesh.GetPrim(), ExportSettings, MDLAbsoluteBasePath, MDLRelativeBasePath, ExportSettings.bModular ? TempExportedMDLs : ExportedMDLs);
            }
        }

        // Bind to Main USD
        auto PrimPath = USDMesh.GetPath();
        auto GeomXform = pxr::UsdGeomXform::Define(USDStage, PrimPath);
        auto GeomXformPrim = GeomXform.GetPrim();
        AddReferenceOrPayload(ExportSettings.bPayloads, GeomXformPrim, MeshRelativePathToRoot);
        
        if (ExportSettings.bMeshInstanced)
        {
            GeomXformPrim.SetInstanceable(true);
        }
        else if (Materials.Num() > 0)
        {
            FString MDLRelativeBasePathToRoot = ExportSettings.bModular ? MeshRelativeBasePathToRoot : TEXT("./Materials");
            auto MeshPrimPath = PrimPath.AppendElementString(pxr::TfMakeValidIdentifier(TCHAR_TO_UTF8(*MeshName)));
            pxr::UsdPrim USDMeshPrim = USDStage->GetPrimAtPath(MeshPrimPath);
            if (USDMeshPrim)
            {
                BindMaterialOverrides(MaterialOversStage, USDMeshPrim, Materials[0], MDLRelativeBasePathToRoot, ExportedMDLs, ExportSettings);
            }
        }
    }
}

void FSimReadyUSDHelper::BindMaterialOverrides(const pxr::UsdStageRefPtr& USDStage, const pxr::UsdPrim& Prim, const UMaterialInterface* MaterialInt, const FString& MDLRelativePath, const TMap<FString, FString>& ExportedMDLs, const FSimReadyExportSettings& ExportSettings)
{
    if (ExportSettings.bMDL && ExportSettings.bMaterialOverSublayer && USDStage && MaterialInt)
    {
        pxr::SdfPath ScopePrimPath;
        if (ExportSettings.bModular)
        {
            ScopePrimPath = Prim.GetPath().GetParentPath();
        }
        else
        {
            ScopePrimPath = USDStage->GetDefaultPrim().GetPath();
        }
        ScopePrimPath = ScopePrimPath.AppendElementString(pxr::UsdUtilsGetMaterialsScopeName().GetString());

        auto CachedUsdMaterialName = ExportedMDLs.FindKey(MaterialInt->GetFullName());
        if (CachedUsdMaterialName)
        {
            auto RefUsdMaterial = BindMaterial(USDStage, Prim, ScopePrimPath, *CachedUsdMaterialName);
            FString RelativeUsdMaterialPath = MDLRelativePath / *CachedUsdMaterialName + GetUSDExtension(ExportSettings.bAsciiFormat);
            RefUsdMaterial.GetPrim().GetReferences().AddReference(TCHAR_TO_UTF8(*RelativeUsdMaterialPath));
        }
    }
}