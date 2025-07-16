// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "CoreMinimal.h"
#include "USDConverter.h"
#include "SimReadyStageActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

#define MIN_CULL_DISTANCE 3000.0f
#define MAX_CULL_DISTANCE 20000.0f

bool FUSDExporter::IsPointInstancer(USceneComponent& SceneComp)
{
    // check child is Prototypes
    auto AttachedComps = SceneComp.GetAttachChildren();
    if (AttachedComps.Num() == 1 && AttachedComps[0] && AttachedComps[0]->GetName() == TEXT("Prototypes"))
    {
        auto PrototypeComps = AttachedComps[0]->GetAttachChildren();
        if (PrototypeComps.Num() > 0)
        {
            for (auto PrototypeComp : PrototypeComps)
            {
                auto MeshComps = PrototypeComp->GetAttachChildren();

                if (MeshComps.Num() != 1)
                {
                    return false;
                }

                if (!MeshComps[0]->IsA<UInstancedStaticMeshComponent>())
                {
                    return false;
                }
            }
            
            return true;
        }
    }
    return false;
}

pxr::UsdGeomMesh FUSDExporter::ExportInstancedStaticMeshComponent(UInstancedStaticMeshComponent& UInstancedStaticMeshComp)
{
    pxr::UsdGeomMesh USDMesh;
    return USDMesh;
}

pxr::UsdGeomPointInstancer FUSDExporter::ExportPointInstancer(USceneComponent& SceneComp)
{
    pxr::UsdGeomPointInstancer PointInstancer;
    return PointInstancer;
}

void ASimReadyStageActor::GetInstancedStaticMeshComponents(const pxr::UsdPrim& Prim, bool bResync, TArray<UHierarchicalInstancedStaticMeshComponent*>& OutInstancedStaticMeshComponents)
{
    if(Prim.GetPath().IsAbsoluteRootOrPrimPath())
    {
        if (Prim.IsA<pxr::UsdGeomMesh>())
        {
            auto PrototypeMesh = LoadStaticMeshComponent<UHierarchicalInstancedStaticMeshComponent, AActor>(Prim.GetPath(), bResync, true);
            if (PrototypeMesh)
            {
                OutInstancedStaticMeshComponents.Add(PrototypeMesh);
            }
        }
        else if (Prim.IsA<pxr::UsdGeomXformable>())
        {
            LoadSceneComponent(Prim.GetPath(), false, true, true);

            auto ChildPrims = Prim.GetFilteredChildren(pxr::UsdTraverseInstanceProxies());
            for (auto ChildPrim : ChildPrims)
            {
                GetInstancedStaticMeshComponents(ChildPrim, bResync, OutInstancedStaticMeshComponents);
            }
        }
    }
}

void ASimReadyStageActor::InitializePrototypes(const pxr::UsdPrim& Prim, const pxr::SdfPathVector& ProtoPaths)
{
    auto ChildPrims = Prim.GetFilteredChildren(pxr::UsdTraverseInstanceProxies());
    for (auto ChildPrim : ChildPrims)
    {
        // NOTE: this could be Scope
        if (ChildPrim.IsA<pxr::UsdGeomImageable>() && !ChildPrim.IsA<pxr::UsdGeomMesh>())
        {
            LoadSceneComponent(ChildPrim.GetPath(), false, true, true);

            bool bFoundProto = false;
            for (auto ProtoPath : ProtoPaths)
            {
                if (ProtoPath == ChildPrim.GetPath())
                {
                    bFoundProto = true;
                    break;
                }
            }
            if (!bFoundProto)
            {
                InitializePrototypes(ChildPrim, ProtoPaths);
            }
        }
    }
}

void ASimReadyStageActor::LoadPointInstancerMesh(const pxr::SdfPath& MeshPath, bool bResync)
{
    pxr::UsdPrim MeshPrim = GetUSDStage()->GetPrimAtPath(MeshPath);
    if (!MeshPrim)
    {
        return;
    }

    TArray<UHierarchicalInstancedStaticMeshComponent*> InstancedStaticMeshComps;
    GetInstancedStaticMeshComponents(MeshPrim, bResync, InstancedStaticMeshComps);

    if (InstancedStaticMeshComps.Num() == 0 || InstancedStaticMeshComps[0] == nullptr)
    {
        return;
    }

    // check instances number
    if (InstancedStaticMeshComps[0]->GetInstanceCount() == 0)
    {
        // need resync instances
        pxr::UsdPrim PointInstancerPrim;
        {
            pxr::SdfPath Path = MeshPath;
            do
            {
                Path = Path.GetParentPath();
                PointInstancerPrim = GetUSDStage()->GetPrimAtPath(Path);
            } while (PointInstancerPrim && !PointInstancerPrim.IsA<pxr::UsdGeomPointInstancer>());

            if (!PointInstancerPrim)
            {
                // invalid
                return;
            }
        }

        pxr::UsdGeomPointInstancer USDPointInstancer(PointInstancerPrim);
        pxr::UsdRelationship Relationship = USDPointInstancer.GetPrototypesRel();
        pxr::SdfPathVector ProtoPaths;
        Relationship.GetTargets(&ProtoPaths);

        int32 PrototypeIndex = INDEX_NONE;
        for (int32 ProtoIndex = 0; ProtoIndex < ProtoPaths.size(); ++ProtoIndex)
        {
            pxr::SdfPath Path = MeshPath;
            while(!Path.IsRootPrimPath())
            {
                if (Path == ProtoPaths[ProtoIndex])
                {
                    PrototypeIndex = ProtoIndex;
                    break;
                }
                Path = Path.GetParentPath();
            }

            if (PrototypeIndex != INDEX_NONE)
            {
                break;
            }
        }

        if (PrototypeIndex == INDEX_NONE)
        {
            return;
        }

        auto Orientations = GetUSDValue<pxr::VtArray<pxr::GfQuath>>(USDPointInstancer.GetOrientationsAttr());
        auto Positions = GetUSDValue<pxr::VtArray<pxr::GfVec3f>>(USDPointInstancer.GetPositionsAttr());
        auto Scales = GetUSDValue<pxr::VtArray<pxr::GfVec3f>>(USDPointInstancer.GetScalesAttr());
        auto ProtoIndices = GetUSDValue<pxr::VtArray<int>>(USDPointInstancer.GetProtoIndicesAttr());

        FBox BoundingBox(ForceInit);
        InstancedStaticMeshComps[0]->ClearInstances();
        for (int32 Index = 0; Index < ProtoIndices.size(); ++Index)
        {
            if (ProtoIndices[Index] == PrototypeIndex)
            {
                auto PrototypeComp = LoadSceneComponent(ProtoPaths[PrototypeIndex], false);
                if (PrototypeComp)
                {
                    FTransform RelativeToPrototypeTransform = InstancedStaticMeshComps[0]->GetComponentToWorld().GetRelativeTransform(PrototypeComp->GetComponentToWorld());
                    FTransform InstanceTransform(USDConvert(Orientations[Index]), USDConvert(Positions[Index]), USDConvert(Scales[Index]));
                    InstanceTransform = RHSTransformConvert(GetUSDStage(), InstanceTransform, true);
                    FTransform InstanceWorldTransform = RelativeToPrototypeTransform * InstanceTransform * PrototypeComp->GetComponentToWorld();
                    FTransform InstanceRelativeTransform = InstanceWorldTransform.GetRelativeTransform(InstancedStaticMeshComps[0]->GetComponentTransform());
                    InstancedStaticMeshComps[0]->AddInstance(InstanceRelativeTransform);
                    BoundingBox += InstanceTransform.GetLocation();
                }
            }
        }

        InstancedStaticMeshComps[0]->SetCullDistances(0.0f, FMath::Clamp(BoundingBox.GetExtent().Size(), MIN_CULL_DISTANCE, MAX_CULL_DISTANCE));
    }
}

void ASimReadyStageActor::LoadPointInstancer(const pxr::SdfPath& Path, bool bResync)
{
    pxr::UsdGeomPointInstancer USDPointInstancer = pxr::UsdGeomPointInstancer::Get(GetUSDStage(), Path.GetPrimPath());
    if(!USDPointInstancer)
    {
        return;
    }

    pxr::UsdRelationship Relationship = USDPointInstancer.GetPrototypesRel();
    pxr::SdfPathVector ProtoPaths;
    Relationship.GetTargets(&ProtoPaths);

    USceneComponent* InstancedMeshRoot = LoadSceneComponent(Path, false);
    if(Path.IsAbsoluteRootOrPrimPath())
    {
        InitializePrototypes(USDPointInstancer.GetPrim(), ProtoPaths);
    }

    auto Orientations = GetUSDValue<pxr::VtArray<pxr::GfQuath>>(USDPointInstancer.GetOrientationsAttr());
    auto Positions = GetUSDValue<pxr::VtArray<pxr::GfVec3f>>(USDPointInstancer.GetPositionsAttr());
    auto Scales = GetUSDValue<pxr::VtArray<pxr::GfVec3f>>(USDPointInstancer.GetScalesAttr());
    auto ProtoIndices = GetUSDValue<pxr::VtArray<int>>(USDPointInstancer.GetProtoIndicesAttr());

    TMap<int32, TArray<UHierarchicalInstancedStaticMeshComponent*>> InstancedStaticMeshMap;
    for(int32 ProtoIndex = 0; ProtoIndex < ProtoPaths.size(); ++ProtoIndex)
    {
        auto Prim = GetUSDStage()->GetPrimAtPath(ProtoPaths[ProtoIndex]);
        if (Prim)
        {
            TArray<UHierarchicalInstancedStaticMeshComponent*> InstancedStaticMeshComps;
            GetInstancedStaticMeshComponents(Prim, bResync, InstancedStaticMeshComps);
            InstancedStaticMeshMap.Add(ProtoIndex, InstancedStaticMeshComps);
        }
    }

    FBox BoundingBox(ForceInit);
    for (auto& Iter : InstancedStaticMeshMap)
    {
        for (auto& InstancedStaticMeshComp : Iter.Value)
        {
            InstancedStaticMeshComp->ClearInstances();
        }
    }

    for (int32 Index = 0; Index < ProtoIndices.size(); ++Index)
    {
        auto InstancedStaticMeshComps = InstancedStaticMeshMap.Find(ProtoIndices[Index]);
        auto PrototypeComp = LoadSceneComponent(ProtoPaths[ProtoIndices[Index]], false);

        if (InstancedStaticMeshComps && PrototypeComp)
        {
            for (auto InstancedStaticMeshComp : *InstancedStaticMeshComps)
            {
                // InstanceTransform is applied to prototype while InstancedMeshComponent is attated to the parent of mesh prim
                // So we should calculate a new instance transform for InstancedMeshComponent
                // 
                // Get relative transform from InstancedMeshComponent to Prototype component
                FTransform RelativeToPrototypeTransform = InstancedStaticMeshComp->GetComponentToWorld().GetRelativeTransform(PrototypeComp->GetComponentToWorld());

                // Get prototype instance transform
                FTransform InstanceTransform(USDConvert(Orientations[Index]), USDConvert(Positions[Index]), USDConvert(Scales[Index]));
                InstanceTransform = RHSTransformConvert(GetUSDStage(), InstanceTransform, true);

                // Calculate new instance transform
                // new_world_instance_transform = relative_to_prototype * instance_transform * prototype_world_transform
                FTransform InstanceWorldTransform = RelativeToPrototypeTransform * InstanceTransform * PrototypeComp->GetComponentToWorld();

                FTransform InstanceRelativeTransform = InstanceWorldTransform.GetRelativeTransform(InstancedStaticMeshComp->GetComponentTransform());
                InstancedStaticMeshComp->AddInstance(InstanceRelativeTransform);
                BoundingBox += InstanceTransform.GetLocation();
            }
        }
    }

    for (auto& Elem : InstancedStaticMeshMap)
    {
        TArray<UHierarchicalInstancedStaticMeshComponent*>& InstancedStaticMeshComps = Elem.Value;
        for (auto Comp : InstancedStaticMeshComps)
        {
            Comp->SetCullDistances(0.0f, FMath::Clamp(BoundingBox.GetExtent().Size(), MIN_CULL_DISTANCE, MAX_CULL_DISTANCE));
        }
    }
    
}