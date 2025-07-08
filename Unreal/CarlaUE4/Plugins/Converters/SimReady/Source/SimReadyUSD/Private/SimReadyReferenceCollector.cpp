// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyReferenceCollector.h"
#include "AssetRegistryModule.h"
#include "SimReadyUSDModule.h"
#include "SimReadyUSDTranslator.h"
#include "SimReadyUSDHelper.h"
#include "Animation/AnimationAsset.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/DecalComponent.h"
#include "LandscapeComponent.h"
#include "LandscapeProxy.h"
#include "Extractors/SequenceExtractor.h"
#include "Materials/MaterialInterface.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Misc/SlowTask.h"
#include "USDConverter.h"


FSimReadyReferenceCollector& FSimReadyReferenceCollector::Get()
{
    static FSimReadyReferenceCollector Singleton;
    return Singleton;
}

FSimReadyReferenceCollector::FSimReadyReferenceCollector()
    : CurrentMaterialSkipped(false)
    , CurrentTextureSkipped(false)
    , CurrentDecalSkipped(false)
    , CurrentUnusedSkipped(false)
{
}

FSimReadyReferenceCollector::~FSimReadyReferenceCollector()
{
}

int32 FSimReadyReferenceCollector::GetAllDependencies(const TArray<AActor*>& Actors, const FSimReadyExportSettings& ExportSettings)
{
    Reset();
    // <No material> doesn't count the materialGetAllDependencies
    CurrentMaterialSkipped = !ExportSettings.bMDL && !ExportSettings.bPreviewSurface;
    // <Only preview surface> or <No-material> doesn't count the texture
    CurrentTextureSkipped = CurrentMaterialSkipped || (!ExportSettings.bMDL && ExportSettings.bPreviewSurface);
    
    CurrentDecalSkipped = !ExportSettings.bExportDecalActors;

    for (auto Actor : Actors)
    {
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
            GetDependenciesInternal(*SceneComp);
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

                GetDependenciesInternal(*StaticMeshComponent);
            }
        }
    }

    return SupportedDependencies.Num();
}

int32 FSimReadyReferenceCollector::GetAllDependencies(const pxr::UsdStageRefPtr& USDStage, const FSimReadyImportSettings& ImportSettings)
{
    Reset();

    CurrentUnusedSkipped = !ImportSettings.bImportUnusedReferences;
    GetDependenciesInternal(USDStage, pxr::SdfPath::AbsoluteRootPath(), false);

    return SupportedDependencies.Num();
}

void FSimReadyReferenceCollector::Reset()
{
    CurrentMaterialSkipped = false;
    CurrentTextureSkipped = false;
    CurrentDecalSkipped = false;
    CurrentUnusedSkipped = false;
    SupportedDependencies.Reset();
    LoadedDependencies.Reset();
}

bool FSimReadyReferenceCollector::FindDependency(const FName& PackageName)
{
    if (SupportedDependencies.Remove(PackageName) > 0)
    {
        return true;
    }

    return false;
}

void FSimReadyReferenceCollector::GetDependenciesInternal(const pxr::UsdStageRefPtr& USDStage, const pxr::SdfPath& Path, bool bFromSkelRoot)
{
    auto Prim = USDStage->GetPrimAtPath(Path.GetAbsoluteRootOrPrimPath());
    if (!Prim)
    {
        return;
    }

    // Mesh | Skeleton | Animation | Material
    if (!bFromSkelRoot && Prim.IsA<pxr::UsdGeomMesh>())
    {
        SupportedDependencies.Add(Prim.GetPath().GetText());
        
        pxr::UsdShadeMaterialBindingAPI USDMaterialBinding(Prim);
        auto Material = pxr::UsdShadeMaterial(USDMaterialBinding.ComputeBoundMaterial());
        if (Material)
        {
            SupportedDependencies.Add(Material.GetPath().GetText());
        }
    }
    else if (!bFromSkelRoot && Prim.IsA<pxr::UsdSkelRoot>())
    {
        bFromSkelRoot = true;
        pxr::UsdSkelCache USDSkelCache;
        USDSkelCache.Populate(pxr::UsdSkelRoot(Prim));

        std::vector< pxr::UsdSkelBinding > USDSkeletonBindings;
        USDSkelCache.ComputeSkelBindings(pxr::UsdSkelRoot(Prim), &USDSkeletonBindings);

        if (USDSkeletonBindings.size() == 0)
        {
            TArray<pxr::UsdPrim> MeshPrims;
            FindAllPrims<pxr::UsdGeomMesh>(Prim, MeshPrims);
            for (auto MeshPrim : MeshPrims)
            {
                SupportedDependencies.Add(MeshPrim.GetPath().GetText());

                pxr::UsdShadeMaterialBindingAPI USDMaterialBinding(MeshPrim);
                auto Material = pxr::UsdShadeMaterial(USDMaterialBinding.ComputeBoundMaterial());
                if (Material)
                {
                    SupportedDependencies.Add(Material.GetPath().GetText());
                }
            }
        }

        for (auto SkeletonBinding : USDSkeletonBindings)
        {
            const pxr::UsdSkelSkeleton& USDSkeleton = SkeletonBinding.GetSkeleton();
            // Skeleton
            SupportedDependencies.Add(USDSkeleton.GetPath().GetText());

            // Mesh
            if (USDSkeletonBindings.size() > 1)
            {
                SupportedDependencies.Add(USDSkeleton.GetPath().GetParentPath().GetText());
            }
            else
            {
                SupportedDependencies.Add(Prim.GetPath().GetText());
            }

            // Material
            for (const pxr::UsdSkelSkinningQuery& SkinningQuery : SkeletonBinding.GetSkinningTargets())
            {
                pxr::UsdShadeMaterialBindingAPI USDMaterialBinding(SkinningQuery.GetPrim());
                auto Material = pxr::UsdShadeMaterial(USDMaterialBinding.ComputeBoundMaterial());
                if (Material)
                {
                    SupportedDependencies.Add(Material.GetPath().GetText());
                }
            }

            // Animation
            pxr::UsdSkelSkeletonQuery SkelQuery = USDSkelCache.GetSkelQuery(USDSkeleton);
            pxr::UsdSkelAnimQuery AnimQuery = SkelQuery.GetAnimQuery();
            std::vector<double> TimeSamples;
            if (AnimQuery && AnimQuery.GetJointTransformTimeSamples(&TimeSamples))
            {
                SupportedDependencies.Add(AnimQuery.GetPrim().GetPath().GetText());
            }
        }

        // Stop recursion if skipping scope searching
        if (CurrentUnusedSkipped)
        {
            return;
        }
    }
    else if (!CurrentUnusedSkipped && Prim.IsA<pxr::UsdShadeMaterial>())
    {
        auto Material = pxr::UsdShadeMaterial(Prim);
        if (Material)
        {
            SupportedDependencies.Add(Material.GetPath().GetText());
        }
    }

    if(Path.IsAbsoluteRootOrPrimPath())
    {
        auto ChildPrims = Prim.GetFilteredChildren(pxr::UsdTraverseInstanceProxies());
        for(auto ChildPrim : ChildPrims)
        {
            GetDependenciesInternal(USDStage, ChildPrim.GetPath(), bFromSkelRoot);
        }
    }
}

void FSimReadyReferenceCollector::GetTextureDependencies(const FName& PackageName, const FString& Root)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    TArray<FName> Dependencies;
    AssetRegistryModule.Get().GetDependencies(PackageName, Dependencies, UE::AssetRegistry::EDependencyCategory::Package);

    for ( auto DependsIt = Dependencies.CreateConstIterator(); DependsIt; ++DependsIt )
    {
        if ( !LoadedDependencies.Contains(*DependsIt) )
        {
            const bool bIsScriptPackage = (*DependsIt).ToString().StartsWith(TEXT("/Script"));
            const bool bIsInSamePackage = (*DependsIt).ToString().StartsWith(Root);
            if (!bIsScriptPackage && !bIsInSamePackage)
            {
                TArray<FAssetData> AssetDatas;
                AssetRegistryModule.Get().GetAssetsByPackageName(*DependsIt, AssetDatas);

                bool bSupported = false;
                for (auto Data : AssetDatas)
                {
                    if (Data.AssetClass.IsEqual(TEXT("Texture2D"))
                    || Data.AssetClass.IsEqual(TEXT("Texture2DDynamic"))
                    || Data.AssetClass.IsEqual(TEXT("TextureCube"))
                    || Data.AssetClass.IsEqual(TEXT("TextureRenderTarget2D")))
                    {
                        bSupported = true;
                        break;
                    }
                }

                if (bSupported)
                {
                    SupportedDependencies.Add(*DependsIt);
                }
                LoadedDependencies.Add(*DependsIt);
                GetTextureDependencies(*DependsIt, Root);
            }
        }
    }
}

void FSimReadyReferenceCollector::GetDependenciesInternal(const USceneComponent& Component)
{
    auto IsComponentHidden = [](const USceneComponent& Comp)
    {
        return (!Comp.IsVisible() && FSequenceExtractor::Get().GetVisibilityTimeSamples(&Comp) == nullptr)
        || (Comp.GetOwner() && Comp.GetOwner()->IsHidden() && FSequenceExtractor::Get().GetVisibilityTimeSamples(Comp.GetOwner()) == nullptr);
    };

    auto AddMaterialDependencies = [&](const FName& PackageName)
    {
        SupportedDependencies.Add(PackageName);
        if (!CurrentTextureSkipped)
        {
            GetTextureDependencies(PackageName, PackageName.ToString());
        }
    };

    if (!IsComponentHidden(Component))
    {
        const int32 LODToExport = 0;
        if (Component.IsA<UStaticMeshComponent>())
        {
            auto StaticMeshComponent = Cast<UStaticMeshComponent>(&Component);

            if (StaticMeshComponent->GetStaticMesh())
            {
                // Mesh
                auto StaticMesh = StaticMeshComponent->GetStaticMesh();
                SupportedDependencies.Add(*StaticMesh->GetOuter()->GetName());

                // Material and Texture
                int32 NumSections = StaticMesh->GetSectionInfoMap().GetSectionNumber(LODToExport);
                for (int SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
                {
                    FMeshSectionInfo Info = StaticMesh->GetSectionInfoMap().Get(LODToExport, SectionIndex);

                    UMaterialInterface* MeshMaterial = StaticMesh->GetMaterial(Info.MaterialIndex);
                    UMaterialInterface* ComponentMaterial = StaticMeshComponent->GetMaterial(Info.MaterialIndex);
                    if (MeshMaterial != ComponentMaterial)
                    {
                        if (MeshMaterial)
                        {
                            AddMaterialDependencies(*MeshMaterial->GetOuter()->GetName());
                        }

                        if (ComponentMaterial)
                        {
                            AddMaterialDependencies(*ComponentMaterial->GetOuter()->GetName());
                        }
                    }
                    else
                    {
                        if (MeshMaterial)
                        {
                            AddMaterialDependencies(*MeshMaterial->GetOuter()->GetName());
                        }
                    }
                }
            }
        }
        else if (Component.IsA<USkeletalMeshComponent>())
        {
            auto SkeletalMeshComponent = Cast<USkeletalMeshComponent>(&Component);
        
            if (SkeletalMeshComponent->SkeletalMesh)
            {
                // Mesh
                auto SkeletalMesh = SkeletalMeshComponent->SkeletalMesh;
                SupportedDependencies.Add(*SkeletalMesh->GetOuter()->GetName());
            
                // Material and Texture
                const FSkeletalMeshRenderData * SkelMeshRenderData = SkeletalMeshComponent->SkeletalMesh->GetResourceForRendering();
                if (SkelMeshRenderData && SkelMeshRenderData->LODRenderData.Num() > 0)
                {
                    const FSkeletalMeshLODRenderData& LODData = SkelMeshRenderData->LODRenderData[LODToExport];
                    const FSkeletalMeshLODInfo& Info = *(SkeletalMeshComponent->SkeletalMesh->GetLODInfo(LODToExport));

                    for (int32 SectionIndex = 0; SectionIndex < LODData.RenderSections.Num(); SectionIndex++)
                    {
                        int32 MaterialIndex = LODData.RenderSections[SectionIndex].MaterialIndex;
                        if (SectionIndex < Info.LODMaterialMap.Num() && SkeletalMeshComponent->SkeletalMesh->Materials.IsValidIndex(Info.LODMaterialMap[SectionIndex]))
                        {
                            MaterialIndex = Info.LODMaterialMap[SectionIndex];
                            MaterialIndex = FMath::Clamp(MaterialIndex, 0, SkeletalMeshComponent->SkeletalMesh->Materials.Num());
                        }
                        UMaterialInterface* MeshMaterial = SkeletalMeshComponent->SkeletalMesh->Materials[MaterialIndex].MaterialInterface;
                        UMaterialInterface* ComponentMaterial = SkeletalMeshComponent->GetMaterial(MaterialIndex);
                        
                        if (MeshMaterial != ComponentMaterial)
                        {
                            if (MeshMaterial)
                            {
                                AddMaterialDependencies(*MeshMaterial->GetOuter()->GetName());
                            }

                            if (ComponentMaterial)
                            {
                                AddMaterialDependencies(*ComponentMaterial->GetOuter()->GetName());
                            }
                        }
                        else
                        {
                            if (MeshMaterial)
                            {
                                AddMaterialDependencies(*MeshMaterial->GetOuter()->GetName());
                            }
                        }
                    }
                }

                // Animation
                UAnimSingleNodeInstance* SingleNodeInstance = SkeletalMeshComponent->GetSingleNodeInstance();
                if (SingleNodeInstance && SingleNodeInstance->GetAnimationAsset())
                {
                    SupportedDependencies.Add(*SingleNodeInstance->GetAnimationAsset()->GetOuter()->GetName());
                }
            }
        }
        else if (!CurrentMaterialSkipped)
        {
            if (Component.IsA<ULandscapeComponent>())
            {
                auto LandscapeComponent = Cast<ULandscapeComponent>(&Component);
                if (LandscapeComponent->GetLandscapeProxy() && LandscapeComponent->GetLandscapeProxy()->LandscapeMaterial)
                {
                    AddMaterialDependencies(*LandscapeComponent->GetLandscapeProxy()->LandscapeMaterial->GetOuter()->GetName());
                }
            }
            else if (Component.IsA<UTextRenderComponent>())
            {
                auto TextRenderComponent = Cast<UTextRenderComponent>(&Component);
                if (TextRenderComponent->TextMaterial)
                {
                    AddMaterialDependencies(*TextRenderComponent->TextMaterial->GetOuter()->GetName());
                }
            
            }
            else if (!CurrentDecalSkipped && Component.IsA<UDecalComponent>())
            {
                auto DecalComponent = Cast<UDecalComponent>(&Component);
                if (DecalComponent->GetDecalMaterial())
                {
                    AddMaterialDependencies(*DecalComponent->GetDecalMaterial()->GetOuter()->GetName());
                }
            }		
        }
    }

    TArray<USceneComponent*> Children;
    Component.GetChildrenComponents(false, Children);
    for(auto Child : Children)
    {
        if(Child)
        {
            GetDependenciesInternal(*Child);
        }
    }
}

