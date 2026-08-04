// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "OpenDriveGenerator.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Game/Tagger.h"
#include "Carla.h"
#include "Traffic/TrafficLightManager.h"
#include "Util/ProceduralCustomMesh.h"
#include "Materials/MaterialInterface.h"
#include "PCGComponent.h"
#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "PCGGraph.h"

#include <util/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <util/enable-ue4-macros.h>

// #include "Engine/Classes/Interfaces/Interface_CollisionDataProvider.h"
// #include "PhysicsCore/Public/BodySetupEnums.h"

namespace crp = carla::rpc;

namespace {

  /// Tags a generated procedural-mesh component for semantic segmentation
  /// and CPU-side tag queries. ATagger::TagActor (Game/Tagger.cpp) only
  /// walks UStaticMeshComponent/USkeletalMeshComponent, inferring the label
  /// from the mesh asset's content folder, so runtime-generated
  /// UProceduralMeshComponents were left with unset/uninitialized
  /// CustomPrimitiveData (garbage semantic-segmentation labels). Here the
  /// label is already known from the OpenDRIVE material name, so this
  /// replicates ATagger's exact per-component tagging steps directly
  /// instead of going through the asset-path inference path.
  void TagGeneratedComponent(UPrimitiveComponent &Component, uint32 ActorId, crp::CityObjectLabel Label)
  {
    ATagger::SetStencilValue(Component, ActorId, Label, true);
    Component.ComponentTags.Add(FName(*ATagger::GetTagAsString(Label)));
  }

  /// Appends Src into Dst, rebasing Src's triangle indices by Dst's current
  /// vertex count so the combined buffers stay a single valid mesh.
  void AppendProceduralMesh(FProceduralCustomMesh &Dst, const FProceduralCustomMesh &Src)
  {
    const int32 VertexOffset = Dst.Vertices.Num();
    Dst.Vertices.Append(Src.Vertices);
    Dst.Normals.Append(Src.Normals);
    Dst.UV0.Append(Src.UV0);
    Dst.Triangles.Reserve(Dst.Triangles.Num() + Src.Triangles.Num());
    for (const int32 Index : Src.Triangles)
    {
      Dst.Triangles.Add(Index + VertexOffset);
    }
  }

  /// Extracts the sub-mesh covering a single material range
  /// ([Range.index_start, Range.index_end) into Mesh.GetIndexes()) from a
  /// full carla::geom::Mesh, converting to Unreal space/units and rebasing
  /// indices into a self-contained, densely-packed vertex/index buffer.
  /// Mirrors the m->cm scale, the "-1" rebase (carla::geom::Mesh indices are
  /// 1-based) and the (i, i+2, i+1) winding flip that
  /// carla::geom::Mesh::operator FProceduralCustomMesh() applies to the
  /// whole mesh (LibCarla/source/carla/geom/Mesh.h:162-227), scoped down to
  /// only the triangles belonging to this material.
  FProceduralCustomMesh ExtractMaterialRange(
      const carla::geom::Mesh &Mesh,
      const carla::geom::MeshMaterial &Range,
      bool bSourceHasPerVertexUVs)
  {
    FProceduralCustomMesh Result;

    const auto &Vertices = Mesh.GetVertices();
    const auto &Indexes = Mesh.GetIndexes();
    const auto &UVs = Mesh.GetUVs();

    if (Range.index_end <= Range.index_start || Range.index_end > Indexes.size())
    {
      return Result;
    }

    TMap<size_t, int32> GlobalToLocal;
    GlobalToLocal.Reserve(static_cast<int32>(Range.index_end - Range.index_start));

    for (size_t i = Range.index_start; i < Range.index_end; i += 3)
    {
      int32 LocalIdx[3];
      for (int32 k = 0; k < 3; ++k)
      {
        const size_t GlobalIdx = Indexes[i + k];
        if (const int32 *Found = GlobalToLocal.Find(GlobalIdx))
        {
          LocalIdx[k] = *Found;
        }
        else
        {
          // carla::geom::Mesh indices are 1-based.
          const size_t VertexIdx = GlobalIdx - 1;
          const auto &V = Vertices[VertexIdx];
          const int32 NewLocalIdx = Result.Vertices.Add(
              FVector{1e2f * V.x, 1e2f * V.y, 1e2f * V.z});
          if (bSourceHasPerVertexUVs && VertexIdx < UVs.size())
          {
            Result.UV0.SetNum(NewLocalIdx + 1);
            Result.UV0[NewLocalIdx] = FVector2D{UVs[VertexIdx].x, UVs[VertexIdx].y};
          }
          GlobalToLocal.Add(GlobalIdx, NewLocalIdx);
          LocalIdx[k] = NewLocalIdx;
        }
      }
      // Unreal is left-handed; flip the last two indices, same as the
      // full-mesh conversion.
      Result.Triangles.Add(LocalIdx[0]);
      Result.Triangles.Add(LocalIdx[2]);
      Result.Triangles.Add(LocalIdx[1]);
    }

    // Recompute per-vertex normals over the extracted sub-mesh only, using
    // the same up-biased face-normal averaging as the full-mesh conversion.
    Result.Normals.Init(FVector::UpVector, Result.Vertices.Num());
    for (int32 i = 0; i < Result.Triangles.Num(); i += 3)
    {
      const FVector &V0 = Result.Vertices[Result.Triangles[i]];
      const FVector &V1 = Result.Vertices[Result.Triangles[i + 1]];
      const FVector &V2 = Result.Vertices[Result.Triangles[i + 2]];
      const FVector U = V1 - V0;
      const FVector V = V2 - V0;
      FVector Normal;
      Normal.X = (U.Y * V.Z) - (U.Z * V.Y);
      Normal.Y = (U.Z * V.X) - (U.X * V.Z);
      Normal.Z = (U.X * V.Y) - (U.Y * V.X);
      Normal = -Normal;
      Normal = Normal.GetSafeNormal(.0001f);
      if (Normal != FVector::ZeroVector)
      {
        if (FVector::DotProduct(Normal, FVector(0, 0, 1)) < 0)
        {
          Normal = -Normal;
        }
        Result.Normals[Result.Triangles[i]] = Normal;
        Result.Normals[Result.Triangles[i + 1]] = Normal;
        Result.Normals[Result.Triangles[i + 2]] = Normal;
      }
    }

    return Result;
  }

} // namespace

ACarlaProceduralMeshActor::ACarlaProceduralMeshActor()
{
  PrimaryActorTick.bCanEverTick = false;
  MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RootComponent"));
  RootComponent = MeshComponent;
}

AProceduralFurnitureAnchor::AProceduralFurnitureAnchor()
{
  PrimaryActorTick.bCanEverTick = false;
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

AOpenDriveGenerator::AOpenDriveGenerator(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
  SetRootComponent(RootComponent);
  RootComponent->Mobility = EComponentMobility::Static;

  // Non-partitioned (default bIsComponentPartitioned = false) so this works
  // on OpenDriveMap.umap's legacy non-World-Partition level with no
  // PCGWorldActor required. Generation is triggered explicitly from
  // GeneratePoles() once all furniture anchors exist, not on load/runtime
  // streaming -- see StreetFurnitureGraph's doc comment.
  PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
  PCGBoundsComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("PCGBoundsComponent"));
  PCGBoundsComponent->SetupAttachment(RootComponent);
  PCGBoundsComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  PCGBoundsComponent->SetGenerateOverlapEvents(false);
}

bool AOpenDriveGenerator::LoadOpenDrive(const FString &OpenDrive)
{
  using OpenDriveLoader = carla::opendrive::OpenDriveParser;

  if (OpenDrive.IsEmpty())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive is empty"));
    return false;
  }

  OpenDriveData = OpenDrive;
  return true;
}

const FString &AOpenDriveGenerator::GetOpenDrive() const
{
  return OpenDriveData;
}

bool AOpenDriveGenerator::IsOpenDriveValid() const
{
  return UCarlaStatics::GetGameMode(GetWorld())->GetMap().has_value();
}

void AOpenDriveGenerator::GenerateRoadMesh()
{
  if (!IsOpenDriveValid())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive has not been loaded"));
    return;
  }

  carla::rpc::OpendriveGenerationParameters Parameters;
  UCarlaGameInstance * GameInstance = UCarlaStatics::GetGameInstance(GetWorld());
  if(GameInstance)
  {
    Parameters = GameInstance->GetOpendriveGenerationParameters();
  }
  else
  {
    carla::log_warning("Missing game instance");
  }

  auto& CarlaMap = UCarlaStatics::GetGameMode(GetWorld())->GetMap();
  const auto Meshes = CarlaMap->GenerateChunkedMesh(Parameters);

  UMaterialInterface* ResolvedRoadMaterial = RoadMaterial.LoadSynchronous();
  UMaterialInterface* ResolvedSidewalkMaterial = SidewalkMaterial.LoadSynchronous();

  // Reset so re-running GenerateRoadMesh (e.g. regenerating the map) doesn't
  // leave GenerateGroundPlane sizing the fill heightfield off stale geometry.
  RoadMeshBounds = FBox(ForceInit);
  GroundHeightSampleGrid.Reset();
  const float SampleCellSize = FMath::Max(GroundHeightSampleCellSize, 1.0f);

  for (const auto &Mesh : Meshes) {
    if (!Mesh->GetVertices().size())
    {
      continue;
    }

    // Split the chunk into one actor per OpenDRIVE material tag ("road",
    // "sidewalk") instead of dumping everything into a single actor's
    // section 0 with no material assigned, which made every generated map
    // render flat grey. This also gives semantic segmentation a single,
    // unambiguous label per component to tag: CustomPrimitiveData (read by
    // TagGeneratedComponent below) is set per UPrimitiveComponent, not per
    // mesh section, so a chunk mixing road+sidewalk geometry in one
    // component could only ever be tagged as one of the two. Ranges sharing
    // the same material name (e.g. several lanes merged into one junction
    // mesh) are combined before spawning, to keep the actor/draw-call count
    // down.
    const auto &MaterialRanges = Mesh->GetMaterials();
    const bool bHasPerVertexUVs = Mesh->GetUVs().size() == Mesh->GetVertices().size();

    TArray<FString> SectionOrder;
    TMap<FString, FProceduralCustomMesh> SectionsByMaterial;

    if (MaterialRanges.empty())
    {
      // No material tags on this chunk (shouldn't normally happen --
      // MeshFactory always tags "road"/"sidewalk"); fall back to treating
      // the whole chunk as "road" rather than dropping the geometry.
      SectionOrder.Add(TEXT("road"));
      SectionsByMaterial.Add(TEXT("road"), *Mesh);
    }
    else
    {
      for (const auto &MatRange : MaterialRanges)
      {
        FProceduralCustomMesh Extracted = ExtractMaterialRange(*Mesh, MatRange, bHasPerVertexUVs);
        if (Extracted.Vertices.Num() == 0)
        {
          continue;
        }
        const FString MatName(MatRange.name.c_str());
        if (FProceduralCustomMesh *Existing = SectionsByMaterial.Find(MatName))
        {
          AppendProceduralMesh(*Existing, Extracted);
        }
        else
        {
          SectionOrder.Add(MatName);
          SectionsByMaterial.Add(MatName, MoveTemp(Extracted));
        }
      }
    }

    for (const FString &MatName : SectionOrder)
    {
      const FProceduralCustomMesh &SectionMesh = SectionsByMaterial[MatName];
      const bool bIsSidewalk = (MatName == TEXT("sidewalk"));

      // Feeds GenerateGroundPlane, which must run after this loop.
      for (const FVector &Vertex : SectionMesh.Vertices)
      {
        RoadMeshBounds += Vertex;
        const FIntPoint Cell(
            FMath::FloorToInt(Vertex.X / SampleCellSize),
            FMath::FloorToInt(Vertex.Y / SampleCellSize));
        if (float *ExistingZ = GroundHeightSampleGrid.Find(Cell))
        {
          *ExistingZ = FMath::Min(*ExistingZ, static_cast<float>(Vertex.Z));
        }
        else
        {
          GroundHeightSampleGrid.Add(Cell, Vertex.Z);
        }
      }

      ACarlaProceduralMeshActor* TempActor = GetWorld()->SpawnActor<ACarlaProceduralMeshActor>();
      UProceduralMeshComponent *TempPMC = TempActor->MeshComponent;
      TempPMC->bUseAsyncCooking = true;
      TempPMC->bUseComplexAsSimpleCollision = true;
      TempPMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

      TempPMC->CreateMeshSection_LinearColor(
          0,
          SectionMesh.Vertices,
          SectionMesh.Triangles,
          SectionMesh.Normals,
          SectionMesh.UV0,
          TArray<FLinearColor>(), // VertexColor
          TArray<FProcMeshTangent>(), // Tangents
          true); // Create collision

      UMaterialInterface* SectionMaterial = bIsSidewalk ? ResolvedSidewalkMaterial : ResolvedRoadMaterial;
      if (SectionMaterial)
      {
        TempPMC->SetMaterial(0, SectionMaterial);
      }

      TagGeneratedComponent(
          *TempPMC,
          TempActor->GetUniqueID(),
          bIsSidewalk ? crp::CityObjectLabel::Sidewalks : crp::CityObjectLabel::Roads);

      ActorMeshList.Add(TempActor);
    }
  }

  if(!Parameters.enable_mesh_visibility)
  {
    for(AActor * actor : ActorMeshList)
    {
      actor->SetActorHiddenInGame(true);
    }
  }

  // // Build collision data
  // FTriMeshCollisionData CollisitonData;
  // CollisitonData.bDeformableMesh = false;
  // CollisitonData.bDisableActiveEdgePrecompute = false;
  // CollisitonData.bFastCook = false;
  // CollisitonData.bFlipNormals = false;
  // CollisitonData.Indices = TriIndices;
  // CollisitonData.Vertices = Vertices;

  // RoadMesh->ContainsPhysicsTriMeshData(true);
  // bool Success = RoadMesh->GetPhysicsTriMeshData(&CollisitonData, true);
  // if (!Success)
  // {
  //   UE_LOG(LogCarla, Error, TEXT("The road collision mesh could not be generated!"));
  // }
}

void AOpenDriveGenerator::GenerateGroundPlane()
{
  if (!IsOpenDriveValid())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive has not been loaded"));
    return;
  }

  if (!RoadMeshBounds.IsValid)
  {
    // GenerateRoadMesh produced no geometry to size the heightfield against
    // (not called yet, or the OpenDRIVE has no road mesh).
    UE_LOG(LogCarla, Warning, TEXT("AOpenDriveGenerator: RoadMeshBounds is empty, skipping ground plane"));
    return;
  }

  carla::rpc::OpendriveGenerationParameters Parameters;
  UCarlaGameInstance * GameInstance = UCarlaStatics::GetGameInstance(GetWorld());
  if (GameInstance)
  {
    Parameters = GameInstance->GetOpendriveGenerationParameters();
  }

  // A heightfield grid spanning the padded road-network footprint. Each
  // grid vertex sits GroundPlaneZOffset below the road height interpolated
  // from GroundHeightSampleGrid (min road/sidewalk z per sample cell,
  // accumulated in GenerateRoadMesh), so the ground follows the road
  // network's real elevation profile. A flat quad at global min-z closed
  // the blue-sky holes on CARLA's flat towns, but real-elevation OpenDRIVE
  // (NuRec: Stuttgart z~293m, graded motorways) left every uphill road
  // flying tens of meters above it.
  const FBox PaddedBounds = RoadMeshBounds.ExpandBy(FVector(GroundPlanePadding, GroundPlanePadding, 0.0f));
  const float MinX = PaddedBounds.Min.X;
  const float MaxX = PaddedBounds.Max.X;
  const float MinY = PaddedBounds.Min.Y;
  const float MaxY = PaddedBounds.Max.Y;

  // Cap the vertex budget: if the padded map is large enough that the
  // configured spacing would blow past it, widen the spacing instead of
  // failing or stalling the load with a multi-million-vertex cook.
  constexpr int32 MaxGridVerticesPerAxis = 512;
  float GridStep = FMath::Max(GroundGridCellSize, 100.0f);
  GridStep = FMath::Max3(
      GridStep,
      (MaxX - MinX) / static_cast<float>(MaxGridVerticesPerAxis - 1),
      (MaxY - MinY) / static_cast<float>(MaxGridVerticesPerAxis - 1));
  const int32 NumX = FMath::Clamp(FMath::CeilToInt((MaxX - MinX) / GridStep), 1, MaxGridVerticesPerAxis - 1) + 1;
  const int32 NumY = FMath::Clamp(FMath::CeilToInt((MaxY - MinY) / GridStep), 1, MaxGridVerticesPerAxis - 1) + 1;
  const float StepX = (MaxX - MinX) / static_cast<float>(NumX - 1);
  const float StepY = (MaxY - MinY) / static_cast<float>(NumY - 1);

  const float SampleCellSize = FMath::Max(GroundHeightSampleCellSize, 1.0f);
  const float FallbackZ = RoadMeshBounds.Min.Z;

  // Inverse-distance interpolation over the road height samples, searching
  // outward ring by ring in the sample grid from the vertex's cell. One
  // extra ring past the first hit keeps the blend smooth where two roads at
  // different heights face each other; a vertex out in the padding just
  // inherits the nearest shoulder's height (flat extrapolation), which is
  // exactly what the fill skirt should do. Ring cap covers the padding plus
  // slack so the search always terminates even off in a map corner.
  const int32 MaxRing = FMath::CeilToInt((GroundPlanePadding + 2.0f * GridStep) / SampleCellSize) + 2;
  const auto SampleGroundHeight = [&](float X, float Y) -> TOptional<float>
  {
    const int32 CenterCellX = FMath::FloorToInt(X / SampleCellSize);
    const int32 CenterCellY = FMath::FloorToInt(Y / SampleCellSize);
    float WeightSum = 0.0f;
    float HeightSum = 0.0f;
    int32 FirstHitRing = -1;
    for (int32 Ring = 0; Ring <= MaxRing; ++Ring)
    {
      if (FirstHitRing >= 0 && Ring > FirstHitRing + 1)
      {
        break;
      }
      bool bHit = false;
      const auto VisitCell = [&](int32 CellX, int32 CellY)
      {
        const float *SampleZ = GroundHeightSampleGrid.Find(FIntPoint(CellX, CellY));
        if (!SampleZ)
        {
          return;
        }
        const float CellCenterX = (static_cast<float>(CellX) + 0.5f) * SampleCellSize;
        const float CellCenterY = (static_cast<float>(CellY) + 0.5f) * SampleCellSize;
        const float DistSq = FMath::Square(X - CellCenterX) + FMath::Square(Y - CellCenterY);
        const float Weight = 1.0f / (DistSq + FMath::Square(SampleCellSize * 0.25f));
        WeightSum += Weight;
        HeightSum += Weight * (*SampleZ);
        bHit = true;
      };
      if (Ring == 0)
      {
        VisitCell(CenterCellX, CenterCellY);
      }
      else
      {
        for (int32 DX = -Ring; DX <= Ring; ++DX)
        {
          VisitCell(CenterCellX + DX, CenterCellY - Ring);
          VisitCell(CenterCellX + DX, CenterCellY + Ring);
        }
        for (int32 DY = -Ring + 1; DY <= Ring - 1; ++DY)
        {
          VisitCell(CenterCellX - Ring, CenterCellY + DY);
          VisitCell(CenterCellX + Ring, CenterCellY + DY);
        }
      }
      if (bHit && FirstHitRing < 0)
      {
        FirstHitRing = Ring;
      }
    }
    return WeightSum > 0.0f ? TOptional<float>(HeightSum / WeightSum) : TOptional<float>();
  };

  // First pass: direct interpolation where road samples are within ring
  // reach. Vertices deeper inside road-free voids (a rural map can leave
  // interior gaps wider than the ring cap) stay unresolved here and get
  // filled below instead of snapping to a global min-z cliff.
  TArray<float> Heights;
  TArray<bool> Resolved;
  Heights.SetNumUninitialized(NumX * NumY);
  Resolved.SetNumUninitialized(NumX * NumY);
  int32 NumUnresolved = 0;
  for (int32 IY = 0; IY < NumY; ++IY)
  {
    const float Y = MinY + static_cast<float>(IY) * StepY;
    for (int32 IX = 0; IX < NumX; ++IX)
    {
      const float X = MinX + static_cast<float>(IX) * StepX;
      const TOptional<float> Sampled = SampleGroundHeight(X, Y);
      const int32 Index = IY * NumX + IX;
      Resolved[Index] = Sampled.IsSet();
      Heights[Index] = Sampled.Get(FallbackZ);
      NumUnresolved += Sampled.IsSet() ? 0 : 1;
    }
  }

  // Fill pass: dilate resolved heights into unresolved vertices, each
  // taking the average of its resolved 4-neighbours, until the whole grid
  // is covered. Bounded by the grid diagonal, so it always terminates.
  for (int32 Pass = 0; NumUnresolved > 0 && Pass < NumX + NumY; ++Pass)
  {
    TArray<TPair<int32, float>> Filled;
    for (int32 IY = 0; IY < NumY; ++IY)
    {
      for (int32 IX = 0; IX < NumX; ++IX)
      {
        const int32 Index = IY * NumX + IX;
        if (Resolved[Index])
        {
          continue;
        }
        float NeighbourSum = 0.0f;
        int32 NeighbourCount = 0;
        const auto VisitNeighbour = [&](int32 NX, int32 NY)
        {
          if (NX < 0 || NX >= NumX || NY < 0 || NY >= NumY)
          {
            return;
          }
          const int32 NeighbourIndex = NY * NumX + NX;
          if (Resolved[NeighbourIndex])
          {
            NeighbourSum += Heights[NeighbourIndex];
            ++NeighbourCount;
          }
        };
        VisitNeighbour(IX - 1, IY);
        VisitNeighbour(IX + 1, IY);
        VisitNeighbour(IX, IY - 1);
        VisitNeighbour(IX, IY + 1);
        if (NeighbourCount > 0)
        {
          Filled.Emplace(Index, NeighbourSum / static_cast<float>(NeighbourCount));
        }
      }
    }
    if (Filled.Num() == 0)
    {
      break;
    }
    for (const TPair<int32, float> &Entry : Filled)
    {
      Heights[Entry.Key] = Entry.Value;
      Resolved[Entry.Key] = true;
    }
    NumUnresolved -= Filled.Num();
  }

  FProceduralCustomMesh MeshData;
  MeshData.Vertices.Reserve(NumX * NumY);
  MeshData.UV0.Reserve(NumX * NumY);
  const float UVTile = FMath::Max(GroundUVTileSize, 1.0f);
  for (int32 IY = 0; IY < NumY; ++IY)
  {
    const float Y = MinY + static_cast<float>(IY) * StepY;
    for (int32 IX = 0; IX < NumX; ++IX)
    {
      const float X = MinX + static_cast<float>(IX) * StepX;
      MeshData.Vertices.Add(FVector(X, Y, Heights[IY * NumX + IX] - GroundPlaneZOffset));
      // World-space UV tiling (GroundUVTileSize cm per texture repeat)
      // instead of stretching one tile across the whole map.
      MeshData.UV0.Add(FVector2D((X - MinX) / UVTile, (Y - MinY) / UVTile));
    }
  }

  // Per-vertex normals from the heightfield gradient (central differences,
  // one-sided at the borders) so lighting shades the slopes instead of
  // treating the whole ground as one flat plane.
  MeshData.Normals.Reserve(NumX * NumY);
  const auto VertexZ = [&](int32 IX, int32 IY) -> float
  {
    return MeshData.Vertices[IY * NumX + IX].Z;
  };
  for (int32 IY = 0; IY < NumY; ++IY)
  {
    for (int32 IX = 0; IX < NumX; ++IX)
    {
      const int32 XPrev = FMath::Max(IX - 1, 0);
      const int32 XNext = FMath::Min(IX + 1, NumX - 1);
      const int32 YPrev = FMath::Max(IY - 1, 0);
      const int32 YNext = FMath::Min(IY + 1, NumY - 1);
      const float DZDX = (VertexZ(XNext, IY) - VertexZ(XPrev, IY)) / (static_cast<float>(XNext - XPrev) * StepX);
      const float DZDY = (VertexZ(IX, YNext) - VertexZ(IX, YPrev)) / (static_cast<float>(YNext - YPrev) * StepY);
      MeshData.Normals.Add(FVector(-DZDX, -DZDY, 1.0f).GetSafeNormal());
    }
  }

  // Winding per cell mirrors the previous single quad (corner order -X+Y,
  // +X+Y, +X-Y, -X-Y with triangles {0,1,3},{1,2,3}, same as the engine's
  // top-face quad in UKismetProceduralMeshLibrary::GenerateBoxMesh), so the
  // surface faces up rather than being backface-culled from above.
  MeshData.Triangles.Reserve((NumX - 1) * (NumY - 1) * 6);
  for (int32 IY = 0; IY + 1 < NumY; ++IY)
  {
    for (int32 IX = 0; IX + 1 < NumX; ++IX)
    {
      const int32 C0 = (IY + 1) * NumX + IX;       // -X +Y
      const int32 C1 = (IY + 1) * NumX + IX + 1;   // +X +Y
      const int32 C2 = IY * NumX + IX + 1;         // +X -Y
      const int32 C3 = IY * NumX + IX;             // -X -Y
      MeshData.Triangles.Append({ C0, C1, C3, C1, C2, C3 });
    }
  }

  ACarlaProceduralMeshActor* TempActor = GetWorld()->SpawnActor<ACarlaProceduralMeshActor>();
  UProceduralMeshComponent *TempPMC = TempActor->MeshComponent;
  TempPMC->bUseAsyncCooking = true;
  TempPMC->bUseComplexAsSimpleCollision = true;
  TempPMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

  TempPMC->CreateMeshSection_LinearColor(
      0,
      MeshData.Vertices,
      MeshData.Triangles,
      MeshData.Normals,
      MeshData.UV0,
      TArray<FLinearColor>(), // VertexColor
      TArray<FProcMeshTangent>(), // Tangents
      true); // Create collision

  if (UMaterialInterface* ResolvedGroundMaterial = GroundMaterial.LoadSynchronous())
  {
    TempPMC->SetMaterial(0, ResolvedGroundMaterial);
  }

  TagGeneratedComponent(*TempPMC, TempActor->GetUniqueID(), crp::CityObjectLabel::Terrain);

  if (!Parameters.enable_mesh_visibility)
  {
    TempActor->SetActorHiddenInGame(true);
  }

  ActorMeshList.Add(TempActor);
}

void AOpenDriveGenerator::GenerateCrosswalkMesh()
{
  if (!IsOpenDriveValid())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive has not been loaded"));
    return;
  }

  carla::rpc::OpendriveGenerationParameters Parameters;
  UCarlaGameInstance * GameInstance = UCarlaStatics::GetGameInstance(GetWorld());
  if (GameInstance)
  {
    Parameters = GameInstance->GetOpendriveGenerationParameters();
  }

  auto& CarlaMap = UCarlaStatics::GetGameMode(GetWorld())->GetMap();
  const carla::geom::Mesh CrosswalkMesh = CarlaMap->GetAllCrosswalkMesh();
  if (!CrosswalkMesh.GetVertices().size())
  {
    return;
  }

  ACarlaProceduralMeshActor* TempActor = GetWorld()->SpawnActor<ACarlaProceduralMeshActor>();
  UProceduralMeshComponent *TempPMC = TempActor->MeshComponent;
  TempPMC->bUseAsyncCooking = true;
  TempPMC->bUseComplexAsSimpleCollision = true;
  TempPMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

  // Previously GetAllCrosswalkMesh() was only consumed for the pedestrian
  // nav OBJ export (CarlaEpisode.cpp) and never rendered. The whole mesh
  // carries a single "crosswalk" material tag (Map::GetAllCrosswalkMesh),
  // so the plain whole-mesh conversion is enough here.
  FProceduralCustomMesh MeshData = CrosswalkMesh;
  for (FVector &Vertex : MeshData.Vertices)
  {
    // Small z-offset above the road surface to avoid z-fighting.
    Vertex.Z += DecalZOffset;
  }

  TempPMC->CreateMeshSection_LinearColor(
      0,
      MeshData.Vertices,
      MeshData.Triangles,
      MeshData.Normals,
      MeshData.UV0,
      TArray<FLinearColor>(), // VertexColor
      TArray<FProcMeshTangent>(), // Tangents
      true); // Create collision

  if (UMaterialInterface* ResolvedCrosswalkMaterial = CrosswalkMaterial.LoadSynchronous())
  {
    TempPMC->SetMaterial(0, ResolvedCrosswalkMaterial);
  }

  // CARLA has no dedicated CityObjectLabel for crosswalks; the ue4 tooling
  // (CarlaTools OpenDriveToMap) tags road markings as RoadLines, mirrored here.
  TagGeneratedComponent(*TempPMC, TempActor->GetUniqueID(), crp::CityObjectLabel::RoadLines);

  if (!Parameters.enable_mesh_visibility)
  {
    TempActor->SetActorHiddenInGame(true);
  }

  ActorMeshList.Add(TempActor);
}

void AOpenDriveGenerator::GenerateLaneMarkings()
{
  if (!IsOpenDriveValid())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive has not been loaded"));
    return;
  }

  carla::rpc::OpendriveGenerationParameters Parameters;
  UCarlaGameInstance * GameInstance = UCarlaStatics::GetGameInstance(GetWorld());
  if (GameInstance)
  {
    Parameters = GameInstance->GetOpendriveGenerationParameters();
  }
  else
  {
    carla::log_warning("Missing game instance");
  }

  auto& CarlaMap = UCarlaStatics::GetGameMode(GetWorld())->GetMap();

  // Map::GenerateLineMarkings filters roads by a min/max bounding box; it
  // backs the tiled/streamed generation path (GenerateOrderedChunkedMeshInLocations).
  // generate_opendrive_world builds the whole map in one shot, so pass an
  // effectively unbounded box instead. Map::FilterRoadsByPosition expects
  // minpos.y > maxpos.y (Unreal's left-handed Y vs. OpenDRIVE), mirrored here.
  constexpr float BoundsExtentMeters = 1e6f;
  const carla::geom::Vector3D MinPos(-BoundsExtentMeters, BoundsExtentMeters, -BoundsExtentMeters);
  const carla::geom::Vector3D MaxPos(BoundsExtentMeters, -BoundsExtentMeters, BoundsExtentMeters);

  std::vector<std::string> MarkInfo;
  const auto Markings = CarlaMap->GenerateLineMarkings(Parameters, MinPos, MaxPos, MarkInfo);

  UMaterialInterface* ResolvedWhite = LaneMarkingWhiteMaterial.LoadSynchronous();
  UMaterialInterface* ResolvedYellow = LaneMarkingYellowMaterial.LoadSynchronous();

  size_t Index = 0;
  for (const auto &Mark : Markings)
  {
    // Map::GenerateLineMarkings/MeshFactory::GenerateLaneMarkForRoad push one
    // color tag per lane pass but only push a mesh when it produced valid
    // geometry, so MarkInfo and Markings can fall out of sync (matches
    // CarlaTools OpenDriveToMap::GenerateLaneMarks, which assumes the same
    // 1:1 index). Guard the lookup so a mismatch can't read out of bounds.
    const bool bIsYellow = (Index < MarkInfo.size()) &&
        (MarkInfo[Index].find("yellow") != std::string::npos);
    ++Index;

    if (!Mark || !Mark->GetVertices().size())
    {
      continue;
    }

    ACarlaProceduralMeshActor* TempActor = GetWorld()->SpawnActor<ACarlaProceduralMeshActor>();
    UProceduralMeshComponent *TempPMC = TempActor->MeshComponent;
    TempPMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FProceduralCustomMesh MeshData = *Mark;
    for (FVector &Vertex : MeshData.Vertices)
    {
      // Small z-offset above the road surface to avoid z-fighting.
      Vertex.Z += DecalZOffset;
    }

    TempPMC->CreateMeshSection_LinearColor(
        0,
        MeshData.Vertices,
        MeshData.Triangles,
        MeshData.Normals,
        MeshData.UV0,
        TArray<FLinearColor>(), // VertexColor
        TArray<FProcMeshTangent>(), // Tangents
        false); // Create collision -- flush with the road, no separate collision needed

    UMaterialInterface* SectionMaterial = bIsYellow ? ResolvedYellow : ResolvedWhite;
    if (SectionMaterial)
    {
      TempPMC->SetMaterial(0, SectionMaterial);
    }

    TagGeneratedComponent(*TempPMC, TempActor->GetUniqueID(), crp::CityObjectLabel::RoadLines);

    if (!Parameters.enable_mesh_visibility)
    {
      TempActor->SetActorHiddenInGame(true);
    }

    ActorMeshList.Add(TempActor);
  }
}

int32 AOpenDriveGenerator::GenerateFurnitureAnchors(const FName &Tag, float Spacing, float Offset)
{
  auto& CarlaMap = UCarlaStatics::GetGameMode(GetWorld())->GetMap();

  // Map::GetTreesTransform filters roads by a min/max bounding box (same
  // pattern as GenerateLineMarkings/GenerateLaneMarkings); generate_opendrive_world
  // builds the whole map in one shot, so pass an effectively unbounded box
  // instead of a real tile. Map::FilterRoadsByPosition expects
  // minpos.y > maxpos.y (Unreal's left-handed Y vs. OpenDRIVE), mirrored here.
  constexpr float BoundsExtentMeters = 1e6f;
  const carla::geom::Vector3D MinPos(-BoundsExtentMeters, BoundsExtentMeters, -BoundsExtentMeters);
  const carla::geom::Vector3D MaxPos(BoundsExtentMeters, -BoundsExtentMeters, BoundsExtentMeters);

  // Reused as-is from the editor-only OpenDriveToMap tool's tree placement
  // (CarlaTools/OpenDriveToMap.cpp:478-506, GenerateMiscActors): it already
  // walks each non-junction road's outermost driving lane at regular
  // s-intervals and offsets outward past it -- exactly the "spacing along
  // road, offset past the sidewalk" placement street furniture needs, with
  // zero new LibCarla code. The second element of each pair is a road-type
  // string (from RoadInfoSpeed, e.g. "Town"/"Highway"/"Rural"), unused here
  // but available on the anchor's tags if the PCG graph wants to vary
  // furniture density by road type later.
  const auto Anchors = CarlaMap->GetTreesTransform(MinPos, MaxPos, Spacing, Offset);

  for (const auto &AnchorPair : Anchors)
  {
    const FTransform AnchorTransform = AnchorPair.first;
    AProceduralFurnitureAnchor* Anchor = GetWorld()->SpawnActor<AProceduralFurnitureAnchor>(
        AnchorTransform.GetLocation(), AnchorTransform.Rotator());
    Anchor->Tags.Add(Tag);
    FurnitureAnchorBounds += AnchorTransform.GetLocation();
    ActorMeshList.Add(Anchor);
  }

  return static_cast<int32>(Anchors.size());
}

void AOpenDriveGenerator::GeneratePoles()
{
  if (!IsOpenDriveValid())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive has not been loaded"));
    return;
  }

  const int32 NumLampAnchors = GenerateFurnitureAnchors(LampAnchorTag, LampAnchorSpacing, LampAnchorOffset);
  const int32 NumVegetationAnchors = GenerateFurnitureAnchors(VegetationAnchorTag, VegetationAnchorSpacing, VegetationAnchorOffset);
  const int32 NumSignageAnchors = GenerateFurnitureAnchors(SignageAnchorTag, SignageAnchorSpacing, SignageAnchorOffset);
  const int32 TotalAnchors = NumLampAnchors + NumVegetationAnchors + NumSignageAnchors;
  UE_LOG(LogCarla, Log, TEXT("AOpenDriveGenerator: furniture anchors spawned: %d lamp, %d vegetation, %d signage"),
      NumLampAnchors, NumVegetationAnchors, NumSignageAnchors);

  if (TotalAnchors == 0)
  {
    return;
  }

  // Trigger the runtime PCG scatter now that every anchor exists. The graph
  // itself does the actual spawning (lamp/vegetation/signage actors),
  // sampling the tagged anchors above via "Get Actor Data" nodes -- see
  // StreetFurnitureGraph's doc comment in the header for the expected
  // graph layout. PCGComponent is non-partitioned, so this call works with
  // no PCGWorldActor / World Partition involved.
  if (UPCGGraphInterface* Graph = StreetFurnitureGraph.LoadSynchronous())
  {
    // PCG refuses to schedule with invalid owner bounds (this actor has no
    // visible geometry), so size the bounds box to the anchor cloud first.
    if (FurnitureAnchorBounds.IsValid)
    {
      const FBox Padded = FurnitureAnchorBounds.ExpandBy(2000.0f);
      PCGBoundsComponent->SetWorldLocation(Padded.GetCenter());
      PCGBoundsComponent->SetBoxExtent(Padded.GetExtent());
      PCGBoundsComponent->UpdateBounds();
    }
    PCGComponent->SetGraph(Graph);
    PCGComponent->Generate(true);
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT("AOpenDriveGenerator: StreetFurnitureGraph is not ")
        TEXT("set, skipping PCG street-furniture scatter (%d lamp/vegetation/signage ")
        TEXT("anchors were still spawned and are ready to sample once a graph is assigned)."),
        TotalAnchors);
  }
}

void AOpenDriveGenerator::GenerateSpawnPoints()
{
  if (!IsOpenDriveValid())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive has not been loaded"));
    return;
  }
  auto& CarlaMap = UCarlaStatics::GetGameMode(GetWorld())->GetMap();
  const auto Waypoints = CarlaMap->GenerateWaypointsOnRoadEntries();
  for (const auto &Wp : Waypoints)
  {
    const FTransform Trans = CarlaMap->ComputeTransform(Wp);
    AVehicleSpawnPoint *Spawner = GetWorld()->SpawnActor<AVehicleSpawnPoint>();
    Spawner->SetActorRotation(Trans.GetRotation());
    Spawner->SetActorLocation(Trans.GetTranslation() + FVector(0.f, 0.f, SpawnersHeight));
    VehicleSpawners.Add(Spawner);
  }
}

void AOpenDriveGenerator::GenerateAll()
{
  GenerateRoadMesh();
  GenerateGroundPlane();
  GenerateCrosswalkMesh();
  GenerateLaneMarkings();
  GenerateSpawnPoints();
  GeneratePoles();
}

void AOpenDriveGenerator::BeginPlay()
{
  Super::BeginPlay();

  const FString XodrContent = UOpenDrive::GetXODR(GetWorld());
  LoadOpenDrive(XodrContent);

  GenerateAll();

  auto World = GetWorld();
  check(World != nullptr);

  // Autogenerate signals
  AActor* TrafficLightManagerActor =  UGameplayStatics::GetActorOfClass(World, ATrafficLightManager::StaticClass());
  if(TrafficLightManagerActor == nullptr) {
    World->SpawnActor<ATrafficLightManager>();
  }
}
