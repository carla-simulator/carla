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

  /// One level of the multilevel B-spline approximation (Lee, Wolberg &
  /// Shin, "Scattered Data Interpolation with Multilevel B-Splines", IEEE
  /// TVCG 1997) used to fit the ground terrain surface to the road height
  /// samples: a uniform bicubic B-spline over a regular control lattice,
  /// fit in closed form to scattered points by distributing each point's
  /// value onto the 4x4 control points whose basis functions cover it,
  /// weighted so points with overlapping support average by influence.
  struct FGroundBSplineLattice
  {
    float OriginX = 0.0f;
    float OriginY = 0.0f;
    float Spacing = 1.0f;
    int32 CellsX = 1;
    int32 CellsY = 1;
    TArray<float> Control; // (CellsX + 3) * (CellsY + 3), row-major in Y

    static void Basis(float T, float B[4])
    {
      const float T2 = T * T;
      const float T3 = T2 * T;
      B[0] = (1.0f - T) * (1.0f - T) * (1.0f - T) / 6.0f;
      B[1] = (3.0f * T3 - 6.0f * T2 + 4.0f) / 6.0f;
      B[2] = (-3.0f * T3 + 3.0f * T2 + 3.0f * T + 1.0f) / 6.0f;
      B[3] = T3 / 6.0f;
    }

    void LocalCoords(float X, float Y, int32 &I, int32 &J, float &S, float &T) const
    {
      const float FX = FMath::Clamp((X - OriginX) / Spacing, 0.0f, static_cast<float>(CellsX) - KINDA_SMALL_NUMBER);
      const float FY = FMath::Clamp((Y - OriginY) / Spacing, 0.0f, static_cast<float>(CellsY) - KINDA_SMALL_NUMBER);
      I = FMath::Clamp(FMath::FloorToInt(FX), 0, CellsX - 1);
      J = FMath::Clamp(FMath::FloorToInt(FY), 0, CellsY - 1);
      S = FX - static_cast<float>(I);
      T = FY - static_cast<float>(J);
    }

    float Eval(float X, float Y) const
    {
      int32 I, J;
      float S, T, BS[4], BT[4];
      LocalCoords(X, Y, I, J, S, T);
      Basis(S, BS);
      Basis(T, BT);
      const int32 NumCtrlX = CellsX + 3;
      float Z = 0.0f;
      for (int32 L = 0; L < 4; ++L)
      {
        for (int32 K = 0; K < 4; ++K)
        {
          Z += BS[K] * BT[L] * Control[(J + L) * NumCtrlX + (I + K)];
        }
      }
      return Z;
    }

    void Fit(const TArray<FVector> &Points)
    {
      const int32 NumCtrlX = CellsX + 3;
      const int32 NumCtrlY = CellsY + 3;
      TArray<float> Num, Den;
      Num.Init(0.0f, NumCtrlX * NumCtrlY);
      Den.Init(0.0f, NumCtrlX * NumCtrlY);
      for (const FVector &P : Points)
      {
        int32 I, J;
        float S, T, BS[4], BT[4];
        LocalCoords(P.X, P.Y, I, J, S, T);
        Basis(S, BS);
        Basis(T, BT);
        float SumW2 = 0.0f;
        for (int32 L = 0; L < 4; ++L)
        {
          for (int32 K = 0; K < 4; ++K)
          {
            SumW2 += FMath::Square(BS[K] * BT[L]);
          }
        }
        if (SumW2 <= KINDA_SMALL_NUMBER)
        {
          continue;
        }
        for (int32 L = 0; L < 4; ++L)
        {
          for (int32 K = 0; K < 4; ++K)
          {
            const float W = BS[K] * BT[L];
            const float Phi = W * static_cast<float>(P.Z) / SumW2;
            const int32 Idx = (J + L) * NumCtrlX + (I + K);
            Num[Idx] += W * W * Phi;
            Den[Idx] += W * W;
          }
        }
      }
      Control.Init(0.0f, NumCtrlX * NumCtrlY);
      for (int32 Idx = 0; Idx < Control.Num(); ++Idx)
      {
        if (Den[Idx] > KINDA_SMALL_NUMBER)
        {
          Control[Idx] = Num[Idx] / Den[Idx];
        }
      }
    }
  };

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

  // A heightfield grid spanning the padded road-network footprint, sampled
  // from a C2-smooth terrain surface fit to the road heights. A flat quad
  // at global min-z closed the blue-sky holes on CARLA's flat towns but
  // left real-elevation OpenDRIVE roads (NuRec: Stuttgart z~293m, graded
  // motorways) flying tens of meters above it; the follow-up piecewise-
  // linear interpolation tracked elevation on average but couldn't follow
  // vertical road curvature -- terrain bulged through the road in sag
  // curves and dropped away under crests, and its weighted averaging
  // between roads at different heights clipped the lower road.
  const FBox PaddedBounds = RoadMeshBounds.ExpandBy(FVector(GroundPlanePadding, GroundPlanePadding, 0.0f));
  const float MinX = PaddedBounds.Min.X;
  const float MaxX = PaddedBounds.Max.X;
  const float MinY = PaddedBounds.Min.Y;
  const float MaxY = PaddedBounds.Max.Y;

  // Cap the vertex budget: if the padded map is large enough that the
  // configured spacing would blow past it, widen the spacing instead of
  // failing or stalling the load with a multi-million-vertex cook.
  constexpr int32 MaxGridVerticesPerAxis = 1024;
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

  // Scattered fit points: one per occupied sample cell, at the cell center,
  // carrying the min road/sidewalk z recorded there.
  TArray<FVector> FitPoints;
  FitPoints.Reserve(GroundHeightSampleGrid.Num());
  for (const auto &Sample : GroundHeightSampleGrid)
  {
    FitPoints.Add(FVector(
        (static_cast<float>(Sample.Key.X) + 0.5f) * SampleCellSize,
        (static_cast<float>(Sample.Key.Y) + 0.5f) * SampleCellSize,
        Sample.Value));
  }

  // Multilevel B-spline approximation: start with a control lattice a few
  // cells across the whole padded map (broad rolling shape, also the
  // smooth extrapolation into the padding and across road-free voids),
  // then repeatedly halve the control spacing, fitting each finer level to
  // the residual the coarser ones left behind, until the control cells
  // approach the road-sample resolution. The summed hierarchy is a
  // C2-smooth surface that passes through the road elevations where there
  // is road and rolls smoothly where there isn't.
  TArray<FGroundBSplineLattice> SplineLevels;
  const float ExtentX = MaxX - MinX;
  const float ExtentY = MaxY - MinY;
  float ControlSpacing = FMath::Max(ExtentX, ExtentY) / 4.0f;
  const float FinestControlSpacing = FMath::Max(4.0f * SampleCellSize, GridStep);
  constexpr int32 MaxSplineLevels = 12;
  TArray<FVector> Residuals = FitPoints;
  while (SplineLevels.Num() < MaxSplineLevels)
  {
    FGroundBSplineLattice Level;
    Level.OriginX = MinX;
    Level.OriginY = MinY;
    Level.Spacing = ControlSpacing;
    Level.CellsX = FMath::Max(1, FMath::CeilToInt(ExtentX / ControlSpacing));
    Level.CellsY = FMath::Max(1, FMath::CeilToInt(ExtentY / ControlSpacing));
    Level.Fit(Residuals);
    for (FVector &P : Residuals)
    {
      P.Z -= Level.Eval(P.X, P.Y);
    }
    SplineLevels.Add(MoveTemp(Level));
    if (ControlSpacing <= FinestControlSpacing)
    {
      break;
    }
    ControlSpacing *= 0.5f;
  }

  const auto EvalSpline = [&SplineLevels](float X, float Y) -> float
  {
    float Z = 0.0f;
    for (const FGroundBSplineLattice &Level : SplineLevels)
    {
      Z += Level.Eval(X, Y);
    }
    return Z;
  };

  // Evaluate the terrain surface at each grid vertex, GroundPlaneZOffset
  // below the fitted road height. Near roads, additionally clamp to the
  // lowest actual road sample in the 3x3 cell neighbourhood: the spline
  // approximates rather than interpolates, and even a small overshoot in a
  // sag curve would poke terrain up through the road surface.
  TArray<float> Heights;
  Heights.SetNumUninitialized(NumX * NumY);
  for (int32 IY = 0; IY < NumY; ++IY)
  {
    const float Y = MinY + static_cast<float>(IY) * StepY;
    for (int32 IX = 0; IX < NumX; ++IX)
    {
      const float X = MinX + static_cast<float>(IX) * StepX;
      float GroundZ = EvalSpline(X, Y) - GroundPlaneZOffset;
      const int32 CellX = FMath::FloorToInt(X / SampleCellSize);
      const int32 CellY = FMath::FloorToInt(Y / SampleCellSize);
      for (int32 DY = -1; DY <= 1; ++DY)
      {
        for (int32 DX = -1; DX <= 1; ++DX)
        {
          if (const float *SampleZ = GroundHeightSampleGrid.Find(FIntPoint(CellX + DX, CellY + DY)))
          {
            GroundZ = FMath::Min(GroundZ, *SampleZ - GroundPlaneZOffset);
          }
        }
      }
      Heights[IY * NumX + IX] = GroundZ;
    }
  }

  // Retain the evaluated grid so GenerateFurnitureAnchors can re-ground
  // scattered objects onto the terrain surface they will stand on.
  GroundGridHeights = Heights;
  GroundGridNumX = NumX;
  GroundGridNumY = NumY;
  GroundGridOrigin = FVector2D(MinX, MinY);
  GroundGridStepX = StepX;
  GroundGridStepY = StepY;

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
      MeshData.Vertices.Add(FVector(X, Y, Heights[IY * NumX + IX]));
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

float AOpenDriveGenerator::SampleGroundGridHeight(float X, float Y) const
{
  const float FX = FMath::Clamp(
      (X - static_cast<float>(GroundGridOrigin.X)) / FMath::Max(GroundGridStepX, 1.0f),
      0.0f, static_cast<float>(GroundGridNumX - 1) - KINDA_SMALL_NUMBER);
  const float FY = FMath::Clamp(
      (Y - static_cast<float>(GroundGridOrigin.Y)) / FMath::Max(GroundGridStepY, 1.0f),
      0.0f, static_cast<float>(GroundGridNumY - 1) - KINDA_SMALL_NUMBER);
  const int32 IX = FMath::Min(FMath::FloorToInt(FX), GroundGridNumX - 2);
  const int32 IY = FMath::Min(FMath::FloorToInt(FY), GroundGridNumY - 2);
  const float S = FX - static_cast<float>(IX);
  const float T = FY - static_cast<float>(IY);
  const float Z00 = GroundGridHeights[IY * GroundGridNumX + IX];
  const float Z10 = GroundGridHeights[IY * GroundGridNumX + IX + 1];
  const float Z01 = GroundGridHeights[(IY + 1) * GroundGridNumX + IX];
  const float Z11 = GroundGridHeights[(IY + 1) * GroundGridNumX + IX + 1];
  return FMath::Lerp(FMath::Lerp(Z00, Z10, S), FMath::Lerp(Z01, Z11, S), T);
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

  const float SampleCellSize = FMath::Max(GroundHeightSampleCellSize, 1.0f);
  int32 NumSpawned = 0;
  int32 NumFilteredOnRoad = 0;

  for (const auto &AnchorPair : Anchors)
  {
    const FTransform AnchorTransform = AnchorPair.first;
    FVector Location = AnchorTransform.GetLocation();

    // Road-occupancy filter: GetTreesTransform offsets each anchor a fixed
    // lateral distance past its own road's outermost driving lane, which is
    // wrong wherever roads widen, merge, overlap or fan into junctions --
    // the anchor can land on the roadway of *another* lane/road and the
    // spawned furniture then blocks traffic. Discard any anchor within the
    // closest driving lane's half-width plus a clearance margin of that
    // lane's centerline, whichever road it belongs to.
    const auto ClosestWp = CarlaMap->GetClosestWaypointOnRoad(carla::geom::Location(Location));
    if (ClosestWp)
    {
      const FTransform LaneTransform = CarlaMap->ComputeTransform(*ClosestWp);
      const float LaneHalfWidth = 0.5f * 1e2f * static_cast<float>(CarlaMap->GetLaneWidth(*ClosestWp));
      if (FVector::Dist2D(Location, LaneTransform.GetLocation()) < LaneHalfWidth + FurnitureRoadClearance)
      {
        ++NumFilteredOnRoad;
        continue;
      }
    }

    // Re-ground: the anchor inherits the road-edge elevation from
    // GetTreesTransform, but it stands offset onto pavement or terrain
    // that is generally lower (ground offset plus lateral grade), leaving
    // spawned furniture floating. If the anchor's sample cell saw road/
    // sidewalk geometry, snap to that surface; otherwise drop it onto the
    // ground heightfield generated by GenerateGroundPlane.
    const FIntPoint Cell(
        FMath::FloorToInt(Location.X / SampleCellSize),
        FMath::FloorToInt(Location.Y / SampleCellSize));
    if (const float *PavementZ = GroundHeightSampleGrid.Find(Cell))
    {
      Location.Z = *PavementZ;
    }
    else if (GroundGridNumX > 1 && GroundGridNumY > 1)
    {
      Location.Z = SampleGroundGridHeight(Location.X, Location.Y);
    }

    AProceduralFurnitureAnchor* Anchor = GetWorld()->SpawnActor<AProceduralFurnitureAnchor>(
        Location, AnchorTransform.Rotator());
    Anchor->Tags.Add(Tag);
    FurnitureAnchorBounds += Location;
    ActorMeshList.Add(Anchor);
    ++NumSpawned;
  }

  if (NumFilteredOnRoad > 0)
  {
    UE_LOG(LogCarla, Log, TEXT("AOpenDriveGenerator: %s: discarded %d/%d anchors too close to a driving lane"),
        *Tag.ToString(), NumFilteredOnRoad, static_cast<int32>(Anchors.size()));
  }

  return NumSpawned;
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
