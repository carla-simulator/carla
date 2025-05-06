// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BlueprintLibrary/CarlaToolsFunctionLibrary.h"

#include <util/ue-header-guard-begin.h>
#include "AssetRegistry/AssetRegistryModule.h"

#include "AssetToolsModule.h"
#include "Materials/MaterialInstance.h"
#include "StaticMeshAttributes.h"
#include "RenderingThread.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/SavePackage.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshOperations.h"
#include "Modules/ModuleManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Editor/Transactor.h"
#endif
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include <util/ue-header-guard-end.h>

DEFINE_LOG_CATEGORY(LogCarlaToolsFunctionLibrary);

void UCarlaToolsFunctionLibrary::SubdivideTriangles(const TArray<UCarlaToolsFunctionLibrary::FTriangle> &InTriangles, int Factor, TArray<UCarlaToolsFunctionLibrary::FTriangle> &OutTriangles)
{
  if (Factor <= 1)
  {
    OutTriangles = InTriangles;
    return;
  }

  TArray<UCarlaToolsFunctionLibrary::FTriangle> Working = InTriangles;

  for (int f = 1; f < Factor; ++f)
  {
    TArray<UCarlaToolsFunctionLibrary::FTriangle> Next;
    for (const UCarlaToolsFunctionLibrary::FTriangle &T : Working)
    {
      FVector3f A = T.V0;
      FVector3f B = T.V1;
      FVector3f C = T.V2;

      FVector3f AB = (A + B) * 0.5f;
      FVector3f BC = (B + C) * 0.5f;
      FVector3f CA = (C + A) * 0.5f;

      Next.Add({A, AB, CA});
      Next.Add({AB, B, BC});
      Next.Add({CA, BC, C});
      Next.Add({AB, BC, CA});
    }
    Working = Next;
  }

  OutTriangles = Working;
}

void UCarlaToolsFunctionLibrary::ChunkAndSubdivideStaticMesh(UStaticMesh *SourceMesh, FString MeshName, int ChunkSize, FString OutputFolder)
{
  if (!SourceMesh || !SourceMesh->GetRenderData())
    return;

  const FStaticMeshLODResources &LOD = SourceMesh->GetRenderData()->LODResources[0];
  const FPositionVertexBuffer &PositionBuffer = LOD.VertexBuffers.PositionVertexBuffer;
  const FStaticMeshVertexBuffer &VertexBuffer = LOD.VertexBuffers.StaticMeshVertexBuffer;
  const FIndexArrayView IndexArray = LOD.IndexBuffer.GetArrayView();

  TArray<FTriangle> AllTriangles;
  TArray<int32> TriangleMaterialIndices;
  TArray<FIntVector> TriangleVertexIndices;

  // Extract triangles and their material indices
  for (int32 SectionIndex = 0; SectionIndex < LOD.Sections.Num(); ++SectionIndex)
  {
    const FStaticMeshSection &Section = LOD.Sections[SectionIndex];
    for (uint32 i = 0; i < Section.NumTriangles; ++i)
    {
      int32 Index0 = IndexArray[Section.FirstIndex + i * 3 + 0];
      int32 Index1 = IndexArray[Section.FirstIndex + i * 3 + 1];
      int32 Index2 = IndexArray[Section.FirstIndex + i * 3 + 2];

      FVector3f V0 = PositionBuffer.VertexPosition(Index0);
      FVector3f V1 = PositionBuffer.VertexPosition(Index1);
      FVector3f V2 = PositionBuffer.VertexPosition(Index2);

      AllTriangles.Add({V0, V1, V2});
      TriangleMaterialIndices.Add(Section.MaterialIndex);
      TriangleVertexIndices.Add(FIntVector(Index0, Index1, Index2));
    }
  }

  // Group triangles by region and material index
  TMap<FIntPoint, TMap<int32, TArray<int32>>> RegionMaterialTriangleIndices;

  for (int32 i = 0; i < AllTriangles.Num(); ++i)
  {
    const auto &T = AllTriangles[i];
    FVector3f Center = (T.V0 + T.V1 + T.V2) / 3.0f;
    FIntPoint Region(FMath::FloorToInt(Center.X / ChunkSize), FMath::FloorToInt(Center.Y / ChunkSize));
    int32 MaterialIndex = TriangleMaterialIndices[i];

    RegionMaterialTriangleIndices.FindOrAdd(Region).FindOrAdd(MaterialIndex).Add(i);
  }

  FAssetToolsModule &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
  int32 ChunkIndex = 0;

  // Process each chunk
  for (const auto &RegionPair : RegionMaterialTriangleIndices)
  {
    if (RegionPair.Value.Num() == 0)
      continue;

    FMeshDescription MeshDesc;
    FStaticMeshAttributes Attributes(MeshDesc);
    Attributes.Register();
    Attributes.GetVertexInstanceNormals();
    Attributes.GetVertexInstanceTangents();
    Attributes.GetVertexInstanceBinormalSigns();
    Attributes.GetVertexInstanceUVs().SetNumChannels(1);

    TMap<int32, FVertexID> VertexMap;
    TMap<int32, FPolygonGroupID> PolygonGroups;

    for (const auto &MatPair : RegionPair.Value)
    {
      int32 MaterialIndex = MatPair.Key;
      const auto &TriangleIndices = MatPair.Value;

      FPolygonGroupID PGID = MeshDesc.CreatePolygonGroup();
      PolygonGroups.Add(MaterialIndex, PGID);

      for (int32 TriIdx : TriangleIndices)
      {
        const FTriangle &T = AllTriangles[TriIdx];
        const FIntVector &Indices = TriangleVertexIndices[TriIdx];

        FVertexID VIDs[3];

        for (int i = 0; i < 3; ++i)
        {
          int32 OriginalIndex = (i == 0 ? Indices.X : (i == 1 ? Indices.Y : Indices.Z));
          if (!VertexMap.Contains(OriginalIndex))
          {
            FVertexID VID = MeshDesc.CreateVertex();
            Attributes.GetVertexPositions()[VID] = PositionBuffer.VertexPosition(OriginalIndex);
            VertexMap.Add(OriginalIndex, VID);
          }
          VIDs[i] = VertexMap[OriginalIndex];
        }

        // Create VertexInstances and assign UVs
        TArray<FVertexInstanceID> VInsts;
        for (int i = 0; i < 3; ++i)
        {
          int32 OriginalIndex = (i == 0 ? Indices.X : (i == 1 ? Indices.Y : Indices.Z));
          FVertexInstanceID InstanceID = MeshDesc.CreateVertexInstance(VIDs[i]);
          VInsts.Add(InstanceID);

          FVector2f UV = VertexBuffer.GetVertexUV(OriginalIndex, 0);
          Attributes.GetVertexInstanceUVs().Set(InstanceID, 0, UV);
        }

        MeshDesc.CreatePolygon(PGID, VInsts);
      }
    }

    // Compute normals/tangents
    FStaticMeshOperations::ComputeTriangleTangentsAndNormals(MeshDesc, 0.0001f);
    FStaticMeshOperations::ComputeTangentsAndNormals(MeshDesc, EComputeNTBsFlags::Normals | EComputeNTBsFlags::Tangents);

    // Create and save new mesh
    FString AssetName = FString::Printf(TEXT("%s_%d"), *MeshName, ChunkIndex++);
    FString PackagePath = OutputFolder + "/" + AssetName;
    UPackage *Package = CreatePackage(*PackagePath);

    UStaticMesh *NewMesh = NewObject<UStaticMesh>(Package, *AssetName, RF_Public | RF_Standalone);
    NewMesh->InitResources();
    NewMesh->SetNumSourceModels(1);
    NewMesh->CreateMeshDescription(0, MoveTemp(MeshDesc));
    NewMesh->CommitMeshDescription(0);

    // Set materials
    for (const auto &PG : PolygonGroups)
    {
      if (SourceMesh->GetStaticMaterials().IsValidIndex(PG.Key))
      {
        NewMesh->GetStaticMaterials().Add(SourceMesh->GetStaticMaterials()[PG.Key]);
      }
      else
      {
        NewMesh->GetStaticMaterials().Add(FStaticMaterial());
      }
    }

    NewMesh->Build(false);
    NewMesh->PostEditChange();
    NewMesh->MarkPackageDirty();

    FAssetRegistryModule::AssetCreated(NewMesh);
    FString Filename = FPackageName::LongPackageNameToFilename(PackagePath, TEXT(".uasset"));
    UPackage::SavePackage(Package, NewMesh, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *Filename);
  }
}

void UCarlaToolsFunctionLibrary::SplitStaticMeshByMaterial(UStaticMesh *SourceMesh, FString BaseName, FString OutputFolder, int SubdivisionFactor)
{
  if (!SourceMesh || !SourceMesh->GetRenderData())
    return;

  const FStaticMeshLODResources &LOD = SourceMesh->GetRenderData()->LODResources[0];
  const FPositionVertexBuffer &PositionBuffer = LOD.VertexBuffers.PositionVertexBuffer;
  const FIndexArrayView Indices = LOD.IndexBuffer.GetArrayView();

  FAssetToolsModule &AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

  for (int32 SectionIndex = 0; SectionIndex < LOD.Sections.Num(); ++SectionIndex)
  {
    const FStaticMeshSection &Section = LOD.Sections[SectionIndex];
    int32 MaterialIndex = Section.MaterialIndex;

    TArray<FTriangle> Triangles;

    for (uint32 i = 0; i < Section.NumTriangles; ++i)
    {
      int32 Index0 = Indices[Section.FirstIndex + i * 3 + 0];
      int32 Index1 = Indices[Section.FirstIndex + i * 3 + 1];
      int32 Index2 = Indices[Section.FirstIndex + i * 3 + 2];

      Triangles.Add({PositionBuffer.VertexPosition(Index0),
                     PositionBuffer.VertexPosition(Index1),
                     PositionBuffer.VertexPosition(Index2)});
    }

    TArray<FTriangle> Subdivided;
    SubdivideTriangles(Triangles, SubdivisionFactor, Subdivided);

    FMeshDescription MeshDesc;
    FStaticMeshAttributes Attributes(MeshDesc);
    Attributes.Register();
    Attributes.GetVertexInstanceNormals();
    Attributes.GetVertexInstanceTangents();
    Attributes.GetVertexInstanceBinormalSigns();
    Attributes.GetVertexInstanceUVs().SetNumChannels(1);

    TMap<FVector3f, FVertexID> VertexMap;
    FPolygonGroupID PolyGroupID = MeshDesc.CreatePolygonGroup();

    for (const FTriangle &T : Subdivided)
    {
      FVertexID VIDs[3];
      FVector3f Verts[3] = {T.V0, T.V1, T.V2};

      for (int i = 0; i < 3; ++i)
      {
        if (!VertexMap.Contains(Verts[i]))
        {
          FVertexID VID = MeshDesc.CreateVertex();
          Attributes.GetVertexPositions()[VID] = Verts[i];
          VertexMap.Add(Verts[i], VID);
        }
        VIDs[i] = VertexMap[Verts[i]];
      }

      TArray<FVertexInstanceID> VInsts;
      for (int i = 0; i < 3; ++i)
      {
        FVertexInstanceID InstanceID = MeshDesc.CreateVertexInstance(VIDs[i]);
        VInsts.Add(InstanceID);
        Attributes.GetVertexInstanceUVs().Set(InstanceID, 0, FVector2f(Verts[i].X * 0.01f, Verts[i].Y * 0.01f));
      }

      MeshDesc.CreatePolygon(PolyGroupID, VInsts);
    }

    FStaticMeshOperations::ComputeTriangleTangentsAndNormals(MeshDesc, 0.0001f);
    FStaticMeshOperations::ComputeTangentsAndNormals(MeshDesc, EComputeNTBsFlags::Normals | EComputeNTBsFlags::Tangents);

    FString AssetName = FString::Printf(TEXT("%s_Mat%d"), *BaseName, SectionIndex);
    FString PackagePath = OutputFolder + "/" + AssetName;
    UPackage *Package = CreatePackage(*PackagePath);

    UStaticMesh *NewMesh = NewObject<UStaticMesh>(Package, *AssetName, RF_Public | RF_Standalone);
    NewMesh->InitResources();
    NewMesh->SetNumSourceModels(1);
    NewMesh->CreateMeshDescription(0, MoveTemp(MeshDesc));
    NewMesh->CommitMeshDescription(0);

    if (SourceMesh->GetStaticMaterials().IsValidIndex(MaterialIndex))
    {
      NewMesh->GetStaticMaterials().Add(SourceMesh->GetStaticMaterials()[MaterialIndex]);
    }

    NewMesh->Build(false);
    NewMesh->PostEditChange();
    NewMesh->MarkPackageDirty();

    FAssetRegistryModule::AssetCreated(NewMesh);
    FString Filename = FPackageName::LongPackageNameToFilename(PackagePath, TEXT(".uasset"));
    UPackage::SavePackage(Package, NewMesh, EObjectFlags::RF_Public | RF_Standalone, *Filename);
  }
}
