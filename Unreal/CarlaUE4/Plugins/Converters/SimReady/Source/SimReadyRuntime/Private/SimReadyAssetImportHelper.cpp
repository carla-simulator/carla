// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyAssetImportHelper.h"

#include "Animation/Skeleton.h"
#include "Modules/ModuleManager.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshLODImporterData.h"
#include "SimReadyMDL.h"

#include "Factories/FbxSkeletalMeshImportData.h"
#include "MeshUtilities.h"

USkeletalMesh* FSimReadyAssetImportHelper::CreateSkeletalMeshFromImportData(UPackage* Package, FName Name, EObjectFlags Flag, FSkeletalMeshImportData& SkelMeshImportData, const FString& SkelMeshName)
{
    USkeletalMesh* SkeletalMesh = NewObject<USkeletalMesh>(Package, Name, Flag);

    if (LoadSkeletalMeshFromImportData(SkeletalMesh, SkelMeshImportData, SkelMeshName))
    {
        return SkeletalMesh;
    }

    return nullptr;
}

bool FSimReadyAssetImportHelper::LoadSkeletalMeshFromImportData(USkeletalMesh* SkeletalMesh, FSkeletalMeshImportData& SkelMeshImportData, const FString& SkelMeshName)
{
    if (SkeletalMesh == nullptr)
    {
        return false;
    }

    // One-to-one mapping from import to raw. Needed for BuildSkeletalMesh
    SkelMeshImportData.PointToRawMap.AddUninitialized(SkelMeshImportData.Points.Num());
    for (int32 PointIndex = 0; PointIndex < SkelMeshImportData.Points.Num(); ++PointIndex)
    {
        SkelMeshImportData.PointToRawMap[PointIndex] = PointIndex;
    }

    // Create initial bounding box based on expanded version of reference pose for meshes without physics assets
    FBox BoundingBox(SkelMeshImportData.Points.GetData(), SkelMeshImportData.Points.Num());
    FBox Temp = BoundingBox;
    FVector MidMesh = 0.5f*(Temp.Min + Temp.Max);
    BoundingBox.Min = Temp.Min + 1.0f*(Temp.Min - MidMesh);
    BoundingBox.Max = Temp.Max + 1.0f*(Temp.Max - MidMesh);
    BoundingBox.Min[2] = Temp.Min[2] + 0.1f*(Temp.Min[2] - MidMesh[2]);
    const FVector BoundingBoxSize = BoundingBox.GetSize();

    if (SkelMeshImportData.Points.Num() > 2 && BoundingBoxSize.X < THRESH_POINTS_ARE_SAME && BoundingBoxSize.Y < THRESH_POINTS_ARE_SAME && BoundingBoxSize.Z < THRESH_POINTS_ARE_SAME)
    {
        return false;
    }

    SkeletalMesh->PreEditChange(nullptr);
    //Dirty the DDC Key for any imported Skeletal Mesh
    SkeletalMesh->InvalidateDeriveDataCacheGUID();

    FSkeletalMeshModel* ImportedModel = SkeletalMesh->GetImportedModel();
    ImportedModel->LODModels.Empty();
    ImportedModel->EmptyOriginalReductionSourceMeshData();
    ImportedModel->LODModels.Add(new FSkeletalMeshLODModel());
    FSkeletalMeshLODModel& LODModel = ImportedModel->LODModels[0];

    // Process materials from import data
    //ProcessImportMeshMaterials(SkeletalMesh->Materials, SkelMeshImportData);
    {
        SkeletalMesh->Materials.Empty();
        for (int32 MatIndex = 0; MatIndex < SkelMeshImportData.Materials.Num(); ++MatIndex)
        {
            const SkeletalMeshImportData::FMaterial& ImportedMaterial = SkelMeshImportData.Materials[MatIndex];
            UMaterialInterface* Material = ImportedMaterial.Material.Get() ? ImportedMaterial.Material.Get() : LoadObject<UMaterialInterface>(nullptr, *USimReadyMDL::GetContentPath(TEXT("/SimReady/DefaultMaterial")));
            FString MaterialNameNoSkin = ImportedMaterial.MaterialImportName;	
            SkeletalMesh->Materials.Add(FSkeletalMaterial(Material, true, false, FName(*ImportedMaterial.MaterialImportName), FName(*ImportedMaterial.MaterialImportName)));
        }
    }

    // Process reference skeleton from import data
    int32 SkeletalDepth = 0;
    if (!SkeletalMeshHelper::ProcessImportMeshSkeleton(nullptr, SkeletalMesh->RefSkeleton, SkeletalDepth, SkelMeshImportData))
    {
        return false;
    }

    // Process bones influence (normalization and optimization); this is not strictly needed for SkeletalMesh to work
    SkeletalMeshHelper::ProcessImportMeshInfluences(SkelMeshImportData, SkelMeshName);

    // Serialize the import data when needed
    //LODModel.RawSkeletalMeshBulkData.SaveRawMesh(SkelMeshImportData);

    SkeletalMesh->ResetLODInfo();
    FSkeletalMeshLODInfo& NewLODInfo = SkeletalMesh->AddLODInfo();
    NewLODInfo.ReductionSettings.NumOfTrianglesPercentage = 1.0f;
    NewLODInfo.ReductionSettings.NumOfVertPercentage = 1.0f;
    NewLODInfo.ReductionSettings.MaxDeviationPercentage = 0.0f;
    NewLODInfo.LODHysteresis = 0.02f;

    SkeletalMesh->SetImportedBounds(FBoxSphereBounds(BoundingBox));

    // Store whether or not this mesh has vertex colors
    SkeletalMesh->bHasVertexColors = SkelMeshImportData.bHasVertexColors;
    SkeletalMesh->VertexColorGuid = SkeletalMesh->bHasVertexColors ? FGuid::NewGuid() : FGuid();

    // Pass the number of texture coordinate sets to the LODModel.  Ensure there is at least one UV coord
    LODModel.NumTexCoords = FMath::Max<uint32>(1, SkelMeshImportData.NumTexCoords);

    // Create the render data
    {
        TArray<FVector> LODPoints;
        TArray<SkeletalMeshImportData::FMeshWedge> LODWedges;
        TArray<SkeletalMeshImportData::FMeshFace> LODFaces;
        TArray<SkeletalMeshImportData::FVertInfluence> LODInfluences;
        TArray<int32> LODPointToRawMap;
        SkelMeshImportData.CopyLODImportData(LODPoints, LODWedges, LODFaces, LODInfluences, LODPointToRawMap);

        IMeshUtilities::MeshBuildOptions BuildOptions;
        // #ueent_todo: Normals and tangents shouldn't need to be recomputed when they are retrieved from USD
        BuildOptions.bComputeNormals = !SkelMeshImportData.bHasNormals;
        BuildOptions.bComputeTangents = !SkelMeshImportData.bHasTangents;

        IMeshUtilities& MeshUtilities = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");

        TArray<FText> WarningMessages;
        TArray<FName> WarningNames;

        bool bBuildSuccess = MeshUtilities.BuildSkeletalMesh(ImportedModel->LODModels[0], SkelMeshName, SkeletalMesh->RefSkeleton, LODInfluences, LODWedges, LODFaces, LODPoints, LODPointToRawMap, BuildOptions, &WarningMessages, &WarningNames);
        if (!bBuildSuccess)
        {
            return false;
        }

        SkeletalMesh->CalculateInvRefMatrices();
        SkeletalMesh->PostEditChange();
    }

    if (SkeletalMesh->RefSkeleton.GetRawBoneNum() == 0)
    {
        return false;
    }

    return true;
}
