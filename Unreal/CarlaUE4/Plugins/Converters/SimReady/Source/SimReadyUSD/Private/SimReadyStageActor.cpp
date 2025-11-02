// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyStageActor.h"
#include "SimReadyUSDLog.h"
#include "SimReadyUsdLuxLightCompat.h"
#include "Algo/Find.h"
#include "Misc/CoreDelegates.h"
#include "UObject/Package.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#if WITH_EDITOR
#include "MeshDescriptionOperations.h"
#endif
#include "Animation/Skeleton.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Camera/CameraComponent.h"
#include "CinematicCamera/Public/CineCameraActor.h"
#include "CinematicCamera/Public/CineCameraComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Animation/AnimSequence.h"
#include "Rendering/SkeletalMeshLODImporterData.h"
#include "MeshDescription.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/RectLight.h"
#include "Engine/SkyLight.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "ActorFactories/ActorFactoryBasicShape.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "ActorFactories/ActorFactoryBoxVolume.h"
#include "Engine/PostProcessVolume.h"
#include "ActorSequenceComponent.h"
#include "ActorSequence.h"
#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "AssetSelection.h"
#include "Editor/EditorEngine.h"
#include "Editor/TransBuffer.h"
#include "Engine/Selection.h"
#include "ScopedTransaction.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "EditorModeManager.h"
#include "LevelEditor.h"
#include "EditorFramework/AssetImportData.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
extern UNREALED_API class UEditorEngine* GEditor;
#endif
#include "TimerManager.h"
#include "USDConverter.h"
#include "SimReadyMDL.h"
#include "SimReadyTexture.h"
#include "SimReadySettings.h"
#include "ISimReadyRuntimeModule.h"
#include "SimReadyUSDSequenceImporter.h"
#include "SimReadyAssetImportHelper.h"
#include "SimReadyNotificationHelper.h"
#include "SimReadyUSDTokens.h"

#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "SimReadyUSDHelper.h"
#include "Features/IModularFeatures.h"
#include "Logging/LogMacros.h"
#include "Extractors/TimeSamplesData.h"
#include "USDGeometryCache.h"
#include "USDHashGenerator.h"
#include "SimReadyReferenceCollector.h"
#include "SimReadySlowTask.h"
#include "SimReadyUSDImporterHelper.h"
#include "USDCARLAVehicleTools.h"

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#undef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include "USDDerivedDataCache.h"

#define LIGHTMAPS_EXPORT_PRIMVARS 1
#define LIGHTMAPS_EXPORT_SCHEMA 0

#if LIGHTMAPS_EXPORT_PRIMVARS || LIGHTMAPS_EXPORT_SCHEMA
#include "Engine/StaticMesh.h"
#include "Engine/MapBuildDataRegistry.h"
#include "Runtime/Engine/Public/ImageUtils.h"
#include "Runtime/Engine/Public/LightMap.h"
#include "Runtime/Engine/Public/ShadowMap.h"
#include "Runtime/Engine/Classes/Engine/ShadowMapTexture2D.h"
#endif

#if LIGHTMAPS_EXPORT_SCHEMA
#include <LightmapsSchema/lightmap.h>
#include <LightmapsSchema/shadowmap.h>
#endif

#define LOCTEXT_NAMESPACE "SimReadyStageActor"


#if LIGHTMAPS_EXPORT_PRIMVARS || LIGHTMAPS_EXPORT_SCHEMA
static inline FString FixupTextureName(UTexture2D* Texture)
{
    return Texture->GetPathName().Replace(TEXT("."), TEXT("_")).Replace(TEXT(":"), TEXT("_"));
}
#endif

UNREALED_API class FEditorModeTools& GLevelEditorModeTools();

FString ASimReadyStageActor::ExportUSDToString()
{
    FString StageString;
    if(GetUSDStage())
    {
        std::string USDString;
        GetUSDStage()->ExportToString(&USDString);

        StageString = USDString.c_str();
    }

    return MoveTemp(StageString);
}

bool ASimReadyStageActor::HasValidImportStage() const
{
    return ImportUSDStage != nullptr;
}

ASimReadyStageActor::ASimReadyStageActor()
{
    USDSequenceImporter = MakeShareable(new FSimReadyUSDSequenceImporter(this));
    ReservedUE4ObjectOuter = NewObject<USceneComponent>(this, "ReservedObjectOuter", EObjectFlags::RF_Transient);
}

ASimReadyStageActor::~ASimReadyStageActor()
{
    USDSequenceImporter = nullptr;
}

ASimReadyStageActor& ASimReadyStageActor::Get(UWorld& World)
{
    if(auto StageActor = Find(World))
    {
        return *StageActor;
    }
    else
    {
        return *World.SpawnActor<ASimReadyStageActor>();
    }
}

ASimReadyStageActor * ASimReadyStageActor::Find(UWorld & World)
{
    for(FActorIterator It(&World); It; ++It)
    {
        if(It->IsA<ASimReadyStageActor>())
        {
            auto OmniStageActor = StaticCast<ASimReadyStageActor*>(*It);
            return OmniStageActor;
        }
    }

    return nullptr;
}

void ASimReadyStageActor::OnUSDStageNotice(const pxr::SdfPath& Path, bool bResync)
{
    // Set sync flag
    auto StatePtr = SetStateScoped(EState::Loading);
    if(!StatePtr.IsValid())
    {
        return;
    }

    ChangedUSDPaths.Paths.Add(Path);
    ChangedUSDPaths.ResyncFlag.Add(bResync);
    if(!ChangedUSDPaths.DelegateHandle.IsValid())
    {
        ChangedUSDPaths.DelegateHandle = FCoreDelegates::OnBeginFrame.AddUObject(this, &ASimReadyStageActor::LoadChangedUSDPaths);
    }
}

void ASimReadyStageActor::LoadChangedUSDPaths()
{
    // Load each path
    for(int32 Index = 0; Index < ChangedUSDPaths.Paths.Num(); ++Index)
    {
        auto Path = ChangedUSDPaths.Paths[Index];
        auto Resync = ChangedUSDPaths.ResyncFlag[Index];
        if (Resync)
        {
            ResyncUSD(Path);
        }
        LoadUSD(Path, Resync);
    }

    // Clear up
    ChangedUSDPaths.Paths.Reset();
    ChangedUSDPaths.ResyncFlag.Reset();

    FCoreDelegates::OnBeginFrame.Remove(ChangedUSDPaths.DelegateHandle);
    ChangedUSDPaths.DelegateHandle.Reset();

#if WITH_EDITOR
    GEditor->RedrawLevelEditingViewports(false);
#endif
}

void ASimReadyStageActor::ResyncUSD(const pxr::SdfPath& Path)
{
    //Set flag
    auto StatePtr = SetStateScoped(EState::Loading);
    if(!StatePtr.IsValid())
    {
        return;
    }

    auto RemoveComponents = [&](const TArray<FString>& RemovedKeys, bool bSkipCamera)
    {
        // Delete components
        bool bActorDestroyed = false;
        TSet<USceneComponent*> HandledCineCameraComponents;
        for (auto Key : RemovedKeys)
        {
            auto FindObject = USDPathToObject.Find(Key);
            if (FindObject == nullptr)
            {
                continue;
            }

            auto SceneObject = *FindObject;
            // Destroy owner if it's the root component
            if(!SceneObject.IsValid())
            {
                continue;
            }

            USceneComponent* SceneComp = Cast<USceneComponent>(SceneObject);
            AActor* OwnerActor = SceneComp ? SceneComp->GetOwner() : nullptr;

            if (bSkipCamera)
            {
                // Skip destroying camera if necessary
                if(OwnerActor && OwnerActor->IsA<ACineCameraActor>())
                {
                    // TODO: This is dirty WAR to avoid destroy and recreate cine camera
                    // as it's possible to cause lost track of sequencer
                    auto CameraRootComponent = OwnerActor->GetRootComponent();
                    if(!HandledCineCameraComponents.Contains(CameraRootComponent))
                    {
                        if(ToggleCineCameraVisibility(CameraRootComponent))
                        {
                            HandledCineCameraComponents.Add(CameraRootComponent);
                            continue;
                        }
                    }
                    else
                    {
                        continue;
                    }
                }
            }

            // Remove from world
            if(OwnerActor && SceneComp && OwnerActor->GetRootComponent() == SceneComp)
            {
                if(!OwnerActor->IsPendingKillPending() && !OwnerActor->HasAllFlags(EObjectFlags::RF_BeingRegenerated))
                {
                    ReserveObject(*OwnerActor, FSimReadyPathHelper::KeyToPrimPath(Key));

                    bActorDestroyed = true;
                }
            }
            else
            {
                ReserveObject(*SceneObject, FSimReadyPathHelper::KeyToPrimPath(Key));
            }

            // Remove from path list
            USDPathToObject.Remove(Key);
        }

        // Notify editor
        if(bActorDestroyed)
        {
            GEngine->BroadcastLevelActorListChanged();
        }
    };

    // Remove all the children
    TArray<FString> RemovedKeys;
    auto ResyncPrimPath = Path.GetText();
    for (auto Pair : USDPathToObject)
    {
        auto EntryPath = FSimReadyPathHelper::KeyToPrimPath(Pair.Key);
        if(IsSameOrChildPrimPath(ResyncPrimPath, EntryPath))
        {
            RemovedKeys.Add(Pair.Key);
        }
    }

    RemoveComponents(RemovedKeys, false);
}

void ASimReadyStageActor::LoadUSD(const pxr::SdfPath& Path, bool bLoadChildren)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("SimReadyStageActorLoadUSD"), STAT_SimReadyStageActorLoadUSD, STATGROUP_SimReady);

#if UE_BUILD_DEBUG
    const FString PathText = Path.GetText();
#endif

    //Set flag
    auto StatePtr = SetStateScoped(EState::Loading);
    if(!StatePtr.IsValid())
    {
        return;
    }

    // Create parent
    auto Prim = GetUSDStage()->GetPrimAtPath(Path.GetAbsoluteRootOrPrimPath());
    if(!Prim)
    {
        return;
    }

    // Filter the prim which is either a master prim or a descendent of a master prim.
    if (Prim.IsInMaster())
    {
        return;
    }

    if (!Prim.IsActive())
    {
        return;
    }

    // Prim filtering from import options
    for (const FString& PrimSubstring : ImportSettings.PathSubstringsToIgnore)
    {
        pxr::SdfPath PrimSubstringSdfPath = ToUSDPath(*PrimSubstring);
        if (Path.GetString().find(PrimSubstringSdfPath.GetString()) != std::string::npos)
        {
            UE_LOG(LogSimReadyUsd, Warning, TEXT("Skipping import: %s"), *FString(Path.GetText()));
            return;
        }
    }

    auto FromSkelRootPrim = IsPrimFromUsdSchema<pxr::UsdSkelRoot>(Prim);
    // Check if skeletal mesh
    if(Prim.IsA<pxr::UsdSkelRoot>())
    {
        LoadSkeletalMeshComponent(Path);

        if (HasValidImportStage())
        {
            if (ImportSettings.bImportUnusedReferences)
            {
                FromSkelRootPrim = Prim;
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }
    }
    else if (!HasValidImportStage() && FromSkelRootPrim)
    {
        LoadSkeletalMeshComponent(FromSkelRootPrim.GetPath());
        return;
    }

    if(!FromSkelRootPrim && Prim.IsA<pxr::UsdGeomXformable>())
    {
        TFunction<void(const pxr::UsdPrim&)> CreateParent;
        CreateParent = [&CreateParent, this](const pxr::UsdPrim& Prim)
        {
            if (!Prim)
            {
                return;
            }

            if(Prim.IsPseudoRoot())
            {
                return;
            }
            
            if(auto SceneComp = FindObjectFromPath<USceneComponent>(Prim.GetPath()))
            {
                if (!Prim.GetParent())
                {
                    return;
                }

                if (auto ParentComp = FindObjectFromPath<USceneComponent>(Prim.GetParent().GetPath()))
                {
                    if (SceneComp->GetAttachParent() == ParentComp)
                    {
                        return;
                    }
                }
                else
                {
                    return;
                }
            }

            CreateParent(Prim.GetParent());

            LoadSceneComponent(Prim.GetPath());
        };

        CreateParent(Prim.GetParent());
    }

    if(!FromSkelRootPrim && Prim.IsA<pxr::UsdGeomMesh>())
    {
        if (IsPrimFromUsdSchema<pxr::UsdGeomPointInstancer>(Prim))
        {
            LoadPointInstancerMesh(Path, bLoadChildren);
        }
        else
        {
            LoadStaticMeshComponent<UStaticMeshComponent, AStaticMeshActor>(Path, bLoadChildren);
        }
    }
    else if(!FromSkelRootPrim && Prim.IsA<pxr::UsdGeomSubset>())	// Geometry subset
    {
        auto GeomSubset = pxr::UsdGeomSubset(Prim);
        do
        {
            // Check mesh prim
            auto USDMesh = pxr::UsdGeomMesh(Prim.GetParent());
            if(!USDMesh)
            {
                break;
            }

            // Skip if parent mesh is reloaded
            auto ReloadedParent = Algo::FindByPredicate(ChangedUSDPaths.Paths,
                [&](const pxr::SdfPath& Path)
                {
                    return Prim.GetPath().HasPrefix(Path);
                }
            );

            if(ReloadedParent)
            {
                break;
            }

            // Load geometry
            auto MeshComponent = FindObjectFromPath<UStaticMeshComponent>(USDMesh.GetPath());
            if(!MeshComponent)
            {
                break;
            }

            if(ShouldLoadProperty(Path, pxr::UsdGeomSubset::GetSchemaAttributeNames(false)))
            {
                LoadStaticMesh(USDMesh, *MeshComponent);
            }

            // Load material binding
            if(ShouldLoadProperty(Path, {pxr::UsdShadeTokens->materialBinding}))
            {
                LoadMaterial(*MeshComponent, USDMesh, GeomSubset);
            }

            LoadSceneComponent(Path, bLoadChildren);
        } while(false);
    }
    else if(!FromSkelRootPrim && SimReadyUsdLuxLightCompat::PrimIsALight(Prim))	// Lights
    {
        if(ImportSettings.bImportLights && LoadLight(Path, bLoadChildren))
        {
            for(auto Actor : PreviewEnvironmentActors)
            {
                if(Actor.IsValid() && Actor->IsA<ADirectionalLight>())
                {
                    GetWorld()->DestroyActor(Actor.Get(), false, false);
                }
            }
        }
    }
    else if (Prim.IsA<pxr::UsdShadeMaterial>())
    {
        pxr::UsdShadeConnectableAPI Source;
        pxr::TfToken SourceName;
        pxr::UsdShadeAttributeType SourceType;

        auto USDMaterial = pxr::UsdShadeMaterial(Prim);
        if (USDMaterial)
        {
            bool bImportAll = HasValidImportStage() && ImportSettings.bImportUnusedReferences;

            auto MdlSurfaceOutput = USDMaterial.GetSurfaceOutput(USDTokens.mdl);
            if (MdlSurfaceOutput)
            {
                if (MdlSurfaceOutput.GetConnectedSource(&Source, &SourceName, &SourceType))
                {
                    auto MDLShader = pxr::UsdShadeShader(Source);
                    if (MDLShader)
                    {
                        auto MaterialInst = FindObjectFromPath<UMaterialInstanceConstant>(MDLShader.GetPath());

                        if (MaterialInst)
                        {
                            UpdateShadeInputs(MaterialInst, MDLShader);
                        }
                        else if (bImportAll)
                        {
                            // Import unused materials
                            LoadMaterial(nullptr, 0, pxr::UsdGeomMesh(), pxr::UsdShadeShader(), MDLShader);
                        }
                    }
                }
            }

            // Check preview surface as well
            if (bImportAll)
            {
                auto SurfaceOutput = USDMaterial.GetSurfaceOutput();

                if (SurfaceOutput)
                {
                    if (SurfaceOutput.GetConnectedSource(&Source, &SourceName, &SourceType))
                    {
                        auto Shader = pxr::UsdShadeShader(Source);	
                        if (Shader)
                        {
                            auto MaterialInst = FindObjectFromPath<UMaterialInstanceConstant>(Shader.GetPath());
                            if (MaterialInst == nullptr)
                            {
                                LoadMaterial(nullptr, 0, pxr::UsdGeomMesh(), Shader, pxr::UsdShadeShader());
                            }
                        }
                    }
                }
            }
        }
    }
    else if (Prim.IsA<pxr::UsdShadeNodeGraph>())
    {
#ifdef TODO_USD_UI_NODE_GRAPH_TYPEID
        if (Prim.HasAPI<pxr::UsdUINodeGraphNodeAPI>())
        {
            UpdateGraphNodePrim(Path);
        }
#endif
    }
    else if(Prim.IsA<pxr::UsdShadeShader>())	// Material
    {
#ifdef TODO_USD_UI_NODE_GRAPH_TYPEID
        if (Prim.HasAPI<pxr::UsdUINodeGraphNodeAPI>())
#else
        if (false)
#endif
        {
            UpdateGraphNodePrim(Path);
        }
        else
        {
            auto USDShader = pxr::UsdShadeShader(Prim);
            auto MaterialInst = FindObjectFromPath<UMaterialInstance>(Path.GetPrimPath());
            if (MaterialInst)
            {
                if (MaterialInst->IsA<UMaterialInstanceDynamic>())
                {
                    FUSDConversion::SyncMaterialParameters(*MaterialInst, USDShader, FUSDConversion::EDirection::Import);
                }
                else if (MaterialInst->IsA<UMaterialInstanceConstant>())
                {
                    auto MaterialInstConst = Cast<UMaterialInstanceConstant>(MaterialInst);
                    if (IsPreviewSurface(MaterialInstConst))
                    {
                        FSimReadyUSDImporterHelper::UpdatePreviewSurfaceInputs(
                            MaterialInstConst,
                            USDShader,
                            nullptr,
                            [&](const FString& PrimName)
                            {
                                USDPathToObject.FindOrAdd(FSimReadyPathHelper::PrimPathToKey(PrimName), MaterialInstConst);
                            },
                            [&](const uint8* Content, uint64 Size, const FString& FileName, UTexture*& OutTexture)
                            {
                                CreateTextureFromBuffer(Content, Size, FileName, OutTexture);
                            }
                        );
                    }
                    else
                    {
                        UpdateShadeInputs(MaterialInstConst, USDShader);
                    }
                }
            }
        }
    }
    else if(!FromSkelRootPrim && Prim.IsA<pxr::UsdGeomCamera>())	// Camera
    {
        UCameraComponent* Camera = nullptr;
        // Check if there's already an ACameraActor, if not, creating or getting an ACineCameraActor
        auto SceneComponent = FindObjectFromPath<USceneComponent>(Prim.GetPath());
        if (SceneComponent)
        {		
            if(SceneComponent->GetOwner() && SceneComponent->GetOwner()->IsA<ACameraActor>() && SceneComponent->GetOwner()->GetRootComponent() == SceneComponent)
            {
                auto CameraActor = Cast<ACameraActor>(SceneComponent->GetOwner());
                Camera = CameraActor->GetCameraComponent();
            }
        }

        if (Camera == nullptr)
        {
            Camera = CreateActorAndComponent<UCineCameraComponent, ACineCameraActor>(Prim, true);
        }

        if(Camera && ShouldLoadProperty(Path, pxr::UsdGeomCamera::GetSchemaAttributeNames(false)))
        {
            pxr::UsdGeomCamera CameraPrim(Prim);
            FNamedParameterTimeSamples NamedParameterTimeSamples;		
            USDImportCamera(CameraPrim, *Camera, NamedParameterTimeSamples);
            USDSequenceImporter->CreateObjectTrack(Camera, NamedParameterTimeSamples);
        }
        LoadSceneComponent(Path, bLoadChildren);
    }
    else if(!FromSkelRootPrim && Prim.IsA<pxr::UsdGeomGprim>())	// Basic shape
    {
        LoadBasicShape(Path, bLoadChildren);
    }
    else if(!FromSkelRootPrim && Prim.IsA<pxr::UsdGeomPointInstancer>())
    {
        LoadPointInstancer(Path, bLoadChildren);
    }
    else if(!FromSkelRootPrim && Prim.IsA<pxr::UsdGeomXformable>())	// Load scene component
    {
        LoadSceneComponent(Path, bLoadChildren);
    }
    else if(Path.IsAbsoluteRootOrPrimPath())
    {
        auto ChildPrims = Prim.GetFilteredChildren(pxr::UsdTraverseInstanceProxies());
        for(auto ChildPrim : ChildPrims)
        {
            LoadUSD(ChildPrim.GetPath(), bLoadChildren);
        }
    }

    // Load default prim
    if(Path == pxr::SdfPath::AbsoluteRootPath())
    {
        LoadDefaultPrim();
    }
}

void ASimReadyStageActor::LoadStaticMeshFromGeomGprim(const pxr::SdfPath& Path, UStaticMeshComponent& MeshComponent, bool bLoadChildren, bool bIgnoreVisibility)
{
    pxr::UsdGeomGprim USDGprim = pxr::UsdGeomGprim::Get(GetUSDStage(), Path.GetPrimPath());
    if(!USDGprim)
    {
        return;
    }

    // Load material binding
    if(ShouldLoadProperty(Path, {pxr::UsdShadeTokens->materialBinding}))
    {
        LoadMaterial(MeshComponent, USDGprim);
    }

    // Cast shadow
    // We should also check parent prim if there's no "doNotCastShadows" primvar in current prim
    pxr::SdfPath CurrentPath = Path;
    pxr::UsdGeomXform CurrentPrim = pxr::UsdGeomXform(USDGprim);
    do
    {
        if(ShouldLoadProperty(CurrentPath, { USDTokens.doNotCastShadows }))
        {
            auto CastShadowVar = CurrentPrim.GetPrimvar(USDTokens.doNotCastShadows);
            bool bNotCastShadow = false;
            if (CastShadowVar && CastShadowVar.Get<bool>(&bNotCastShadow))
            {
                MeshComponent.CastShadow = !bNotCastShadow;
                MeshComponent.MarkRenderStateDirty();
                break;
            }
        }

        CurrentPath = CurrentPath.GetParentPath();
        CurrentPrim = pxr::UsdGeomXform::Get(GetUSDStage(), CurrentPath.GetPrimPath());
    }
    while(CurrentPrim);

    LoadSceneComponent(Path, bLoadChildren);
}

void ASimReadyStageActor::LoadAnimationSequence(USkeletalMeshComponent* SkeletalMeshComponent, const pxr::UsdSkelRoot& Root, const pxr::UsdSkelSkeletonQuery& USDSkeletonQuery)
{
    pxr::UsdSkelAnimQuery AnimQuery = USDSkeletonQuery.GetAnimQuery();
    auto SkeletalMesh = SkeletalMeshComponent->SkeletalMesh;
    if (!SkeletalMesh || !AnimQuery.IsValid())
    {
        return;
    }
    
    std::vector<double> TimeSamples;
    if (!AnimQuery.GetJointTransformTimeSamples(&TimeSamples))
    {
        return;
    }

    double TimeCodesPerSecond = TimeCodeInfo.TimeCodesPerSecond.IsSet() ? TimeCodeInfo.TimeCodesPerSecond.GetValue() : DEFAULT_TIMECODESPERSECOND;
    double StartTimeCode = TimeCodeInfo.StartTimeCode.IsSet() ? TimeCodeInfo.StartTimeCode.GetValue() : DEFAULT_STARTTIMECODE;
    double EndTimeCode = TimeCodeInfo.EndTimeCode.IsSet() ? TimeCodeInfo.EndTimeCode.GetValue() : 0.0;

    FSHAHash Hash = FUSDHashGenerator::ComputeSHAHash(AnimQuery, StartTimeCode, EndTimeCode, TimeCodesPerSecond);
    FString Name = AnimQuery.GetPrim().GetName().GetText();
    if (FSimReadyReferenceCollector::Get().FindDependency(AnimQuery.GetPrim().GetPath().GetText()))
    {
        FString Text = FString::Printf(TEXT("Loading Animation %s..."), *Name);
        FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
    }
    FString HashString = Hash.ToString();
    Name = GetUniqueImportName(HashString, Name);
    UAnimSequence* Sequence = Cast<UAnimSequence>(FUSDGeometryCache::Find(HashString));
    if (Sequence == nullptr)
    {
        Sequence = LoadImportObject<UAnimSequence>(ImportType::Animation, Name);
    }
    // NOTE: AnimSequence need to be recreated if skeleton was changed
    if (!Sequence || !Sequence->GetSkeleton()->IsCompatible(SkeletalMesh->Skeleton))
    {
        // Force a new name for incompatible skeleton
        if (HasValidImportStage() && Sequence && !Sequence->GetSkeleton()->IsCompatible(SkeletalMesh->Skeleton))
        {
            Name = Name + TEXT("_") + FString::FromInt(ImportedSuffixIndex++);
        }

        FBoxSphereBounds Result = SkeletalMesh->GetBounds();
        FBox BoundingBox(ForceInit);

#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 26		
        USkeleton* Skeleton = SkeletalMesh->Skeleton;
#else
        USkeleton* Skeleton = SkeletalMesh->GetSkeleton();
#endif
        Sequence = FSimReadyUSDImporterHelper::CreateAnimSequence(USDSkeletonQuery, GetAssetPackage(ImportType::Animation, Name), GetAssetName(Name), GetAssetFlag(), Skeleton, SkeletalMesh, TimeCodesPerSecond, StartTimeCode, EndTimeCode, &BoundingBox);
        if (Sequence)
        {
            if (BoundingBox.IsValid)
            {
                Result = Result + FBoxSphereBounds(BoundingBox);
            }
            SkeletalMesh->SetImportedBounds(Result);
            if (HasValidImportStage())
            {
                FAssetRegistryModule::AssetCreated(Sequence);
            }
        }
    }

    if (Sequence)
    {
        FUSDGeometryCache::Add(HashString, Sequence);
    }

    if (Sequence && Sequence->IsValidToPlay())
    {
        // set to sequence to play the animation
        USDSequenceImporter->CreateSkeletalAnimationTrack(SkeletalMeshComponent->GetOwner(), Sequence, StartTimeCode);
        // set to component to initialize the preview skel pose in the level
        SkeletalMeshComponent->AnimationData.AnimToPlay = Sequence;
        SkeletalMeshComponent->PlayAnimation(Sequence, false);
        SkeletalMeshComponent->InitAnim(true);
    }
}

bool ASimReadyStageActor::LoadSkeletalMeshComponent(const pxr::SdfPath& Path)
{
    auto USDSkelRoot = pxr::UsdSkelRoot(GetUSDStage()->GetPrimAtPath(Path.GetAbsoluteRootOrPrimPath()));
    if (!USDSkelRoot)
    {
        return false;
    }
    
    pxr::UsdSkelCache USDSkelCache;
    USDSkelCache.Populate(USDSkelRoot);

    std::vector< pxr::UsdSkelBinding > USDSkeletonBindings;
    USDSkelCache.ComputeSkelBindings(USDSkelRoot, &USDSkeletonBindings);

    // Skel root could contain mesh without skeleton binding. UE import it as Static mesh
    if (USDSkeletonBindings.size() == 0)
    {
        TArray<pxr::UsdPrim> MeshPrims;
        FindAllPrims<pxr::UsdGeomMesh>(USDSkelRoot.GetPrim(), MeshPrims);

        for (auto MeshPrim : MeshPrims)
        {
            auto Prim = MeshPrim.GetParent();
            while (Prim && !Prim.IsPseudoRoot())
            {
                LoadSceneComponent(Prim.GetPath(), false);
                if (Prim.IsA<pxr::UsdSkelRoot>())
                {
                    break;
                }
                Prim = Prim.GetParent();
            }

            LoadStaticMeshComponent<UStaticMeshComponent, AStaticMeshActor>(MeshPrim.GetPath(), false);
        }
    }

    // Skeletal mesh not support multiple root skeletons, having to separate them into several skeletal meshes.
    for (auto SkeletonBinding : USDSkeletonBindings)
    {
        auto SkeletonMeshPrim = SkeletonBinding.GetSkeleton().GetPrim().GetParent();
        if (!SkeletonMeshPrim)
        {
            continue;
        }

        // Load the parents of skeletal mesh
        if (!SkeletonMeshPrim.IsA<pxr::UsdSkelRoot>())
        {
            auto Prim = SkeletonMeshPrim.GetParent();
            while (Prim && !Prim.IsPseudoRoot())
            {
                LoadSceneComponent(Prim.GetPath(), false);
                if (Prim.IsA<pxr::UsdSkelRoot>())
                {
                    break;
                }
                Prim = Prim.GetParent();
            }
        }

        USkeletalMeshComponent* SkeletalMeshComponent = CreateActorAndComponent<USkeletalMeshComponent, ASkeletalMeshActor>(SkeletonMeshPrim);
        if (!SkeletalMeshComponent)
        {
            continue;
        }

        bool bPrimvarsAttribute = false;
        if (Path.IsPrimPropertyPath())
        {
            if (Path.GetName().find("primvars:", 0) == 0)
            {
                bPrimvarsAttribute = true;
            }
        }

        const pxr::UsdSkelSkeleton& USDSkeleton = SkeletonBinding.GetSkeleton();
        pxr::UsdSkelSkeletonQuery SkelQuery = USDSkelCache.GetSkelQuery(USDSkeleton);

        if (ShouldLoadProperty(Path, pxr::UsdGeomMesh::GetSchemaAttributeNames(), pxr::UsdGeomBoundable::GetSchemaAttributeNames())
            || ShouldLoadProperty(Path, pxr::UsdSkelSkeleton::GetSchemaAttributeNames(false))
            || ShouldLoadProperty(Path, pxr::UsdSkelBindingAPI::GetSchemaAttributeNames(false))
            || bPrimvarsAttribute)
        {
            FSHAHash Hash = FUSDHashGenerator::ComputeSHAHash(USDSkelCache, SkeletonBinding);
            FString HashString = Hash.ToString();
            FString SkelMeshName = USDSkelRoot.GetPrim().GetName().GetText();
            FName Dependency = USDSkelRoot.GetPath().GetText();
            if (USDSkeletonBindings.size() > 1)
            {
                SkelMeshName = SkeletonMeshPrim.GetName().GetText();
                Dependency = SkeletonMeshPrim.GetPath().GetText();
            }

            if (FSimReadyReferenceCollector::Get().FindDependency(Dependency))
            {
                FString Text = FString::Printf(TEXT("Loading Skeletal Mesh %s..."), *SkelMeshName);
                FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
            }

            SkelMeshName = GetUniqueImportName(HashString, SkelMeshName);
            USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(FUSDGeometryCache::Find(HashString));
            if (SkeletalMesh == nullptr)
            {
                SkeletalMesh = LoadImportObject<USkeletalMesh>(ImportType::Mesh, SkelMeshName);
            }
            if (!SkeletalMesh)
            {
                FSkeletalMeshImportData SkeletalMeshImportData;
                if (!FUSDDerivedDataCache::Load(HashString, SkeletalMeshImportData))
                {
                    if (!FSimReadyUSDImporterHelper::USDImportSkeleton(SkelQuery, SkeletalMeshImportData))
                    {
                        continue;
                    }

                    for (const pxr::UsdSkelSkinningQuery& SkinningQuery : SkeletonBinding.GetSkinningTargets())
                    {
                        pxr::UsdGeomMesh SkinningMesh = pxr::UsdGeomMesh(SkinningQuery.GetPrim());
                        if (SkinningMesh)
                        {
                            FSimReadyUSDImporterHelper::USDImportSkinning(SkelQuery, SkinningQuery, SkeletalMeshImportData);
                        }
                    }

                    FUSDDerivedDataCache::Save(HashString, SkeletalMeshImportData);
                }

                if (SkeletalMeshComponent->SkeletalMesh)
                {
                    SkeletalMesh = SkeletalMeshComponent->SkeletalMesh;
                    FSimReadyAssetImportHelper::LoadSkeletalMeshFromImportData(SkeletalMesh, SkeletalMeshImportData, SkelMeshName); // Path can't be the skeletal mesh name because it may contain several skinning binding
                }
                else
                {
                    SkeletalMesh = FSimReadyAssetImportHelper::CreateSkeletalMeshFromImportData(GetAssetPackage(ImportType::Mesh, SkelMeshName), GetAssetName(SkelMeshName), GetAssetFlag(), SkeletalMeshImportData, SkelMeshName);
                }

                if (HasValidImportStage())
                {
                    FAssetRegistryModule::AssetCreated(SkeletalMesh);
                }
            }

            if (SkeletalMesh)
            {
                FUSDGeometryCache::Add(HashString, SkeletalMesh);
            }

            SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh);

            // Skeleton
            Hash = FUSDHashGenerator::ComputeSHAHash(USDSkeleton, false);
            HashString = Hash.ToString();
            FString SkeletonName = USDSkeleton.GetPrim().GetName().GetText();
            if (FSimReadyReferenceCollector::Get().FindDependency(USDSkeleton.GetPath().GetText()))
            {
                FString Text = FString::Printf(TEXT("Loading Skeleton %s..."), *SkeletonName);
                FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
            }
            SkeletonName = GetUniqueImportName(HashString, SkeletonName);
            USkeleton* Skeleton = Cast<USkeleton>(FUSDGeometryCache::Find(HashString));
            if (Skeleton == nullptr)
            {
                Skeleton = LoadImportObject<USkeleton>(ImportType::Mesh, SkeletonName);
            }
            if (!Skeleton || !Skeleton->IsCompatibleMesh(SkeletalMesh))
            {
                // Force a new name for incompatible mesh
                if (HasValidImportStage() && Skeleton && !Skeleton->IsCompatibleMesh(SkeletalMesh))
                {
                    SkeletonName = SkeletonName + TEXT("_") + FString::FromInt(ImportedSuffixIndex++);
                }

                Skeleton = NewObject<USkeleton>(GetAssetPackage(ImportType::Mesh, SkeletonName), GetAssetName(SkeletonName), GetAssetFlag());
                Skeleton->SetPreviewMesh(SkeletalMesh);
                Skeleton->MergeAllBonesToBoneTree(SkeletalMesh);
                if (HasValidImportStage())
                {
                    FAssetRegistryModule::AssetCreated(Skeleton);
                }
            }

            if (Skeleton)
            {
                FUSDGeometryCache::Add(HashString, Skeleton);
            }

            SkeletalMesh->Skeleton = Skeleton;

            // Set rest transform as animation
#if 0 // Rest Pose
            if (SkelQuery.HasRestPose())
            {
                Hash = FUSDHashGenerator::ComputeSHAHash(USDSkeleton, true);
                FString RestAnimName = SkeletonName + TEXT("_RestPose");
                RestAnimName = GetUniqueImportName(Hash.ToString(), RestAnimName);
                UAnimSequence* Sequence = Cast<UAnimSequence>(FUSDGeometryCache::Find(Hash.ToString()));
                if (Sequence == nullptr)
                {
                    Sequence = LoadImportObject<UAnimSequence>(ImportType::Animation, RestAnimName);
                }
                if (!Sequence || !Sequence->GetSkeleton()->IsCompatible(SkeletalMesh->Skeleton))
                {
                    // Force a new name for incompatible skeleton
                    if (HasValidImportStage() && Sequence && !Sequence->GetSkeleton()->IsCompatible(SkeletalMesh->Skeleton))
                    {
                        RestAnimName = RestAnimName + TEXT("_") + FString::FromInt(ImportedSuffixIndex++);
                    }

                    auto Prim = USDSkeleton.GetPrim();
                    pxr::UsdGeomXformCache Cache = pxr::UsdGeomXformCache();
                    bool resetsXformStack;
                    pxr::GfMatrix4d LocalMatrix = Cache.GetLocalTransformation(Prim, &resetsXformStack);
                    FTransform SkeletonToComponent = USDConvert(GetUSDStage(), LocalMatrix);

                    double TimeCodesPerSecond = TimeCodeInfo.TimeCodesPerSecond.IsSet() ? TimeCodeInfo.TimeCodesPerSecond.GetValue() : DEFAULT_TIMECODESPERSECOND;
                    Sequence = NewObject<UAnimSequence>(GetAssetPackage(ImportType::Animation, RestAnimName), GetAssetName(RestAnimName), GetAssetFlag());
                    Sequence->SetSkeleton(SkeletalMesh->Skeleton);
                    Sequence->ImportFileFramerate = TimeCodesPerSecond;
                    Sequence->ImportResampleFramerate = TimeCodesPerSecond;
                    double TotalTimeSample = 1.0;
                    Sequence->SequenceLength = TotalTimeSample / TimeCodesPerSecond;
                    Sequence->SetRawNumberOfFrame(TotalTimeSample);

                    TArray<FName> BoneNames;
                    SkeletalMeshComponent->GetBoneNames(BoneNames);

                    for (int32 BoneIndex = 0; BoneIndex < BoneNames.Num(); ++BoneIndex)
                    {
                        // add tracks for the bone existing
                        Sequence->AddNewRawTrack(BoneNames[BoneIndex]);
                    }

                    auto RestTransforms = GetUSDValue<pxr::VtArray<pxr::GfMatrix4d>>(USDSkeleton.GetRestTransformsAttr());
                    for (int32 JointIndex = 0; JointIndex < BoneNames.Num(); ++JointIndex)
                    {
                        FTransform LocalTransform = USDConvert(GetUSDStage(), RestTransforms[JointIndex]);
                        // Applying the transform of skeleton to root of the animation.
                        if (JointIndex == 0)
                        {
                            LocalTransform *= SkeletonToComponent;
                        }
                    
                        FRawAnimSequenceTrack& RawTrack = Sequence->GetRawAnimationTrack(JointIndex);
                        RawTrack.PosKeys.Add(LocalTransform.GetTranslation());
                        RawTrack.RotKeys.Add(LocalTransform.GetRotation());
                        RawTrack.ScaleKeys.Add(LocalTransform.GetScale3D());
                    }

                    Sequence->MarkRawDataAsModified();
                    Sequence->PostEditChange();
                    FUSDGeometryCache::Add(Hash.ToString(), Sequence);
                    if (HasValidImportStage())
                    {
                        FAssetRegistryModule::AssetCreated(Sequence);
                    }
                    Sequence->SetPreviewMesh(SkeletalMesh);
                }

                if (Sequence && Sequence->IsValidToPlay())
                {
                    // set to component to initialize the preview skel pose in the level
                    SkeletalMeshComponent->AnimationData.AnimToPlay = Sequence;
                    SkeletalMeshComponent->PlayAnimation(Sequence, false);
                    SkeletalMeshComponent->InitAnim(true);
                }
            }
#endif
        }

        if (ShouldLoadProperty(Path, { pxr::UsdShadeTokens->materialBinding }))
        {
            int32 MaterialIndex = 0;
            for (const pxr::UsdSkelSkinningQuery& SkinningQuery : SkeletonBinding.GetSkinningTargets())
            {
                pxr::UsdGeomMesh SkinningMesh = pxr::UsdGeomMesh(SkinningQuery.GetPrim());
                SkeletalMeshComponent->SetMaterial(MaterialIndex, nullptr);
                auto USDGeomSubsets = pxr::UsdGeomSubset::GetAllGeomSubsets(SkinningMesh);
                if (!USDGeomSubsets.empty())
                {
                    for (auto USDGeomSubset : USDGeomSubsets)
                    {
                        pxr::TfToken FamilyName;
                        if (USDGeomSubset.GetFamilyNameAttr().Get(&FamilyName) && FamilyName == pxr::UsdShadeTokens->materialBind)
                        {
                            auto MaterialBinding = FUSDConversion::ParsePrimMaterial(USDGeomSubset.GetPrim());
                            LoadMaterial(SkeletalMeshComponent, MaterialIndex, SkinningMesh, MaterialBinding.Shader, MaterialBinding.MdlSurfaceShader);
                        }
                        ++MaterialIndex;
                    }	
                }
                else
                {
                    auto MaterialBinding = FUSDConversion::ParsePrimMaterial(SkinningMesh.GetPrim());
                    LoadMaterial(SkeletalMeshComponent, MaterialIndex, SkinningMesh, MaterialBinding.Shader, MaterialBinding.MdlSurfaceShader);
                    ++MaterialIndex;
                }
            }
        }

        LoadAnimationSequence(SkeletalMeshComponent, USDSkelRoot, SkelQuery);

        LoadSceneComponent(SkeletonMeshPrim.GetPath(), false);
    }
    
    return true;
}

#if LIGHTMAPS_EXPORT_PRIMVARS
static bool CreateUniformAttribute(pxr::UsdPrim& Prim, const std::string& Name, const int Value, pxr::UsdTimeCode TimeCode)
{
    auto Attribute = Prim.CreateAttribute(pxr::TfToken(Name), pxr::SdfValueTypeNames->Int, pxr::SdfVariabilityUniform);
    return Attribute.Set<int>(Value, TimeCode);
}

static bool CreateUniformAttribute(pxr::UsdPrim& Prim, const std::string& Name, const FVector2D& Value, pxr::UsdTimeCode TimeCode)
{
    auto Attribute = Prim.CreateAttribute(pxr::TfToken(Name), pxr::SdfValueTypeNames->Float2, pxr::SdfVariabilityUniform);
    return Attribute.Set<pxr::GfVec2f>(USDConvert(Value), TimeCode);
}

static bool CreateUniformAttribute(pxr::UsdPrim& Prim, const std::string& Name, const FString& value, pxr::UsdTimeCode TimeCode)
{
    auto Attribute = Prim.CreateAttribute(pxr::TfToken(Name), pxr::SdfValueTypeNames->String);
    return Attribute.Set(std::string(TCHAR_TO_ANSI(*value)), TimeCode);
}
#endif

#if LIGHTMAPS_EXPORT_PRIMVARS || LIGHTMAPS_EXPORT_SCHEMA
static void SaveLightmappingInfo(const pxr::UsdStageRefPtr& Stage, pxr::UsdPrim MeshPrim, const UStaticMeshComponent& Component, pxr::UsdTimeCode StageTimeCode, TSet<UTexture2D*> * Textures = nullptr)
{
    if (!Component.HasLightmapTextureCoordinates())
    {
        // Lightmap local mapping is not available
        return;
    }

    if (!Component.LODData.Num())
    {
        // LOD information is not available
        return;
    }
    const FStaticMeshComponentLODInfo & ComponentLODInfo = Component.LODData[0];
    const FMeshMapBuildData * MeshMapBuildData = Component.GetMeshMapBuildData(ComponentLODInfo);

    if (!MeshMapBuildData)
    {
        // Mesh build data is not available
        return;
    }

    const UStaticMesh* StaticMesh = Component.GetStaticMesh();
    const int32& CoordIndex = StaticMesh->LightMapCoordinateIndex;

    // Lightmaps info
    FLightMap * LightMap = MeshMapBuildData->LightMap.GetReference();
    if (LightMap)
    {
        FLightMap2D * LightMap2D = LightMap->GetLightMap2D();
        if (LightMap2D)
        {
            const FVector2D& CoordBias = LightMap2D->GetCoordinateBias();
            const FVector2D& CoordScale = LightMap2D->GetCoordinateScale();
            UTexture2D* LightTextureHQ = LightMap2D->GetTexture(0);
            UTexture2D* LightTextureLQ = LightMap2D->GetTexture(1);

#if LIGHTMAPS_EXPORT_PRIMVARS
            // keep information on uniform primvars
            CreateUniformAttribute(MeshPrim, "LightmapCoordIndex", CoordIndex, StageTimeCode);
            CreateUniformAttribute(MeshPrim, "LightmapCoordBias", CoordBias, StageTimeCode);
            CreateUniformAttribute(MeshPrim, "LightmapCoordScale", CoordScale, StageTimeCode);
            CreateUniformAttribute(MeshPrim, "LightmapTextureHQ", FixupTextureName(LightTextureHQ), StageTimeCode);
            CreateUniformAttribute(MeshPrim, "LightmapTextureLQ", FixupTextureName(LightTextureLQ), StageTimeCode);
#endif

#if LIGHTMAPS_EXPORT_SCHEMA
            // keep information on schema prim
            auto PrimPath = MeshPrim.GetPath().AppendElementString("LightmapInfo");
            pxr::LightmapsSchemaLightmap LightmapPrim(Stage->DefinePrim(PrimPath, pxr::TfToken("Lightmap")));
            LightmapPrim.CreateCoordIndexAttr(pxr::VtValue(static_cast<int>(CoordIndex)));
            LightmapPrim.CreateCoordBiasAttr(pxr::VtValue(static_cast<pxr::GfVec2f>(USDConvert(CoordBias))));
            LightmapPrim.CreateCoordScaleAttr(pxr::VtValue(static_cast<pxr::GfVec2f>(USDConvert(CoordScale))));
            LightmapPrim.CreateTextureHQAttr(pxr::VtValue(static_cast<std::string>(TCHAR_TO_ANSI(*FixupTextureName(LightTextureHQ)))));
            LightmapPrim.CreateTextureLQAttr(pxr::VtValue(static_cast<std::string>(TCHAR_TO_ANSI(*FixupTextureName(LightTextureLQ)))));
#endif

            if (Textures)
            {
                Textures->Add(LightTextureHQ);
                Textures->Add(LightTextureLQ);
            }
        }
    }

    // Shadomap info
    FShadowMap * ShadowMap = MeshMapBuildData->ShadowMap.GetReference();
    if (ShadowMap)
    {
        FShadowMap2D * ShadowMap2D = ShadowMap->GetShadowMap2D();
        if (ShadowMap2D)
        {
            const FVector2D& CoordBias = ShadowMap2D->GetCoordinateBias();
            const FVector2D& CoordScale = ShadowMap2D->GetCoordinateScale();
            UTexture2D* ShadowTexture = ShadowMap2D->GetShadowMap2D()->GetTexture();

#if LIGHTMAPS_EXPORT_PRIMVARS
            // keep information on uniform primvars
            CreateUniformAttribute(MeshPrim, "ShadowmapCoordIndex", CoordIndex, StageTimeCode);
            CreateUniformAttribute(MeshPrim, "ShadowmapCoordBias", CoordBias, StageTimeCode);
            CreateUniformAttribute(MeshPrim, "ShadowmapCoordScale", CoordScale, StageTimeCode);
            CreateUniformAttribute(MeshPrim, "ShadowmapTexture", FixupTextureName(ShadowTexture), StageTimeCode);
#endif

#if LIGHTMAPS_EXPORT_SCHEMA
            // keep information on schema prim
            auto PrimPath = MeshPrim.GetPath().AppendElementString("ShadowmapInfo");
            pxr::LightmapsSchemaShadowmap ShadowmapPrim(Stage->DefinePrim(PrimPath, pxr::TfToken("Shadowmap")));
            ShadowmapPrim.CreateCoordIndexAttr(pxr::VtValue(static_cast<int>(CoordIndex)));
            ShadowmapPrim.CreateCoordBiasAttr(pxr::VtValue(static_cast<pxr::GfVec2f>(USDConvert(CoordBias))));
            ShadowmapPrim.CreateCoordScaleAttr(pxr::VtValue(static_cast<pxr::GfVec2f>(USDConvert(CoordScale))));
            ShadowmapPrim.CreateTextureAttr(pxr::VtValue(static_cast<std::string>(TCHAR_TO_ANSI(*FixupTextureName(ShadowTexture)))));
#endif

            if (Textures)
            {
                Textures->Add(ShadowTexture);
            }
        }
    }
}
#endif

bool ASimReadyStageActor::ToggleCineCameraVisibility(USceneComponent* Component)
{
    if (!Component || !Component->GetOwner())
    {
        return false;
    }

    auto Actor = Component->GetOwner();
    if (!Actor->IsA<ACineCameraActor>())
    {
        return false;
    }

    auto FoundPathKey = USDPathToObject.FindKey((UObject*)Actor->GetRootComponent());
    if (!FoundPathKey)
    {
        return false;
    }

    auto PrimPath = ToUSDPath(*FSimReadyPathHelper::KeyToPrimPath(*FoundPathKey));
    auto Prim = GetUSDStage()->GetPrimAtPath(PrimPath);
    if (!Prim)
    {
        return false;
    }

    pxr::UsdGeomCamera CameraPrim;
    if (Prim.IsA<pxr::UsdGeomCamera>())
    {
        CameraPrim = pxr::UsdGeomCamera(Prim);
    }
    else if(!Prim.GetAllChildren().empty())
    {
        auto ChildPrim = Prim.GetAllChildren().front();
        CameraPrim = pxr::UsdGeomCamera(ChildPrim);
    }

    bool bVisible = false;
    if (CameraPrim)
    {
        bVisible = true;
        auto CineComponent = Cast<UCameraComponent>(Actor->GetComponentByClass(UCameraComponent::StaticClass()));
        if (CineComponent)
        {
            // what's doing here?
            FNamedParameterTimeSamples NamedParameterTimeSamples;
            USDImportCamera(CameraPrim, *CineComponent, NamedParameterTimeSamples);
        }
    }

    TArray<USceneComponent*> Components;
    Actor->GetComponents(Components);
    for (auto ActorComponent : Components)
    {
        Cast<USceneComponent>(ActorComponent)->SetVisibility(bVisible);
    }
        
    return true;
}

bool ASimReadyStageActor::IsComponentSupported(USceneComponent& Component)
{
    bool bSupported = false;
    if (Component.IsA<UStaticMeshComponent>() || Component.IsA<UDirectionalLightComponent>() ||
        Component.IsA<UPointLightComponent>() || Component.IsA<URectLightComponent>() ||
        Component.IsA<UCameraComponent>() ||
        Component.IsA<USkeletalMeshComponent>())
    {
        bSupported = true;
    }

    // Actor to Xform
    if (Component.GetClass() == USceneComponent::StaticClass()
    && Component.GetOwner() && Component.GetOwner()->GetClass() == AActor::StaticClass()
    && Component.GetOwner()->GetRootComponent() == &Component)
    {
        bSupported = true;
    }

    if (!bSupported)
    {
        auto AttachChildren = Component.GetAttachChildren();
        for (int32 ChildIndex = 0; ChildIndex < AttachChildren.Num(); ChildIndex++)
        {
            bSupported = IsComponentSupported(*AttachChildren[ChildIndex]);
            if (bSupported)
                break;
        }
    }

    return bSupported;
}

static void RemoveActorFromList(TMap<AActor*, const AActor*>& List, AActor* Actor)
{
    List.Remove(Actor);

    // Can't get children by API GetAttachedActors, the children has been detached.
    // Search the list to get the children.
    while(true)
    {
        auto Child = List.FindKey(Actor);
        if (Child == nullptr)
        {
            break;
        }

        RemoveActorFromList(List, *Child);
    };
}

static void GetAllAttachedActors( AActor* Actor, TSet< AActor* >& OutActors )
{
    TArray< AActor* > ChildrenActors;
    Actor->GetAttachedActors( ChildrenActors );
    for ( AActor* ChildActor : ChildrenActors )
    {
        OutActors.Add( ChildActor );

        GetAllAttachedActors( ChildActor, OutActors );
    }
}

USceneComponent* ASimReadyStageActor::CreateActorAndComponent(UClass& SceneCompClass, UClass& ActorClass, const pxr::UsdPrim& Prim, bool bForceCreateActor)
{
    // Create actor if no parent or is model
    USceneComponent* ParentComp = FindObjectFromPath<USceneComponent>(Prim.GetPath().GetParentPath());

    auto IsModel = [](pxr::UsdPrim Prim)
    {
        pxr::TfToken Kind;
        pxr::UsdModelAPI(Prim).GetKind(&Kind);

        return pxr::KindRegistry::IsA(Kind, pxr::KindTokens->model);
    };

#if 0
    bool bCreateActor = false;
    if(!ParentComp || IsModel(Prim) || bForceCreateActor)
    {
        bCreateActor = true;
    }
    else
    {
        // Still create actor if all ascenders are not model.
        bool bModelParent = false;
        for(auto ParentPrim = Prim.GetParent(); !ParentPrim.IsPseudoRoot(); ParentPrim = ParentPrim.GetParent())
        {
            if(IsModel(ParentPrim))
            {
                bModelParent = true;
                break;
            }
        }

        if(!bModelParent)
        {
            bCreateActor = true;
        }
    }
#else
    bool bCreateActor = true;
#endif

    // Check whether existing object can be reused
    auto SceneComponent = FindObjectFromPath<USceneComponent>(Prim.GetPath());
    do
    {
        if(!SceneComponent)
        {
            break;
        }

        if(bCreateActor)
        {
            if(SceneComponent->GetOwner()->GetRootComponent() != SceneComponent)
            {
                break;
            }

            if(!SceneComponent->GetOwner()->IsA(&ActorClass))
            {
                break;
            }
        }

        if(SceneComponent->GetClass() != &SceneCompClass)
        {
            if(bCreateActor)
            {
                // check children
                for (auto Child : SceneComponent->GetAttachChildren())
                {
                    if (Child->GetClass() == &SceneCompClass)
                    {
                        return Child;
                    }
                }
            }

            break;
        }

        if (ParentComp && SceneComponent->GetAttachParent() != ParentComp)
        {
            SceneComponent->AttachToComponent(ParentComp, FAttachmentTransformRules::KeepRelativeTransform);
        }

        return SceneComponent;
    } while(false);

    // Reserve the object
    if(SceneComponent)
    {
        if(SceneComponent->GetOwner()->GetRootComponent() == SceneComponent)
        {
            ReserveObject(*SceneComponent->GetOwner(), Prim.GetPath().GetText());
        }
        else
        {
            ReserveObject(*SceneComponent, Prim.GetPath().GetText());
        }

        USDPathToObject.Remove(FSimReadyPathHelper::PrimPathToKey(Prim.GetPath().GetText()));

        SceneComponent = nullptr;
    }

    // Create new object
    if(bCreateActor)
    {
        // Spawn new actor if it's not reserved
        AActor* Actor = nullptr;

        UObject* ReservedObject = nullptr;
        ReservedUE4Objects.RemoveAndCopyValue(Prim.GetPath().GetText(), ReservedObject);
        if(ReservedObject && ReservedObject->GetClass() == &ActorClass)
        {
            Actor = Cast<AActor>(ReservedObject);
            // If root component class is different from the target component class, we still need to create a new actor
            if (Actor->GetRootComponent() && Actor->GetRootComponent()->GetClass() != &SceneCompClass)
            {
                Actor = GetWorld()->SpawnActor<AActor>(&ActorClass);
            }
            else
            {
                ReservedObject->ClearPendingKill();
                Actor->Rename(nullptr, GetLevel());
            }
        }
        else
        {
            Actor = GetWorld()->SpawnActor<AActor>(&ActorClass);
        }

        // Setup actor
        StaticCast<ASimReadyStageActor*>(Actor)->bEditable = ActorClass.GetDefaultObject<AActor>()->IsEditable();
        GetWorld()->GetCurrentLevel()->Actors.AddUnique(Actor);	// Rename() would automatically add to level

#if WITH_EDITOR
        Actor->SetActorLabel(Prim.GetName().GetText(), false);
#endif

        Actor->Tags.AddUnique(TEXT("SequencerActor"));	// Hack to show transient actors in world outliner

        auto FoundComponentClass = [](AActor* InActor, UClass& InComponentClass)
        {
            if (InActor->GetRootComponent())
            {
                auto Root = InActor->GetRootComponent();
                if (Root->GetClass() == &InComponentClass)
                {
                    return true;
                }
                else
                {
                    for( auto Child : Root->GetAttachChildren())
                    {
                        if (Child->GetClass() == &InComponentClass)
                        {
                            return true;
                        }
                    }
                }
            }

            return false;
        };

        // root is the target component
        if(FoundComponentClass(Actor, SceneCompClass))
        {
            SceneComponent = Actor->GetRootComponent();
            SceneComponent->ClearPendingKill();
            if(SceneComponent->CreationMethod == EComponentCreationMethod::Instance)
            {
                SceneComponent->Rename(ANSI_TO_TCHAR(Prim.GetName().GetText()));
            }
        }
        else
        {
            // Create component
            SceneComponent = NewObject<USceneComponent>(Actor, &SceneCompClass, Prim.GetName().GetText(), DefaultObjFlag);
            Actor->SetRootComponent(SceneComponent);
            Actor->AddInstanceComponent(SceneComponent);
        }

        GEngine->BroadcastLevelActorAdded(Actor);
    }
    else
    {
        auto ExistingObject = FindObject<UObject>(ParentComp, ANSI_TO_TCHAR(Prim.GetName().GetText()));
        if(ExistingObject)
        {
            if(ExistingObject->GetClass() == &SceneCompClass)
            {
                ExistingObject->ClearPendingKill();
                SceneComponent = Cast<USceneComponent>(ExistingObject);
            }
            else
            {
                ExistingObject->MarkPendingKill();
            }
        }

        if(!SceneComponent)
        {
            UObject* ReservedObject = nullptr;
            ReservedUE4Objects.RemoveAndCopyValue(Prim.GetPath().GetText(), ReservedObject);
            if(auto Actor = Cast<AActor>(ReservedObject))
            {
                ReservedObject = Actor->GetRootComponent();
            }

            if(ReservedObject && ReservedObject->GetClass() == &SceneCompClass)
            {
                ReservedObject->ClearPendingKill();
                SceneComponent = Cast<USceneComponent>(ReservedObject);
                SceneComponent->Rename(ANSI_TO_TCHAR(Prim.GetName().GetText()), ParentComp);
            }
            else
            {
                SceneComponent = NewObject<USceneComponent>(ParentComp, &SceneCompClass, Prim.GetName().GetText(), DefaultObjFlag);
            }
        }

        SceneComponent->GetOwner()->AddInstanceComponent(SceneComponent);

        // UI is not correct if two components have the same name. So we need to rename it if necessary.
        FName NewName = SceneComponent->GetFName();
        bool bSameNameFound = false;

        for(auto Component : SceneComponent->GetOwner()->GetComponents())
        {
            if(Component != SceneComponent && Component->GetFName() == NewName && Component->GetClass() != SceneComponent->GetClass())
            {
                NewName.SetNumber(NewName.GetNumber() + 1);
                bSameNameFound = true;
                break;
            }
        }

        if(bSameNameFound && NewName.GetNumber())
        {
            SceneComponent->Rename(*(NewName.GetPlainNameString() + "(" + FString::FromInt(NewName.GetNumber()) + ")"));
        }
    }

    SceneComponent->SetMobility(HasValidImportStage() ? EComponentMobility::Static : EComponentMobility::Movable);	// Static can't be attached under movable

    if(ParentComp)
    {
        SceneComponent->AttachToComponent(ParentComp, FAttachmentTransformRules::KeepRelativeTransform);
    }

    USDPathToObject.Add(FSimReadyPathHelper::PrimPathToKey(Prim.GetPath().GetText()), (UObject*)SceneComponent);

    if (bCreateActor)
    {
        if (SceneComponent->GetClass() != &SceneCompClass)
        {
            for (auto Child : SceneComponent->GetAttachChildren())
            {
                if (Child->GetClass() == &SceneCompClass)
                {
                    SceneComponent = Child;
                    break;
                }
            }
        }
    }

    return SceneComponent;
}

void ASimReadyStageActor::ReserveObject(UObject& Object, const FString& USDPath)
{
    if(auto Actor = Cast<AActor>(&Object))
    {
        {
            // When this is called during transaction, objects are loaded from transaction buffer, objects would have strange state, which causes assertion failure.
            for(auto Component : Actor->GetComponents())
            {
                if(auto SceneComp = Cast<USceneComponent>(Component))
                {
                    if(SceneComp->GetOwner() != Actor)
                    {
                        SceneComp->Rename(nullptr, SceneComp->GetAttachParent());
                    }
                }
            }
        }

        Actor->Rename(nullptr, ReservedUE4ObjectOuter);	// Must rename before unregistoring. Otherwise meshes will be still rendered and left in the scene
        checkSlow(Actor->GetLevel());
        if(Actor->GetLevel())
        {
            GetWorld()->RemoveActor(Actor, false);	// Rename() would add actor to level.
            Actor->UnregisterAllComponents();
            Actor->RegisterAllActorTickFunctions(false, true);
        }

        ReservedUE4Objects.Add(USDPath, Actor);
    }
    else if(auto Component = Cast<UActorComponent>(&Object))
    {
        if(Component->IsRegistered())
        {
            Component->UnregisterComponent();
        }
        Component->GetOwner()->RemoveOwnedComponent(Component);
        Component->RegisterAllComponentTickFunctions(false);
        Component->Rename(nullptr, ReservedUE4ObjectOuter);

        ReservedUE4Objects.Add(USDPath, Component);
    }
    else if (auto Material = Cast<UMaterialInterface>(&Object))
    {
        if (!IsValidPrimPath(ToUSDPath(*USDPath)))
        {
            for (UActorComponent* ActorComponent : TObjectRange<UActorComponent>())
            {
                if (ActorComponent->IsA<UMeshComponent>())
                {
                    auto MeshComponent = Cast<UMeshComponent>(ActorComponent);

                    for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
                    {
                        if (MeshComponent->GetMaterial(MaterialIndex) == Material)
                        {
                            MeshComponent->SetMaterial(MaterialIndex, nullptr);
                        }
                    }
                }
            }

            Material->MarkPendingKill();
        }
    }
}

bool ASimReadyStageActor::ShouldLoadProperty(const pxr::SdfPath& Path, const pxr::TfTokenVector& PropNames, const pxr::TfTokenVector& ExcludedPropNames)
{
    if(!Path.IsPropertyPath())
    {
        return true;
    }

    return std::find(PropNames.begin(), PropNames.end(), Path.GetNameToken()) != PropNames.end() && std::find(ExcludedPropNames.begin(), ExcludedPropNames.end(), Path.GetNameToken()) == ExcludedPropNames.end();
}

void ASimReadyStageActor::LoadMaterial(UMeshComponent* Component, int32 SlotIndex, const pxr::UsdGeomMesh& USDMesh, const pxr::UsdShadeShader& Shader, const pxr::UsdShadeShader& MdlShader)
{
    if (GetDefault<USimReadySettings>()->bDisableMaterialLoading)
    {
        return;
    }

    if (Shader || MdlShader)
    {
        auto LoadMaterialByShaderID = [&](const pxr::UsdShadeShader& ShadeShader)
        {
            if (!ShadeShader)
            {
                return false;
            }

            const auto ShaderID = GetUSDValue<pxr::TfToken>(ShadeShader.GetIdAttr());
            if (ShaderID == USDTokens.previewSurface && LoadPreviewSurface(Component, SlotIndex, USDMesh, ShadeShader))
            {
                return true;
            }

            return false;
        };

        auto USDMaterial = Shader ? FindShadeMaterial(Shader.GetPrim()) : FindShadeMaterial(MdlShader.GetPrim());
        if (USDMaterial && FSimReadyReferenceCollector::Get().FindDependency(USDMaterial.GetPath().GetText()))
        {
            FString Name = USDMaterial.GetPrim().GetName().GetText();
            FString Text = FString::Printf(TEXT("Loading Material %s..."), *Name);
            FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
        }

        const bool bRenderMDLPriority = GetDefault<USimReadySettings>()->RenderContext == ERenderContext::ERC_MDL;
        if (bRenderMDLPriority)
        {
            // try mdl surface output at first
            if (!LoadMdlSchema(Component, SlotIndex, MdlShader))
            {
                // try surface output with mdl schema
                if (!LoadMdlSchema(Component, SlotIndex, Shader))
                {
                    if (!LoadMaterialByShaderID(MdlShader))
                    {
                        LoadMaterialByShaderID(Shader);
                    }
                }
            }
        }
        else
        {
            if (!LoadMaterialByShaderID(Shader))
            {
                if (!LoadMaterialByShaderID(MdlShader))
                {
                    if (!LoadMdlSchema(Component, SlotIndex, Shader))
                    {
                        // try mdl surface output at last
                        LoadMdlSchema(Component, SlotIndex, MdlShader);
                    }
                }
            }
        }
    }
}

void ASimReadyStageActor::LoadMaterial(UStaticMeshComponent& Component, const pxr::UsdGeomGprim& USDGprim, pxr::UsdGeomSubset InGeomSubset)
{
    // Collect material bindings
    TArray<FUSDConversion::FMaterialBinding> MaterialBindings;

    pxr::UsdGeomMesh USDMesh(USDGprim);

    auto USDGeomSubsets = pxr::UsdGeomSubset::GetAllGeomSubsets(USDMesh);
    if(!USDGeomSubsets.empty())
    {
        bool bSkipGeomSubset = false;
        if (USDGeomSubsets.size() == 1)
        {
            // Check if USD mesh bound material
            pxr::UsdShadeMaterialBindingAPI USDMaterialBinding(USDMesh);
            auto Material = pxr::UsdShadeMaterial(USDMaterialBinding.ComputeBoundMaterial());
            if (Material)
            {
                MaterialBindings.Add(FUSDConversion::ParsePrimMaterial(USDMesh.GetPrim()));
                bSkipGeomSubset = true;
            }
        }
        
        if (!bSkipGeomSubset)
        {
            for (auto GeomSubset : USDGeomSubsets)
            {
                pxr::TfToken FamilyName;
                if (!GeomSubset.GetFamilyNameAttr().Get(&FamilyName) || FamilyName != pxr::UsdShadeTokens->materialBind)
                {
                    // Add warning instead of not loading material
                    UE_LOG(LogSimReadyUsd, Warning, TEXT("%s: Material bindings authored on GeomSubsets are honored by renderers only if their familyName is UsdShadeTokens->materialBind"), *FString(GeomSubset.GetPath().GetText()));
                }

                MaterialBindings.Add(FUSDConversion::ParsePrimMaterial(GeomSubset.GetPrim()));
            }
        }
    }
    else
    {
        MaterialBindings.Add(FUSDConversion::ParsePrimMaterial(USDMesh.GetPrim()));
    }

    // Add material slots
    if(!Component.GetStaticMesh())
    {
        return;
    }

    if(Component.GetStaticMesh()->StaticMaterials.Num() < MaterialBindings.Num())
    {
        Component.GetStaticMesh()->StaticMaterials.SetNum(MaterialBindings.Num());
        Component.GetStaticMesh()->UpdateUVChannelData(false);
    }

    // Reset material to default value first, so after layer is muted,
    // The reference material will be calculated again or it's removed.
    if(!InGeomSubset)
    {
        Component.EmptyOverrideMaterials();
    }

    // Load each material binding
    for(auto SlotIndex = 0; SlotIndex < MaterialBindings.Num(); ++SlotIndex)
    {
        // Skip
        if(InGeomSubset && InGeomSubset.GetPrim() != USDGeomSubsets[SlotIndex].GetPrim())
        {
            continue;
        }

        // Clear current material
        Component.SetMaterial(SlotIndex, nullptr);

        //Get shader
        auto& MaterialBinding = MaterialBindings[SlotIndex];
        LoadMaterial(&Component, SlotIndex, USDMesh, MaterialBinding.Shader, MaterialBinding.MdlSurfaceShader);
    }
}

UStaticMeshComponent* ASimReadyStageActor::LoadBasicShape(const pxr::SdfPath& Path, bool bLoadChildren)
{
    pxr::UsdGeomGprim USDGprim = pxr::UsdGeomGprim::Get(GetUSDStage(), Path.GetPrimPath());
    if(!USDGprim)
    {
        return nullptr;
    }

    FString MeshPath;
    TFunction<void(UStaticMeshComponent&)> PostCreateComponent;

    auto ShouldLoadTransform = [&](const pxr::TfTokenVector& PropNames)
    {
        if(!ShouldLoadProperty(Path, PropNames) 
            && Path.IsPropertyPath() && !pxr::UsdGeomXformOp::IsXformOp(Path.GetNameToken())
            )
        {
            return false;
        }

        LoadSceneComponent(Path.GetPrimPath().AppendProperty(pxr::UsdGeomXformOp::GetOpName(pxr::UsdGeomXformOp::TypeScale)), bLoadChildren);

        return true;
    };

    auto SetRotateAndScaleWithAxis = [USDGprim](const pxr::TfToken& MeshAxis, USceneComponent& SceneComp)
    {
        FVector Scale = SceneComp.GetRelativeScale3D();
        auto UpAxis = pxr::UsdGeomGetStageUpAxis(USDGprim.GetPrim().GetStage());
        if(MeshAxis == pxr::UsdGeomTokens->x)
        {
            SceneComp.SetRelativeRotation(SceneComp.GetRelativeTransform().GetRotation() * FQuat(FVector(0, 1, 0), HALF_PI));
            // swap x and z
            Swap(Scale.X, Scale.Z);
        }
        else if(MeshAxis == pxr::UsdGeomTokens->y && UpAxis == pxr::UsdGeomTokens->z)
        {
            SceneComp.SetRelativeRotation(SceneComp.GetRelativeTransform().GetRotation() * FQuat(FVector(1, 0, 0), HALF_PI));
            // swap y and z
            Swap(Scale.Y, Scale.Z);
        }
        else if (MeshAxis == pxr::UsdGeomTokens->z && UpAxis == pxr::UsdGeomTokens->y)
        {
            SceneComp.SetRelativeRotation(SceneComp.GetRelativeTransform().GetRotation() * FQuat(FVector(1, 0, 0), -HALF_PI));
            // swap y and z
            Swap(Scale.Y, Scale.Z);
        }

        SceneComp.SetRelativeScale3D(Scale);
    };

    float UnitScale = UnitScaleFromUSDToUE(GetUSDStage()) * 0.01f;

    if(auto USDSphere = pxr::UsdGeomSphere(USDGprim))
    {
        MeshPath = UActorFactoryBasicShape::BasicSphere.ToString();

        PostCreateComponent = [=](UStaticMeshComponent& MeshComp)
        {
            if(ShouldLoadTransform({pxr::UsdGeomTokens->radius}))
            {
                float Radius = GetUSDValue<double>(USDSphere.GetRadiusAttr());
                MeshComp.SetRelativeScale3D(MeshComp.GetRelativeScale3D() * FVector(Radius * 2.0f * UnitScale));
            }
        };
    }
    else if(auto USDCube = pxr::UsdGeomCube(USDGprim))
    {
        MeshPath = UActorFactoryBasicShape::BasicCube.ToString();

        PostCreateComponent = [=](UStaticMeshComponent& MeshComp)
        {
            if(ShouldLoadTransform({pxr::UsdGeomTokens->size}))
            {
                float Size = GetUSDValue<double>(USDCube.GetSizeAttr());
                MeshComp.SetRelativeScale3D(MeshComp.GetRelativeScale3D() * FVector(Size * UnitScale));
            }
        };
    }
    else if(auto USDCone = pxr::UsdGeomCone(USDGprim))
    {
        MeshPath = UActorFactoryBasicShape::BasicCone.ToString();

        PostCreateComponent = [=](UStaticMeshComponent& MeshComp)
        {
            if(ShouldLoadTransform({pxr::UsdGeomTokens->radius, pxr::UsdGeomTokens->height, pxr::UsdGeomTokens->axis}))
            {
                float Radius = GetUSDValue<double>(USDCone.GetRadiusAttr());
                float Height = GetUSDValue<double>(USDCone.GetHeightAttr());
                SetRotateAndScaleWithAxis(GetUSDValue<pxr::TfToken>(USDCone.GetAxisAttr()), MeshComp);
                MeshComp.SetRelativeScale3D(MeshComp.GetRelativeScale3D() * FVector(Radius * 2, Radius * 2, Height) * UnitScale);
            }
        };
    }
    else if(auto USDCylinder = pxr::UsdGeomCylinder(USDGprim))
    {
        MeshPath = UActorFactoryBasicShape::BasicCylinder.ToString();

        PostCreateComponent = [=](UStaticMeshComponent& MeshComp)
        {
            if(ShouldLoadTransform({pxr::UsdGeomTokens->radius, pxr::UsdGeomTokens->height, pxr::UsdGeomTokens->axis}))
            {
                float Radius = GetUSDValue<double>(USDCylinder.GetRadiusAttr());
                float Height = GetUSDValue<double>(USDCylinder.GetHeightAttr());
                SetRotateAndScaleWithAxis(GetUSDValue<pxr::TfToken>(USDCylinder.GetAxisAttr()), MeshComp);
                MeshComp.SetRelativeScale3D(MeshComp.GetRelativeScale3D() * FVector(Radius * 2, Radius * 2, Height) * UnitScale);
            }
        };
    }
    else if (auto USDCapsule = pxr::UsdGeomCapsule(USDGprim))
    {
        MeshPath = USimReadyMDL::GetContentPath(TEXT("/SimReady/Capsule.Capsule"));

        PostCreateComponent = [=](UStaticMeshComponent& MeshComp)
        {
            if (ShouldLoadTransform({ pxr::UsdGeomTokens->radius, pxr::UsdGeomTokens->height, pxr::UsdGeomTokens->axis }))
            {
                float Radius = GetUSDValue<double>(USDCapsule.GetRadiusAttr());
                float Height = GetUSDValue<double>(USDCapsule.GetHeightAttr());
                SetRotateAndScaleWithAxis(GetUSDValue<pxr::TfToken>(USDCapsule.GetAxisAttr()), MeshComp);
                MeshComp.SetRelativeScale3D(MeshComp.GetRelativeScale3D() * FVector(Radius * 4, Radius * 4, Height * 2) * UnitScale);
            }
        };
    }
    else
    {
        // Not implemented BasicShape
        return nullptr;
    }

    auto MeshComp = CreateActorAndComponent<UStaticMeshComponent, AStaticMeshActor>(USDGprim.GetPrim());
    if(!MeshComp)
    {
        return nullptr;
    }

    if(Path.IsPrimPath())
    {
        if(MeshPath.IsEmpty())
        {
            return nullptr;
        }

        // Don't duplicate the mesh, we want it to actually point to the engine cube
        // If we want to duplicate the mesh into the asset folder we can use something like this,
        // or maybe just DuplicateObject<>() could work?:
        // 
        // UObject* ObjectToCopy = LoadObject<UObject>(nullptr, *MeshPath);
        // TArray<UObject*> Assets;
        // Assets.Add(ObjectToCopy);
        // ObjectTools::DuplicateObjects(Assets, TEXT(""), FPaths::Combine(SavePackagePath, TEXT("Meshes")), /*bOpenDialog=*/false);
        // MeshPath = FPaths::Combine(SavePackagePath, TEXT("Meshes"), "Cube.Cube");		
        //
        // MeshComp->SetStaticMesh(DuplicateObject<UStaticMesh>(StaticMesh, nullptr));
        auto StaticMesh = LoadObject<UStaticMesh>(NULL, *MeshPath);
        MeshComp->SetStaticMesh(StaticMesh);
    }

    LoadStaticMeshFromGeomGprim(Path, *MeshComp, bLoadChildren);

    if(PostCreateComponent)
    {
        PostCreateComponent(*MeshComp);

        // if no usd material was set, setting Omniverse Default material instead of UE4 default material
        if (MeshComp->GetNumOverrideMaterials() == 0 || MeshComp->OverrideMaterials[0] == nullptr)
        {
            if (UMaterial* DefaultMaterial = LoadObject<UMaterial>(nullptr, *SimReadyDefaultMaterial))
            {
                MeshComp->SetMaterial(0, DefaultMaterial);
            }
        }
    }

    return MeshComp;
}

USceneComponent * ASimReadyStageActor::LoadSceneComponent(const pxr::SdfPath& Path, bool bLoadChildren, bool bUpdateBound, bool bIgnoreVisibility)
{
    bool ShouldLoadChildren = bLoadChildren && Path.IsAbsoluteRootOrPrimPath();
    USceneComponent* SceneComponent = FindObjectFromPath<USceneComponent>(Path.GetPrimPath());
    auto USDPrim = GetUSDStage()->GetPrimAtPath(Path.GetPrimPath());
    if (!USDPrim)
    {
        return nullptr;
    }

    if(!SceneComponent)
    {
        SceneComponent = CreateActorAndComponent<USceneComponent, AActor>(USDPrim);
    }
    else
    {
        USceneComponent* ParentComp = FindObjectFromPath<USceneComponent>(Path.GetPrimPath().GetParentPath());
        if (ParentComp && SceneComponent->GetAttachParent() != ParentComp)
        {
            SceneComponent->AttachToComponent(ParentComp, FAttachmentTransformRules::KeepRelativeTransform);
        }
    }

    if(!SceneComponent)
    {
        return SceneComponent;
    }

    // Lock the actor if it's a descendent of an instance prim or scope
    // NOTE: UE4 has nothing similar to USD Scope, we used a actor which locks location to represent
    if (SceneComponent->GetOwner())
    {
        SceneComponent->GetOwner()->bLockLocation = USDPrim.IsInstanceProxy() || USDPrim.IsA<pxr::UsdGeomScope>();
    }

    if (bUpdateBound)
    {
        StageBounds += SceneComponent->Bounds.GetBox();
    }

    // Setup new create component
    if(Path.IsPrimPath())
    {
        // Set mobility
        SceneComponent->SetMobility(HasValidImportStage() ? EComponentMobility::Static : EComponentMobility::Movable);

        // Register component
        if(!SceneComponent->IsRegistered())
        {
            // bEnableTraceCollision was changed from boolean to uint8:1, can't use swap anymore 
            bool bBackupValue = GetWorld()->bEnableTraceCollision;
            GetWorld()->bEnableTraceCollision = false; // Disable PhysX setup to boost mesh importing

            SceneComponent->RegisterComponent();

            GetWorld()->bEnableTraceCollision = bBackupValue;
        }
    }

    // Transform
    auto USDXform = pxr::UsdGeomXformable::Get(GetUSDStage(), Path.GetPrimPath());
    if(USDXform)
    {
        auto FixPrimaryAxis = [](const pxr::UsdPrim& Prim, USceneComponent* Component, FTransform& Transform)
        {
            bool bFixCamera = false;
            // Do not fix the axis on the camera component, it's easy to have the gimbal lock when pilot the camera
            if (Component->GetOwner() && (Component->GetOwner()->IsA<ACineCameraActor>() || Component->GetOwner()->IsA<ACameraActor>()))
            {
                auto RootComponent = Component->GetOwner()->GetRootComponent();
                bFixCamera = RootComponent == Component;
            }

            if (bFixCamera || Component->IsA<ULightComponent>())
            {
                // convert from usd camera (-z and y-up) to UE4 camera (+x and z-up)
                // convert from usd light (-z) to UE4 light (+x)
                // convert from Width of USD in the local X axis to Width of UE4, in the local Y axis.
                if (GetUSDStageAxis(Prim.GetStage()) == pxr::UsdGeomTokens->z)
                {
                    Transform.SetRotation(Transform.GetRotation() * FQuat(FVector(0, 0, 1), -HALF_PI) * FQuat(FVector(0, 1, 0), HALF_PI));
                }
                else
                {
                    Transform.SetRotation(Transform.GetRotation() * FQuat(FVector(0, 0, 1), -HALF_PI));
                }
            }
            else if (Component->IsA<UMeshComponent>())
            {
                if (GetUSDStageAxis(Prim.GetStage()) == pxr::UsdGeomTokens->y)
                {
                    auto MeshComponent = Cast<UMeshComponent>(Component);
                    if (IsSphericalOrCylindricalProjectionUsed(MeshComponent))
                    {
                        Transform.SetRotation(Transform.GetRotation() * FQuat(FVector(1, 0, 0), -HALF_PI));
                    }
                }
            }

            if (Prim.IsA<pxr::UsdLuxDomeLight>())
            {
                // NOTE: location and scale do nothing on Dome light, we should reset them for UE4 sky sphere
                Transform.SetLocation(FVector::ZeroVector);
                Transform.SetScale3D(FVector::OneVector);

                // Rotate for Y-up dome light
                if (GetUSDStageAxis(Prim.GetStage()) == pxr::UsdGeomTokens->y)
                {
                    Transform.SetRotation(Transform.GetRotation() * FQuat(FVector(1, 0, 0), -HALF_PI));
                }
            }
        };

        auto GetPrimTransform = [&](USceneComponent* Component, FTransform& Transform, const pxr::UsdTimeCode& TimeCode)
        {
            auto UsdPrim = USDXform.GetPrim();
            Transform = ConvertRelativeTransformFromUSDToUE4(UsdPrim, TimeCode);
            FixPrimaryAxis(UsdPrim, Component, Transform);

            if (!ShouldLoadChildren)
            {
                auto ChildPrims = USDPrim.GetFilteredChildren(pxr::UsdTraverseInstanceProxies());
                for (auto ChildPrim : ChildPrims)
                {
                    auto UsdPrimPath = ChildPrim.GetPrimPath();
                    USceneComponent* ChildComponent = FindObjectFromPath<USceneComponent>(UsdPrimPath);
                    if (ChildComponent)
                    {
                        auto ChildTransform = ConvertRelativeTransformFromUSDToUE4(ChildPrim, TimeCode);
                        FixPrimaryAxis(ChildPrim, ChildComponent, ChildTransform);
                        ChildComponent->SetRelativeTransform(ChildTransform);
                    }
                }
            }
        };

        if (!Path.IsPropertyPath() || pxr::UsdGeomXformOp::IsXformOp(Path.GetNameToken()))
        {
            // Set transform
            std::vector<double> TimeSamples;
            USDXform.GetTimeSamples(&TimeSamples);

            if (TimeSamples.size() > 0)
            {
                FTransformTimeSamples TransformSamples;
                TransformSamples.Transform.Reserve(TimeSamples.size());

                for (double TimeCode = TimeSamples.front(); TimeCode <= TimeSamples.back(); TimeCode += 1.0)
                {
                    FTransform Transform;
                    GetPrimTransform(SceneComponent, Transform, pxr::UsdTimeCode(TimeCode));
                    TransformSamples.Transform.Add(TimeCode, Transform);
                }

                SceneComponent->SetRelativeTransform(TransformSamples.Transform[TimeSamples[0]]);
                USDSequenceImporter->CreateTransformTrack(SceneComponent == SceneComponent->GetOwner()->GetRootComponent() ? Cast<UObject>(SceneComponent->GetOwner()) : Cast<UObject>(SceneComponent), TransformSamples);
            }
            else
            {
                FTransform Transform;
                GetPrimTransform(SceneComponent, Transform, StageTimeCode);
                SceneComponent->SetRelativeTransform(Transform);
            }
        }
    }

    // Visibility
    pxr::UsdGeomImageable USDImageable(USDPrim);
    if(!bIgnoreVisibility && USDImageable && ShouldLoadProperty(Path, {pxr::UsdGeomTokens->visibility}))
    {
        auto IsInherited = [](const pxr::UsdPrim& Prim)
        {
            pxr::UsdGeomImageable USDImageable(Prim);
            if (USDImageable)
            {
                if (USDImageable.ComputePurpose() == pxr::UsdGeomTokens->guide)
                {
                    return false;
                }

                std::vector<double> TimeSamples;
                USDImageable.GetVisibilityAttr().GetTimeSamples(&TimeSamples);

                if (TimeSamples.size() > 0) // ignore the prim with timesamples
                {
                    return false;
                }

                pxr::TfToken VisibilityValue;
                USDImageable.GetVisibilityAttr().Get(&VisibilityValue);

                return VisibilityValue != pxr::UsdGeomTokens->invisible; // ignore the prim which is invisible
            }

            return false;
        };

        TFunction<void(USceneComponent&, const pxr::UsdPrim&, const FBooleanTimeSamples&)> CreateVisibilityTrack;
        CreateVisibilityTrack = [this, &CreateVisibilityTrack, &IsInherited](USceneComponent& Comp, const pxr::UsdPrim& USDPrim, const FBooleanTimeSamples& VisTimeSamples)
        {
            USDSequenceImporter->CreateVisibilityTrack(&Comp == Comp.GetOwner()->GetRootComponent() ? Cast<UObject>(Comp.GetOwner()) : Cast<UObject>(&Comp), VisTimeSamples);

            for (auto ChildPrim : USDPrim.GetChildren())
            {
                if (!IsInherited(ChildPrim))
                {
                    continue;
                }

                USceneComponent* ChildComp = FindObjectFromPath<USceneComponent>(ChildPrim.GetPath());
                if (!ChildComp)
                {
                    continue;
                }

                CreateVisibilityTrack(*ChildComp, ChildPrim, VisTimeSamples);
            }
        };

        auto GetVisTimeSamples = [](const pxr::UsdGeomImageable& Imageable, FBooleanTimeSamples& VisibilityTimeSamples)
        {
            std::vector<double> TimeSamples;
            Imageable.GetVisibilityAttr().GetTimeSamples(&TimeSamples);
            if (TimeSamples.size() > 0)
            {
                VisibilityTimeSamples.TimeSamples.Reserve(TimeSamples.size());

                for (auto TimeCode : TimeSamples)
                {
                    pxr::TfToken Value;
                    Imageable.GetVisibilityAttr().Get(&Value, pxr::UsdTimeCode(TimeCode));
                    VisibilityTimeSamples.TimeSamples.Add(TimeCode, Value == pxr::TfToken("invisible") ? false : true);
                }

                return true;
            }

            return false;
        };

        FBooleanTimeSamples VisibilityTimeSamples;
        if (GetVisTimeSamples(USDImageable, VisibilityTimeSamples))
        {
            CreateVisibilityTrack(*SceneComponent, USDPrim, VisibilityTimeSamples);
        }
        else
        {
            TFunction<void(USceneComponent&, const pxr::UsdPrim&, const bool bVisible)> SetVisibility;
            SetVisibility = [this, &SetVisibility, &IsInherited](USceneComponent& SceneComp, const pxr::UsdPrim& USDPrim, const bool bVisible)
            {
                if (SceneComp.ShouldRender() != bVisible)
                {
                    SceneComp.SetVisibility(bVisible);
                    SceneComp.SetHiddenInGame(false);

                    if (SceneComp.GetOwner()->GetRootComponent() == &SceneComp)
                    {
                        SceneComp.GetOwner()->SetActorHiddenInGame(false);
                        SceneComp.GetOwner()->SetIsTemporarilyHiddenInEditor(!bVisible);
                    }
                }

                for (auto ChildPrim : USDPrim.GetChildren())
                {
                    if (!IsInherited(ChildPrim))
                    {
                        continue;
                    }
                    
                    USceneComponent* ChildComp = FindObjectFromPath<USceneComponent>(ChildPrim.GetPath());
                    if (!ChildComp)
                    {
                        continue;
                    }

                    SetVisibility(*ChildComp, ChildPrim, bVisible);
                }
            };

            
            if (!IsInherited(USDPrim))
            {
                SetVisibility(*SceneComponent, USDPrim, false);
            }
            else
            {
                if (USDPrim.GetParent())
                {
                    // check if parent has sequence
                    pxr::UsdPrim PrimWithSamples;
                    auto Prim = USDPrim.GetParent();
                    while (Prim)
                    {
                        pxr::UsdGeomImageable Imageable(Prim);
                        if (!Imageable)
                        {
                            break;
                        }

                        if (Imageable.ComputePurpose() == pxr::UsdGeomTokens->guide)
                        {
                            break;
                        }

                        pxr::TfToken Value;
                        Imageable.GetVisibilityAttr().Get(&Value);
                        if (Value == pxr::UsdGeomTokens->invisible)
                        {
                            break;
                        }

                        std::vector<double> TimeSamples;
                        Imageable.GetVisibilityAttr().GetTimeSamples(&TimeSamples);
                        if (TimeSamples.size() > 0)
                        {
                            PrimWithSamples = Prim;
                            break;
                        }
                        else
                        {
                            Prim = Prim.GetParent();
                        }
                    }

                    if (PrimWithSamples) // found the parent with vis time samples
                    {
                        pxr::UsdGeomImageable Imageable(PrimWithSamples);
                        FBooleanTimeSamples VisTimeSamples;
                        if (GetVisTimeSamples(Imageable, VisTimeSamples))
                        {
                            CreateVisibilityTrack(*SceneComponent, USDPrim, VisTimeSamples);
                        }
                    }
                    else
                    {
                        // inherit from parent
                        USceneComponent* Comp = USDPrim.GetParent() ? FindObjectFromPath<USceneComponent>(USDPrim.GetParent().GetPath()) : nullptr;
                        SetVisibility(*SceneComponent, USDPrim, Comp ? Comp->ShouldRender() : true /*if no parent, inherit means visible*/);
                    }
                }
                else // Root - always visible
                {
                    SetVisibility(*SceneComponent, USDPrim, true);
                }
            }
        }
    }

    // Load descendant
    if(ShouldLoadChildren)
    {
        auto ChildPrims = USDPrim.GetFilteredChildren(pxr::UsdTraverseInstanceProxies());
        for(auto ChildPrim : ChildPrims)
        {
            LoadUSD(ChildPrim.GetPath(), bLoadChildren);
        }
    }

    return SceneComponent;
}

void ASimReadyStageActor::LoadDefaultPrim()
{
    auto DefaultPrim = GetUSDStage()->GetDefaultPrim();
    if(!DefaultPrim)
    {
        USDDefaultPrim = nullptr;
        return;
    }

    auto SceneComp = FindObjectFromPath<USceneComponent>(DefaultPrim.GetPath());
    if(!SceneComp)
    {
        return;
    }

    if(SceneComp->GetOwner()->GetRootComponent() != SceneComp)
    {
        return;
    }

    USDDefaultPrim = SceneComp->GetOwner();
}

bool ASimReadyStageActor::IsInPIEOrStandaloneMode()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (GEditor && World && World == GEditor->GetEditorWorldContext().World() && !World->IsPlayInEditor()
        && !GEditor->PlayWorld)
    {
        return false;
    }
#endif

    return true;
}

#if WITH_EDITOR
void ASimReadyStageActor::SpawnGlobalPostProcessVolume()
{
    TActorIterator<APostProcessVolume> ActorIter = TActorIterator<APostProcessVolume>(GetWorld());
    if (!ActorIter)
    {
        APostProcessVolume* PostProcessVolume = GetWorld()->SpawnActor<APostProcessVolume>();
        PostProcessVolume->bUnbound = true;
        PostProcessVolume->SetActorScale3D(FVector(0.01f, 0.01f, 0.01f));
    }
}

TArray<UMeshComponent*> ASimReadyStageActor::FindMeshComponents(const FString& PrimPath)
{
    if (auto Object = USDPathToObject.Find(FSimReadyPathHelper::PrimPathToKey(PrimPath)))
    {
        if (!Object->IsValid())
        {
            return {};
        }

        auto SceneComponent = Cast<USceneComponent>(Object->Get());
        if (!SceneComponent)
        {
            return {};
        }


        auto Actor = SceneComponent->GetOwner();
        if (!Actor)
        {
            return {};
        }

        TArray<UMeshComponent*> MeshComponents;
        Actor->GetComponents<UMeshComponent>(MeshComponents, false);

        return MeshComponents;
    }

    return {};
}

TSet<FString> ASimReadyStageActor::GetSelectedPrimPaths()
{
    TSet<FString> FocusedPrims;
    USelection* SelectedActors = GEditor->GetSelectedActors();
    for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
    {
        AActor* Actor = Cast<AActor>(*Iter);
        if (Actor)
        {
            auto PrimPathKey = USDPathToObject.FindKey((UObject*)Actor->GetRootComponent());
            if (PrimPathKey)
            {
                FocusedPrims.Add(FSimReadyPathHelper::KeyToPrimPath(*PrimPathKey));
            }
        }
    }

    return FocusedPrims;
}
#endif

TSharedPtr<void> ASimReadyStageActor::SetStateScoped(EState InState)
{
    if(State != EState::None && State != InState)
    {
        return nullptr;
    }

    auto OldState = State;
    State = InState;

    auto OldUndo = GUndo;
    GUndo = nullptr;	// Not modify level

    return MakeShareable((void*)ULLONG_MAX, [this, OldState, OldUndo](auto)
        {
            State = OldState;
            GUndo = OldUndo;
        }
    );
}

bool ASimReadyStageActor::LoadStaticMesh(const pxr::UsdGeomMesh& USDMesh, UStaticMeshComponent& MeshComponent, bool bIgnoreVisibility)
{
    DECLARE_SCOPE_CYCLE_COUNTER(TEXT("SimReadyStageActorLoadStaticMesh"), STAT_SimReadyStageActorLoadStaticMesh, STATGROUP_SimReady);

    // Find in Cache
    if(!USDMesh)
    {
        return false;
    }

    if (HasValidImportStage())
    {
        // check if the collision shape is visible
        // the invisible shape won't be imported
        if (USDMesh.ComputePurpose() == pxr::UsdGeomTokens->guide)
        {
            return false;
        }

        if (!bIgnoreVisibility)
        {
            pxr::TfToken VisibilityValue;
            USDMesh.GetVisibilityAttr().Get(&VisibilityValue);
            if (VisibilityValue == pxr::UsdGeomTokens->invisible)
            {
                return false;
            }

            // check parent as well
            if (USDMesh.GetPrim().GetParent())
            {
                USceneComponent* Parent = FindObjectFromPath<USceneComponent>(USDMesh.GetPath().GetParentPath());
                if (Parent && (!Parent->IsVisible() || (Parent->GetOwner() && Parent->GetOwner()->IsHidden())))
                {
                    return false;
                }
            }
        }
    }
    
    FString Name = USDMesh.GetPrim().GetName().GetText();
    if (FSimReadyReferenceCollector::Get().FindDependency(USDMesh.GetPath().GetText()))
    {
        FString Text = FString::Printf(TEXT("Loading Mesh %s..."), *Name);
        FSimReadySlowTask::Get().UpdateProgress(1.0, FText::FromString(Text));
    }

    FSHAHash Hash = FUSDHashGenerator::ComputeSHAHash(USDMesh);
    if (Name.StartsWith(TEXT("Section"), ESearchCase::CaseSensitive))
    {
        if (USDMesh.GetPrim().GetParent())
        {
            Name = FString(USDMesh.GetPrim().GetParent().GetName().GetText()) + TEXT("_") + Name;
        }
    }
    FString HashString = Hash.ToString();
    Name = GetUniqueImportName(HashString, Name);
    UStaticMesh* StaticMesh = Cast<UStaticMesh>(FUSDGeometryCache::Find(HashString));
    if (StaticMesh == nullptr)
    {
        StaticMesh = LoadImportObject<UStaticMesh>(ImportType::Mesh, Name);
    }
    if(!StaticMesh)
    {	
        FVector Offset = FVector::ZeroVector;
        if (ImportSettings.bSimReadyVehicle)
        {
            Offset = FUSDCARLAVehicleTools::GetPartOffset(USDMesh);
        }

        // Create static mesh to avoid breaking existing mesh
        StaticMesh = FSimReadyUSDImporterHelper::CreateStaticMesh(USDMesh, GetAssetPackage(ImportType::Mesh, Name), GetAssetName(Name), GetAssetFlag(), Offset);

        // Construct mesh
        if(!StaticMesh)
        {
            return false;
        }
    }

    if (StaticMesh)
    {
        FUSDGeometryCache::Add(HashString, StaticMesh);
    }

    MeshComponent.SetStaticMesh(StaticMesh);

    bool bEnableQueryCollision = GetDefault<USimReadySettings>()->bEnableQueryCollision;
    MeshComponent.BodyInstance.SetCollisionEnabled(bEnableQueryCollision ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);	// Disable collision to boost mesh construction

    return true;
}

void ASimReadyStageActor::UpdateDomeLightSphere()
{
    if (SkyMeshComponent.IsValid())
    {
        FBox SkyBound = SkyMeshComponent->Bounds.GetBox();
        // Check whether the sky is smaller than the world
        if (!SkyBound.IsInside(StageBounds))
        {
            // Set new scale to the sky
            float MinScale = StageBounds.Min.SizeSquared() / SkyBound.Min.SizeSquared();
            float MaxScale = StageBounds.Max.SizeSquared() / SkyBound.Max.SizeSquared();
            SkyMeshComponent->SetRelativeScale3D(FVector(FMath::CeilToFloat(FMath::Max(MinScale, MaxScale))));
        }
    }
}

void ASimReadyStageActor::LoadDomeLight(const pxr::UsdLuxDomeLight& DomeLight)
{
    // Get the old/new UsdLuxLight schema preference
    bool bPreferNewSchema = GetDefault<USimReadySettings>()->bPreferNewUsdLuxLightSchemaOnImport;
    pxr::UsdAttribute LightAttr;

    auto SkySphere = LoadObject<UStaticMesh>(nullptr, *USimReadyMDL::GetContentPath(TEXT("/SimReady/Skysphere")));
    auto SkyMaterial = LoadObject<UMaterial>(nullptr, *USimReadyMDL::GetContentPath(TEXT("/SimReady/SkyMaterial")));
    if (SkySphere == nullptr || SkyMaterial == nullptr)
    {
        return;
    }

    // Set a skybox
    bool bRecap = false;
    bool bVisibleInPrimaryRay = true;
    auto VisibleInPrimaryRayAttr = DomeLight.GetPrim().GetAttribute(pxr::TfToken("visibleInPrimaryRay"));
    if (VisibleInPrimaryRayAttr)
    {
        bVisibleInPrimaryRay = GetUSDValue<bool>(VisibleInPrimaryRayAttr);
    }
    auto Path = DomeLight.GetPath();
    UStaticMeshComponent* SkyComponent = FindObjectFromPath<UStaticMeshComponent>(Path);
    if (SkyComponent == nullptr)
    {
        SkyComponent = CreateActorAndComponent<UStaticMeshComponent, AStaticMeshActor>(DomeLight.GetPrim());
        bRecap = true;
    }
    SkyComponent->SetStaticMesh(SkySphere);
    SkyComponent->SetCastShadow(false);
    SkyMeshComponent = SkyComponent;
    
    FString Name = DomeLight.GetPrim().GetName().GetText();
    Name += TEXT("_SkyMaterialInst");
    Name = GetUniqueImportName(Path.GetText(), Name);
    UMaterialInstanceConstant* SkyMaterialInst = nullptr;//LoadImportObject<UMaterialInstanceConstant>(ImportType::Material, Name);
    if (SkyMaterialInst == nullptr)
    {
        SkyMaterialInst = Cast<UMaterialInstanceConstant>(SkyComponent->GetMaterial(0));

        if (SkyMaterialInst == nullptr)
        {
            SkyMaterialInst = NewObject<UMaterialInstanceConstant>(GetAssetPackage(ImportType::Material, Name), GetAssetName(Name), GetAssetFlag());
            if (SkyMaterialInst == nullptr)
            {
                return;
            }

            SkyMaterialInst->SetParentEditorOnly(SkyMaterial);
        }

        // Set texture/intensity/exposure
        LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(DomeLight.GetPrim(), DomeLight.GetTextureFileAttr(), bPreferNewSchema);
        auto TextureFile = GetUSDValue<pxr::SdfAssetPath>(LightAttr);
        FString OmniPath = UTF8_TO_TCHAR(TextureFile.GetResolvedPath().c_str());
        auto Texture = GetTexture(OmniPath);
        LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(DomeLight.GetPrim(), DomeLight.GetColorAttr(), bPreferNewSchema);
        FLinearColor Color = USDConvertToLinearColor(GetUSDValue<pxr::GfVec3f>(LightAttr));
        LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(DomeLight.GetPrim(), DomeLight.GetIntensityAttr(), bPreferNewSchema);
        float Intensity = GetUSDValue<float>(LightAttr);
        LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(DomeLight.GetPrim(), DomeLight.GetExposureAttr(), bPreferNewSchema);
        float Exposure = GetUSDValue<float>(LightAttr);
        bool bTextureEnabled = (Texture != nullptr);
        
        // Check exist material
        if (!bRecap)
        {
            float PreTextureEnabled;
            if (SkyMaterialInst->GetScalarParameterValue(TEXT("TextureEnabled"), PreTextureEnabled))
            {
                float FloatTextureEnabled = bTextureEnabled ? 1.0f : 0.0f;
                if (PreTextureEnabled != FloatTextureEnabled)
                {
                    bRecap = true;
                }
            }
        }
        if (!bRecap && bTextureEnabled)
        {
            UTexture* PreTexture;
            if (SkyMaterialInst->GetTextureParameterValue(TEXT("Texture"), PreTexture))
            {
                if (PreTexture != Texture)
                {
                    bRecap = true;
                }
            }
        }

        if (!bRecap)
        {
            FLinearColor PreColor;
            if (SkyMaterialInst->GetVectorParameterValue(TEXT("Color"), PreColor))
            {
                if (PreColor != Color)
                {
                    bRecap = true;
                }
            }
        }

        if (!bRecap)
        {
            float PreIntensity;
            if (SkyMaterialInst->GetScalarParameterValue(TEXT("Intensity"), PreIntensity))
            {
                if (PreIntensity != Intensity)
                {
                    bRecap = true;
                }
            }
        }

        if (!bRecap)
        {
            float PreExposure;
            if (SkyMaterialInst->GetScalarParameterValue(TEXT("Exposure"), PreExposure))
            {
                if (PreExposure != Exposure)
                {
                    bRecap = true;
                }
            }
        }

        if (bTextureEnabled)
        {
            SkyMaterialInst->SetTextureParameterValueEditorOnly(TEXT("Texture"), Texture);
        }
        SkyMaterialInst->SetScalarParameterValueEditorOnly(TEXT("TextureEnabled"), bTextureEnabled ? 1.0f : 0.0f);
        SkyMaterialInst->SetVectorParameterValueEditorOnly(TEXT("Color"), Color);
        SkyMaterialInst->SetScalarParameterValueEditorOnly(TEXT("Intensity"), Intensity);
        SkyMaterialInst->SetScalarParameterValueEditorOnly(TEXT("Exposure"), Exposure);

        SkyMaterialInst->MarkPackageDirty();
        FAssetRegistryModule::AssetCreated(SkyMaterialInst);
    }
    SkyComponent->SetMaterial(0, SkyMaterialInst);

    FNamedParameterTimeSamples ParameterTimeSamples;
    FColorTimeSamples ColorTimeSamples;
    LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(DomeLight.GetPrim(), DomeLight.GetColorAttr(), bPreferNewSchema);
    if (GetUSDTimeSamples<pxr::GfVec3f, FColorTimeSamples>(LightAttr, ColorTimeSamples))
    {
        ParameterTimeSamples.ColorTimeSamples.Add(TEXT("Color"), ColorTimeSamples);
    }
    FFloatTimeSamples IntensityTimeSamples;
    LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(DomeLight.GetPrim(), DomeLight.GetIntensityAttr(), bPreferNewSchema);
    if (GetUSDTimeSamples<float, FFloatTimeSamples>(LightAttr, IntensityTimeSamples))
    {
        ParameterTimeSamples.ScalarTimeSamples.Add(TEXT("Intensity"), IntensityTimeSamples);
    }
    FFloatTimeSamples ExposureTimeSamples;
    LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(DomeLight.GetPrim(), DomeLight.GetExposureAttr(), bPreferNewSchema);
    if (GetUSDTimeSamples<float, FFloatTimeSamples>(LightAttr, ExposureTimeSamples))
    {
        ParameterTimeSamples.ScalarTimeSamples.Add(TEXT("Exposure"), ExposureTimeSamples);
    }
    USDSequenceImporter->CreateMaterialTrack(SkyComponent, 0, ParameterTimeSamples);

    const float DomeSkyDistance = 70000.0f;
    for (auto Actor : PreviewEnvironmentActors)
    {
        if (Actor.IsValid())
        {
            if (Actor->IsA<ASkyLight>())
            {
                auto SkyLight = Cast<ASkyLight>(Actor.Get());
                LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(DomeLight.GetPrim(), DomeLight.GetColorAttr(), bPreferNewSchema);
                SkyLight->GetLightComponent()->SetLightColor(USDConvertToLinearColor(GetUSDValue<pxr::GfVec3f>(LightAttr)));
                SkyLight->GetLightComponent()->SkyDistanceThreshold = DomeSkyDistance;
                SkyLight->GetLightComponent()->SourceType = SLS_CapturedScene;
                SkyLight->GetLightComponent()->SetCubemap(nullptr);
                if (bRecap)
                {
                    SkyLight->GetLightComponent()->RecaptureSky();
                }
            }
        }
    }	
}

bool ASimReadyStageActor::LoadLight(const pxr::SdfPath& Path, bool bLoadChildren)
{
    pxr::UsdPrim USDLight(GetUSDStage()->GetPrimAtPath(Path.GetPrimPath()));
    if(!USDLight)
    {
        return false;
    }

    // Get the old/new UsdLuxLight schema preference
    bool bPreferNewSchema = GetDefault<USimReadySettings>()->bPreferNewUsdLuxLightSchemaOnImport;
    pxr::UsdAttribute LightAttr;

    ULightComponent* LightComponent = nullptr;

    bool bShouldLoadProperty = false;
    if(USDLight.IsA<pxr::UsdLuxDistantLight>())
    {
        // Create directional light component and actor
        LightComponent = CreateActorAndComponent<UDirectionalLightComponent, ADirectionalLight>(USDLight);
        bShouldLoadProperty = ShouldLoadProperty(Path, SimReadyUsdLuxLightCompat::DistantLightGetSchemaAttributeNames());
    }
    else if(USDLight.IsA<pxr::UsdLuxSphereLight>())
    {
        bool bSpotLight = false;
        auto Cone = pxr::UsdLuxShapingAPI(USDLight);
        if(Cone)
        {
            LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(USDLight, Cone.GetShapingConeAngleAttr(), bPreferNewSchema);
            float OuterConeAngle = GetUSDValue<float>(LightAttr);
            if (OuterConeAngle > 0 && OuterConeAngle < 180)
            {
                bSpotLight = true;
            }
        }

        //NOTE: USD sphere light might be imported as Spot or Point light. We should check if there's already one in the stage and remove it.
        auto ExistedLightComponent = FindObjectFromPath<ULightComponent>(Path.GetPrimPath());
        if (ExistedLightComponent)
        {
            if (bSpotLight != ExistedLightComponent->IsA<USpotLightComponent>())
            {
                // remove the existed component
                FString UEPrimPath = Path.GetPrimPath().GetText();
                ReserveObject(*ExistedLightComponent->GetOwner(), UEPrimPath);
                USDPathToObject.Remove(FSimReadyPathHelper::PrimPathToKey(UEPrimPath));
                GEngine->BroadcastLevelActorListChanged();
            }
        }

        if (bSpotLight)
        {
            LightComponent = CreateActorAndComponent<USpotLightComponent, ASpotLight>(USDLight);
        }
        else
        {
            LightComponent = CreateActorAndComponent<UPointLightComponent, APointLight>(USDLight);
        }
        bShouldLoadProperty = ShouldLoadProperty(Path, SimReadyUsdLuxLightCompat::SphereLightGetSchemaAttributeNames());
    }
    else if(USDLight.IsA<pxr::UsdLuxRectLight>())
    {
        LightComponent = CreateActorAndComponent<URectLightComponent, ARectLight>(USDLight);
        bShouldLoadProperty = ShouldLoadProperty(Path, SimReadyUsdLuxLightCompat::RectLightGetSchemaAttributeNames());
    }
    else if (USDLight.IsA<pxr::UsdLuxDomeLight>())
    {
        LoadDomeLight(pxr::UsdLuxDomeLight(USDLight));
        LoadSceneComponent(Path, bLoadChildren, false);
        return true;
    }

    if(!LightComponent)
    {
        UE_LOG(LogSimReadyUsd, Warning, TEXT("Doesn't support this type of light: %s"), *FString(USDLight.GetPath().GetText()));
        return false;
    }

    LightComponent->SetMobility(EComponentMobility::Movable); // Light is always moveable.

    if (!bShouldLoadProperty)
    {
        bShouldLoadProperty = ShouldLoadProperty(Path, SimReadyUsdLuxLightCompat::LightGetSchemaAttributeNames()) || ShouldLoadProperty(Path, SimReadyUsdLuxLightCompat::ShapingAPIGetSchemaAttributeNames());
    }

    // Set basic light properties
    if(bShouldLoadProperty)
    {
        FNamedParameterTimeSamples NamedParameterTimeSamples;
        if (USDImportLight(USDLight, *LightComponent, NamedParameterTimeSamples))
        {
            auto Shape = pxr::UsdLuxShapingAPI(USDLight.GetPrim());
            if (Shape)
            {
                // Load IES
                LightAttr = SimReadyUsdLuxLightCompat::GetLightAttr(USDLight, Shape.GetShapingIesFileAttr(), bPreferNewSchema);
                auto LightProfileFile = GetUSDValue<pxr::SdfAssetPath>(LightAttr);
                FString OmniPath = LightProfileFile.GetResolvedPath().c_str();
                auto Texture = GetTexture(OmniPath);
                if (Texture && Texture->IsA<UTextureLightProfile>())
                {
                    LightComponent->IESTexture = Cast<UTextureLightProfile>(Texture);
                }
            }
        }

        USDSequenceImporter->CreateObjectTrack(LightComponent, NamedParameterTimeSamples);
        LightComponent->MarkRenderStateDirty();
    }

    LoadSceneComponent(Path, bLoadChildren);

    return true;
}

template <typename UsdType>
void LoadLayerDataSetting(const pxr::VtDictionary& UsdSettings, const std::string SettingName, UsdType& OutValue)
{
    auto SettingEntry = UsdSettings.find(SettingName);
    if (SettingEntry != UsdSettings.end())
    {
        OutValue = pxr::VtValue::Cast<UsdType>(SettingEntry->second).template UncheckedGet<UsdType>();
    }
};

FORCEINLINE float EV100ToLuminance(float EV100)
{
    return 1.2 * FMath::Pow(2.0f, EV100);
}

FORCEINLINE float HistogramEVToEV100(float EV)
{
    return FMath::Log2(EV * 0.001f) + 3;
}

bool ASimReadyStageActor::SetEditorCamera(const FVector& CamPosition, const FRotator& CamRotation)
{
    auto World = GetWorld();
    if (World)
    {
        FLevelViewportInfo& ViewportInfo = World->EditorViews[ELevelViewportType::LVT_Perspective];
        ViewportInfo.CamPosition = CamPosition;
        ViewportInfo.CamRotation = CamRotation;

        if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
        {
            FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");

            // Notify level editors of the map change
            LevelEditor.BroadcastMapChanged(World, EMapChangeType::LoadMap);
        }

        // According to viewport to setup preview directional light
        for (auto Actor : PreviewEnvironmentActors)
        {
            if (Actor.IsValid() && Actor->IsA<ADirectionalLight>())
            {
                auto DirectionalLight = Cast<ADirectionalLight>(Actor.Get());
                DirectionalLight->SetActorRotation(ViewportInfo.CamRotation);
                break;
            }
        }

        return true;
    }

    return false;
}

void ASimReadyStageActor::ComputeEditorCameraPosition()
{
    if (StageBounds.IsValid)
    {
        FVector Center, Ext;
        StageBounds.GetCenterAndExtents(Center, Ext);
        Ext.Y = -Ext.Y;

        FVector CamPosition = Center + Ext * 2.0f;
        FVector CamTarget = Center;
        FRotator CamRotation = (CamTarget - CamPosition).ToOrientationRotator();

        SetEditorCamera(CamPosition, CamRotation);	
    }
}

void ASimReadyStageActor::LoadCustomLayerData(bool& bCameraSet)
{
    bCameraSet = false;
    SkyMeshComponent = nullptr;
    StageBounds.Init();

    if (!GetUSDStage())
    {
        return;
    }

    auto RootLayer = GetUSDStage()->GetRootLayer();
    
    pxr::VtDictionary CustomLayerData;
    const pxr::VtDictionary* CameraSettingsDict = nullptr;
    const pxr::VtDictionary* RenderSettingsDict = nullptr;
    if (RootLayer->HasCustomLayerData())
    {
        CustomLayerData = RootLayer->GetCustomLayerData();
        auto CameraSettingsEntry = CustomLayerData.find(std::string("cameraSettings"));
        if (CameraSettingsEntry != CustomLayerData.end())
        {
            CameraSettingsDict = &(CameraSettingsEntry->second.Get<pxr::VtDictionary>());
        }

        auto RenderSettingsEntry = CustomLayerData.find(std::string("renderSettings"));
        if (RenderSettingsEntry != CustomLayerData.end())
        {
            RenderSettingsDict = &(RenderSettingsEntry->second.Get<pxr::VtDictionary>());
        }
    }

    if (CameraSettingsDict)
    {
        auto CamUsdSettingsEntry = (*CameraSettingsDict).find("Perspective");
        if (CamUsdSettingsEntry != (*CameraSettingsDict).end())
        {
            const auto& CamUsdSettings = CamUsdSettingsEntry->second.Get<pxr::VtDictionary>();
            pxr::GfVec3d Position;
            pxr::GfVec3d Target;
            LoadLayerDataSetting<pxr::GfVec3d>(CamUsdSettings, "position", Position);
            LoadLayerDataSetting<pxr::GfVec3d>(CamUsdSettings, "target", Target);

            FVector CamPosition = USDConvertPosition(GetUSDStage(), USDConvert(Position), true);
            FVector CamTarget = USDConvertPosition(GetUSDStage(), USDConvert(Target), true);
            FRotator CamRotation = (CamTarget- CamPosition).ToOrientationRotator();

            bCameraSet = SetEditorCamera(CamPosition, CamRotation);
        }
    }

    if (RenderSettingsDict)
    {
        float CameraShutter = 50.0f; // ms in Kit, should convert to s
        float CamerafNumber = 5.0f;
        float CameraFilmIso = 100.0f;
        float TonemapFactor = 1.0f;
        LoadLayerDataSetting<float>(*RenderSettingsDict, "rtx:post:tonemap:cameraShutter", CameraShutter);
        LoadLayerDataSetting<float>(*RenderSettingsDict, "rtx:post:tonemap:fNumber", CamerafNumber);
        LoadLayerDataSetting<float>(*RenderSettingsDict, "rtx:post:tonemap:filmIso", CameraFilmIso);
        LoadLayerDataSetting<float>(*RenderSettingsDict, "rtx:post:tonemap:cm2Factor", TonemapFactor);

        bool bAutoExposure = false;
        float AdaptSpeed = 3.5f;
        float MinEV = 50.0f;
        float MaxEV = 100000.0f;
        float WhiteScale = 10.0f;
        LoadLayerDataSetting<bool>(*RenderSettingsDict, "rtx:post:histogram:enabled", bAutoExposure);
        LoadLayerDataSetting<float>(*RenderSettingsDict, "rtx:post:histogram:minEV", MinEV);
        LoadLayerDataSetting<float>(*RenderSettingsDict, "rtx:post:histogram:maxEV", MaxEV);
        LoadLayerDataSetting<float>(*RenderSettingsDict, "rtx:post:histogram:tau", AdaptSpeed);
        LoadLayerDataSetting<float>(*RenderSettingsDict, "rtx:post:histogram:whiteScale", WhiteScale);

        auto World = GetWorld();
        if (World)
        {
            TActorIterator<APostProcessVolume> PPVIter = TActorIterator<APostProcessVolume>(World);
            if (PPVIter)
            {
                PPVIter->Settings.bOverride_AutoExposureMethod = true;
                PPVIter->Settings.bOverride_AutoExposureBias = true;
                PPVIter->Settings.AutoExposureMethod = bAutoExposure ? EAutoExposureMethod::AEM_Histogram : EAutoExposureMethod::AEM_Manual;

                if (bAutoExposure)
                {
                    PPVIter->Settings.bOverride_AutoExposureSpeedUp = true;
                    PPVIter->Settings.bOverride_AutoExposureSpeedDown = true;
                    PPVIter->Settings.bOverride_AutoExposureMinBrightness = true;
                    PPVIter->Settings.bOverride_AutoExposureMaxBrightness = true;

                    PPVIter->Settings.AutoExposureSpeedUp = AdaptSpeed;
                    PPVIter->Settings.AutoExposureSpeedDown = AdaptSpeed;

                    static const auto VarDefaultAutoExposureExtendDefaultLuminanceRange = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.DefaultFeature.AutoExposure.ExtendDefaultLuminanceRange"));
                    const bool bExtendedLuminanceRange = VarDefaultAutoExposureExtendDefaultLuminanceRange->GetValueOnGameThread() == 1;

                    if (bExtendedLuminanceRange)
                    {
                        PPVIter->Settings.AutoExposureMinBrightness = HistogramEVToEV100(MinEV);
                        PPVIter->Settings.AutoExposureMaxBrightness = HistogramEVToEV100(MaxEV);
                    }
                    else
                    {
                        PPVIter->Settings.AutoExposureMinBrightness = EV100ToLuminance(HistogramEVToEV100(MinEV));
                        PPVIter->Settings.AutoExposureMaxBrightness = EV100ToLuminance(HistogramEVToEV100(MaxEV));
                    }

                    PPVIter->Settings.AutoExposureBias = FMath::Log2(10.0f / WhiteScale);
                }
                else
                {
                    PPVIter->Settings.bOverride_CameraShutterSpeed = true;
                    PPVIter->Settings.bOverride_DepthOfFieldFstop = true;
                    PPVIter->Settings.bOverride_CameraISO = true;
                    
                    PPVIter->Settings.CameraShutterSpeed = CameraShutter * 0.001f;
                    PPVIter->Settings.DepthOfFieldFstop = CamerafNumber;
                    PPVIter->Settings.CameraISO = CameraFilmIso;
                    PPVIter->Settings.AutoExposureBias = FMath::Log2(TonemapFactor);
                }
            }
        }
    }
}

void ASimReadyStageActor::LoadLayerTimeInfo()
{
    auto USDStage = GetUSDStage();
    if (!USDStage)
    {
        return;
    }

    pxr::SdfLayerRefPtr RootLayer = USDStage->GetRootLayer();

    if (!TimeCodeInfo.StartTimeCode.IsSet())
    {
        TimeCodeInfo.StartTimeCode = RootLayer->HasStartTimeCode() ? RootLayer->GetStartTimeCode() : TOptional<double>();
    }
    if (!TimeCodeInfo.EndTimeCode.IsSet())
    {
        TimeCodeInfo.EndTimeCode = RootLayer->HasEndTimeCode() ? RootLayer->GetEndTimeCode() : TOptional<double>();
    }
    if (!TimeCodeInfo.TimeCodesPerSecond.IsSet())
    {
        TimeCodeInfo.TimeCodesPerSecond = RootLayer->HasTimeCodesPerSecond() ? RootLayer->GetTimeCodesPerSecond() : (RootLayer->HasFramesPerSecond() ? RootLayer->GetFramesPerSecond() : TOptional<double>());
    }
}

bool ASimReadyStageActor::IsValidPrimPath(const pxr::SdfPath& Path)
{
    auto Stage = GetUSDStage();
    if(Stage && Stage->GetPrimAtPath(Path))
    {
        return true;
    }

    return false;
}

const pxr::UsdStageRefPtr& ASimReadyStageActor::GetUSDStage()
{
    return ImportUSDStage;
}

FString ASimReadyStageActor::GetUSDPath()
{
    if (ImportUSDStage)
    {
        return ImportUSDStage->GetRootLayer()->GetIdentifier().c_str();
    }

    return FString();
}

FString ASimReadyStageActor::GetUSDName()
{
    FString Name = FPaths::GetBaseFilename(GetUSDPath());
    if (!Name.IsEmpty())
    {
        FSimReadyPathHelper::FixAssetName(Name);
    }
    return Name;
}

EObjectFlags ASimReadyStageActor::GetAssetFlag()
{
    if (HasValidImportStage())
    {
        return EObjectFlags::RF_Standalone | EObjectFlags::RF_Public;
    }
    else
    {
        return EObjectFlags::RF_Transactional | EObjectFlags::RF_Public;
    }
}

UPackage* ASimReadyStageActor::GetAssetPackage(ImportType Type, const FString& AssetName)
{
    if (HasValidImportStage())
    {
        FString SubPath;
        switch(Type)
        {
        case ImportType::Mesh:
            SubPath = TEXT("Meshes");
            break;
        case ImportType::Material:
            SubPath = TEXT("Materials");
            break;
        case ImportType::Texture:
            SubPath = TEXT("Textures");
            break;
        case ImportType::Animation:
            SubPath = TEXT("Animations");
            break;
        }

        FString PkgPath = FPaths::Combine(SavePackagePath, SubPath);
        PkgPath = FPaths::Combine(PkgPath, AssetName);
        auto Package = CreatePackage(*PkgPath);
        Package->FullyLoad();
        return Package;
    }
    else
    {
        return GetTransientPackage();
    }
}

UTexture* ASimReadyStageActor::GetTexture(const FString& OmniPath)
{
    if (HasValidImportStage())
    {
        UTexture* Texture = nullptr;
        auto Object = USDPathToObject.Find(FSimReadyPathHelper::PrimPathToKey(OmniPath));
        if (Object && Object->IsValid())
        {
            Texture = Cast<UTexture>(Object->Get());
            if (Texture)
            {
                return Texture;
            }
        }

        FString TextureName = FPaths::GetBaseFilename(OmniPath);
        FSimReadyPathHelper::FixAssetName(TextureName);
        TextureName = GetUniqueImportName(OmniPath, TextureName);
        Texture = LoadImportObject<UTexture>(ImportType::Texture, TextureName);
        if (Texture)
        {
            return Texture;
        }

        Texture = USimReadyTexture::CreateTextureFromFile(OmniPath, GetAssetPackage(ImportType::Texture, TextureName), *TextureName, GetAssetFlag());
        if (Texture)
        {
            USDPathToObject.FindOrAdd(FSimReadyPathHelper::PrimPathToKey(OmniPath), Texture);
        }
        
        return Texture;
    }
    else
    {
        return nullptr;
    }
}

FName ASimReadyStageActor::GetAssetName(const FString& Name)
{
    if (HasValidImportStage())
    {
        return *Name;
    }
    else
    {
        return NAME_None;
    }
}

FString ASimReadyStageActor::GetUniqueImportName(const FString& KeyPath, const FString& Name)
{
    if (HasValidImportStage())
    {
        auto ExistName = ImportedNames.Find(KeyPath);
        if (ExistName == nullptr)
        {
            FString UniqueName = Name;
            while(ImportedNames.FindKey(UniqueName))
            {
                UniqueName = UniqueName + TEXT("_") + FString::FromInt(ImportedSuffixIndex++);
            }
            ImportedNames.Add(KeyPath, UniqueName);
            return UniqueName;
        }
        else
        {
            return *ExistName;
        }
    }
    else
    {
        return Name;
    }
}

void ASimReadyStageActor::PostLoadUSDStage(bool UpdateViewport)
{
    if (UpdateViewport)
    {
        ComputeEditorCameraPosition();
    }

    UpdateDomeLightSphere();
}

void ASimReadyStageActor::ImportUSD(const FString& UsdPath, const FString& PackagePath, const FSimReadyImportSettings& InImportSettings)
{
    FUSDGeometryCache::BackupCurrentCache();
    USimReadyMDL::ResetImportParametersSheet();
    ImportUSDStage = FSimReadyUSDHelper::LoadUSDStageFromPath(UsdPath);
    if (ImportUSDStage)
    {
        ImportOverwriteStatus = 0;
        ImportUSDSourceFile = UsdPath;
        ImportSettings = InImportSettings;
        int32 NumDependencies = FSimReadyReferenceCollector::Get().GetAllDependencies(ImportUSDStage, InImportSettings);
        FString AssetName = FPaths::GetBaseFilename(UsdPath);
        FSimReadyPathHelper::FixAssetName(AssetName);
        FSimReadySlowTask::Get().BeginProgress(NumDependencies, FText::FromString(FString::Printf(TEXT("Importing USD %s..."), *AssetName)), true, false);
        SavePackagePath = PackagePath / AssetName;
        if (ImportSettings.bSimReadyVehicle)
        {
            FUSDCARLAVehicleTools::InitPartOffsets(ImportUSDStage);
        }
        SpawnGlobalPostProcessVolume();
        bool bCameraSet = false;
        LoadCustomLayerData(bCameraSet);
        LoadLayerTimeInfo();
        if (InImportSettings.bImportAsBlueprint)
        {
            USDSequenceImporter->SetActorSequenceMode();
        }
        else
        {
            USDSequenceImporter->CreateLevelSequence(TimeCodeInfo.StartTimeCode, TimeCodeInfo.EndTimeCode, TimeCodeInfo.TimeCodesPerSecond);
        }
        OnUSDStageNotice(pxr::SdfPath::AbsoluteRootPath(), true);
        LoadChangedUSDPaths();
        PostLoadUSDStage(!bCameraSet && !ImportSettings.bImportAsBlueprint);

        UBlueprint* Blueprint = nullptr;
        if (InImportSettings.bImportAsBlueprint)
        {
            TArray<TWeakObjectPtr<UObject>> UsdObjects;
            USDPathToObject.GenerateValueArray(UsdObjects);
            TArray<AActor*> Actors;
            for (auto Obj : UsdObjects)
            {
                if (Obj.IsValid() && Obj.Get()->IsA<USceneComponent>())
                {
                    Actors.AddUnique(Cast<USceneComponent>(Obj.Get())->GetOwner());
                }
            }
            FKismetEditorUtilities::FHarvestBlueprintFromActorsParams Params;
            Params.bReplaceActors = true;
            Params.ParentClass = AActor::StaticClass();
            Params.bOpenBlueprint = false;
            
            FString BlueprintAssetName = AssetName + TEXT("_Blueprint");
            FString BlueprintPath = SavePackagePath / BlueprintAssetName;
            auto BlueprintPackage = CreatePackage(*BlueprintPath);
            if (BlueprintPackage)
            {
                Blueprint = FindObject<UBlueprint>(BlueprintPackage, *BlueprintAssetName);
                if (Blueprint)
                {
                    Blueprint->Rename(nullptr, GetTransientPackage(), REN_DoNotDirty | REN_DontCreateRedirectors | REN_NonTransactional);
                    Blueprint->MarkPendingKill();
                }
            }
            Blueprint = FKismetEditorUtilities::HarvestBlueprintFromActors(BlueprintPath, Actors, Params);
            if (Blueprint)
            {
                // Create ActorSequenceComponent to hold usd time-samples
                USCS_Node* NewNode = Blueprint->SimpleConstructionScript->CreateNode(UActorSequenceComponent::StaticClass());
                Blueprint->SimpleConstructionScript->AddNode(NewNode);
                USDSequenceImporter->BuildActorSequence(Blueprint, Actors, TimeCodeInfo.StartTimeCode, TimeCodeInfo.EndTimeCode, TimeCodeInfo.TimeCodesPerSecond);
                // Recompile
                FKismetEditorUtilities::CompileBlueprint(Blueprint);
            }
        }
        else
        {
            USDSequenceImporter->RemoveEmptyLevelSequence();
        }

        FSimReadySlowTask::Get().EndProgress();
        FSimReadyReferenceCollector::Get().Reset();

        if (!IsRunningCommandlet())
        {
            if (InImportSettings.bImportAsBlueprint && Blueprint)
            {
                TArray<UObject*> Objects;
                Objects.Add(Blueprint);
                GEditor->SyncBrowserToObjects( Objects, false );
            }
            else
            {
                TArray<FString> Folders;
                Folders.Add(PackagePath);
                FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
                ContentBrowserModule.Get().SyncBrowserToFolders(Folders);
            }
        }
        ImportUSDStage.Reset();
        ImportUSDStage = nullptr;
    }

    FUSDGeometryCache::RestoreLastBackup();
}

void ASimReadyStageActor::InitializePreviewEnvironment()
{
    UWorld* World = GetWorld();
    if (World)
    {
        // Add Sky Light
        ASkyLight* SkyLight = World->SpawnActor<ASkyLight>();
        SkyLight->GetLightComponent()->bCaptureEmissiveOnly = true;
        // Make the sky light movable
        SkyLight->GetLightComponent()->Mobility = EComponentMobility::Movable;
        // Set cubemap
        SkyLight->GetLightComponent()->SourceType = SLS_SpecifiedCubemap;
        UTextureCube* CubeTexture = LoadObject<UTextureCube>(nullptr, TEXT("/Engine/EditorMaterials/AssetViewer/EpicQuadPanorama_CC+EV1"));
        if (CubeTexture)
        {
            SkyLight->GetLightComponent()->SetCubemap(CubeTexture);
        }
        SkyLight->SetFolderPath_Recursively("SimReady/Preview");
        PreviewEnvironmentActors.Add(SkyLight);
        // Add Directional Light
        auto DirectionalLight = World->SpawnActor<ADirectionalLight>();
        // Make the directional light movable
        DirectionalLight->GetLightComponent()->Mobility = EComponentMobility::Movable;
        DirectionalLight->SetFolderPath_Recursively("SimReady/Preview");
        PreviewEnvironmentActors.Add(DirectionalLight);
    }
}

bool ASimReadyStageActor::GetUserResponse(const FString& PkgPath)
{
    EAppReturnType::Type UserResponse;
    if (ImportOverwriteStatus == 1)
    {
        UserResponse = EAppReturnType::YesAll;
    }
    else if (ImportOverwriteStatus == 2)
    {
        UserResponse = EAppReturnType::NoAll;
    }
    else
    {
        UserResponse = FMessageDialog::Open(
            EAppMsgType::YesNoYesAllNoAll,
            FText::Format(LOCTEXT("ImportAssetAlreadyExists", "Do you want to overwrite the existing asset?\n\nAn asset already exists at the import location: {0}"), FText::FromString(PkgPath)));
                
        ImportOverwriteStatus = UserResponse == EAppReturnType::YesAll ? 1 : (UserResponse == EAppReturnType::NoAll ? 2 : 0);
    }

    return (UserResponse == EAppReturnType::No || UserResponse == EAppReturnType::NoAll);
}

#undef LOCTEXT_NAMESPACE