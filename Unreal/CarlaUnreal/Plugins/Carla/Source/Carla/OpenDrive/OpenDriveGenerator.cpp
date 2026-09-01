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
#include "Traffic/TrafficSignBase.h"
#include "Util/ProceduralCustomMesh.h"
#include "Materials/MaterialInterface.h"
#include "PCGComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "PCGGraph.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UObjectIterator.h"

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

    // Recompute per-vertex normals over the extracted sub-mesh: accumulate
    // the (area-weighted) up-biased face normal of every incident triangle
    // and normalize, so shared vertices shade smoothly. Assigning the raw
    // face normal per triangle (last writer wins) made every 2 m strip
    // quad a separately-lit facet and the whole road read as shattered.
    TArray<FVector> Accum;
    Accum.Init(FVector::ZeroVector, Result.Vertices.Num());
    for (int32 i = 0; i < Result.Triangles.Num(); i += 3)
    {
      const FVector &V0 = Result.Vertices[Result.Triangles[i]];
      const FVector &V1 = Result.Vertices[Result.Triangles[i + 1]];
      const FVector &V2 = Result.Vertices[Result.Triangles[i + 2]];
      const FVector U = V1 - V0;
      const FVector V = V2 - V0;
      // Unnormalized cross product: magnitude = 2x triangle area, giving
      // the area weighting for free.
      FVector Normal;
      Normal.X = (U.Y * V.Z) - (U.Z * V.Y);
      Normal.Y = (U.Z * V.X) - (U.X * V.Z);
      Normal.Z = (U.X * V.Y) - (U.Y * V.X);
      Normal = -Normal;
      if (Normal.Z < 0.0f)
      {
        // Road geometry always faces up; mirrored winding otherwise
        // cancels neighbouring contributions instead of averaging.
        Normal = -Normal;
      }
      Accum[Result.Triangles[i]] += Normal;
      Accum[Result.Triangles[i + 1]] += Normal;
      Accum[Result.Triangles[i + 2]] += Normal;
    }
    Result.Normals.Init(FVector::UpVector, Result.Vertices.Num());
    for (int32 i = 0; i < Accum.Num(); ++i)
    {
      const FVector Normal = Accum[i].GetSafeNormal(.0001f);
      if (Normal != FVector::ZeroVector)
      {
        Result.Normals[i] = Normal;
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

void AOpenDriveGenerator::FRoadSurfaceRaster::Initialize(const FBox &Bounds, float InCellSize)
{
  CellSize = FMath::Max(InCellSize, 50.0f);
  OriginX = Bounds.Min.X;
  OriginY = Bounds.Min.Y;
  Width = FMath::Max(1, FMath::CeilToInt((Bounds.Max.X - Bounds.Min.X) / CellSize)) + 1;
  Height = FMath::Max(1, FMath::CeilToInt((Bounds.Max.Y - Bounds.Min.Y) / CellSize)) + 1;
  Cells.Empty();
  Cells.SetNum(Width * Height);
}

void AOpenDriveGenerator::FRoadSurfaceRaster::RasterizeTriangle(
    const FVector &A, const FVector &B, const FVector &C, bool bIsDrive)
{
  const float MinX = FMath::Min3(A.X, B.X, C.X);
  const float MaxX = FMath::Max3(A.X, B.X, C.X);
  const float MinY = FMath::Min3(A.Y, B.Y, C.Y);
  const float MaxY = FMath::Max3(A.Y, B.Y, C.Y);
  const int32 X0 = FMath::Clamp(FMath::FloorToInt((MinX - OriginX) / CellSize), 0, Width - 1);
  const int32 X1 = FMath::Clamp(FMath::CeilToInt((MaxX - OriginX) / CellSize), 0, Width - 1);
  const int32 Y0 = FMath::Clamp(FMath::FloorToInt((MinY - OriginY) / CellSize), 0, Height - 1);
  const int32 Y1 = FMath::Clamp(FMath::CeilToInt((MaxY - OriginY) / CellSize), 0, Height - 1);

  const float Denom = (B.Y - C.Y) * (A.X - C.X) + (C.X - B.X) * (A.Y - C.Y);
  if (FMath::Abs(Denom) < KINDA_SMALL_NUMBER)
  {
    return; // degenerate in XY (vertical face); carries no surface footprint
  }

  for (int32 CY = Y0; CY <= Y1; ++CY)
  {
    for (int32 CX = X0; CX <= X1; ++CX)
    {
      const float PX = OriginX + (static_cast<float>(CX) + 0.5f) * CellSize;
      const float PY = OriginY + (static_cast<float>(CY) + 0.5f) * CellSize;
      float W0 = ((B.Y - C.Y) * (PX - C.X) + (C.X - B.X) * (PY - C.Y)) / Denom;
      float W1 = ((C.Y - A.Y) * (PX - C.X) + (A.X - C.X) * (PY - C.Y)) / Denom;
      float W2 = 1.0f - W0 - W1;
      // Tolerance of ~1/8 cell in barycentric space keeps thin triangles
      // whose interior never covers a cell center from leaving footprint
      // holes along lane edges.
      constexpr float Tolerance = -0.05f;
      if (W0 < Tolerance || W1 < Tolerance || W2 < Tolerance)
      {
        continue;
      }
      // Genuine containment (no tolerance): the cell center is inside this
      // triangle, so the cell really is covered by driving surface.
      const bool bInside = (W0 >= 0.0f && W1 >= 0.0f && W2 >= 0.0f);
      W0 = FMath::Max(W0, 0.0f);
      W1 = FMath::Max(W1, 0.0f);
      W2 = FMath::Max(W2, 0.0f);
      const float WSum = W0 + W1 + W2;
      const float Z = (W0 * A.Z + W1 * B.Z + W2 * C.Z) / FMath::Max(WSum, KINDA_SMALL_NUMBER);

      FCell &Cell = Cells[CY * Width + CX];
      if (!Cell.bPaved)
      {
        Cell.PavedMinZ = Z;
        Cell.bPaved = 1;
      }
      else
      {
        Cell.PavedMinZ = FMath::Min(Cell.PavedMinZ, Z);
      }
      if (bIsDrive)
      {
        if (!Cell.bDrive)
        {
          Cell.DriveMinZ = Z;
          Cell.DriveMaxZ = Z;
          Cell.bDrive = 1;
        }
        else
        {
          Cell.DriveMinZ = FMath::Min(Cell.DriveMinZ, Z);
          Cell.DriveMaxZ = FMath::Max(Cell.DriveMaxZ, Z);
        }
        if (bInside)
        {
          Cell.bDriveCovered = 1;
        }
      }
    }
  }
}

void AOpenDriveGenerator::FRoadSurfaceRaster::BuildDistanceFields()
{
  // Two-pass chamfer distance transform (3-4 weights scaled to cell size,
  // ~4% max error -- fine for meter-scale clearances). Runs twice over the
  // same storage: once seeded by the driving footprint (DistToDrive), once
  // by any pavement (DistToPaved, also propagating the seed cell's
  // PavedMinZ as NearestPavedMinZ for the terrain skirt blend).
  const float Straight = CellSize;
  const float Diagonal = CellSize * 1.41421356f;
  const float Infinite = 1e12f;

  const auto Chamfer = [&](auto GetSeed, auto GetDist, auto SetDist, auto GetZ, auto SetZ)
  {
    for (int32 Idx = 0; Idx < Cells.Num(); ++Idx)
    {
      if (GetSeed(Cells[Idx]))
      {
        SetDist(Cells[Idx], 0.0f);
        SetZ(Cells[Idx], Cells[Idx].PavedMinZ);
      }
      else
      {
        SetDist(Cells[Idx], Infinite);
      }
    }
    const auto Relax = [&](FCell &Cell, int32 NX, int32 NY, float Cost)
    {
      if (NX < 0 || NX >= Width || NY < 0 || NY >= Height)
      {
        return;
      }
      const FCell &N = Cells[NY * Width + NX];
      if (GetDist(N) + Cost < GetDist(Cell))
      {
        SetDist(Cell, GetDist(N) + Cost);
        SetZ(Cell, GetZ(N));
      }
    };
    for (int32 CY = 0; CY < Height; ++CY)
    {
      for (int32 CX = 0; CX < Width; ++CX)
      {
        FCell &Cell = Cells[CY * Width + CX];
        Relax(Cell, CX - 1, CY, Straight);
        Relax(Cell, CX, CY - 1, Straight);
        Relax(Cell, CX - 1, CY - 1, Diagonal);
        Relax(Cell, CX + 1, CY - 1, Diagonal);
      }
    }
    for (int32 CY = Height - 1; CY >= 0; --CY)
    {
      for (int32 CX = Width - 1; CX >= 0; --CX)
      {
        FCell &Cell = Cells[CY * Width + CX];
        Relax(Cell, CX + 1, CY, Straight);
        Relax(Cell, CX, CY + 1, Straight);
        Relax(Cell, CX + 1, CY + 1, Diagonal);
        Relax(Cell, CX - 1, CY + 1, Diagonal);
      }
    }
  };

  float ScratchZ = 0.0f;
  Chamfer(
      [](const FCell &C) { return C.bDrive != 0; },
      [](const FCell &C) { return C.DistToDrive; },
      [](FCell &C, float V) { C.DistToDrive = V; },
      [&ScratchZ](const FCell &) { return ScratchZ; },
      [](FCell &, float) {});
  Chamfer(
      [](const FCell &C) { return C.bPaved != 0; },
      [](const FCell &C) { return C.DistToPaved; },
      [](FCell &C, float V) { C.DistToPaved = V; },
      [](const FCell &C) { return C.NearestPavedMinZ; },
      [](FCell &C, float V) { C.NearestPavedMinZ = V; });
}

void AOpenDriveGenerator::DumpRasterAt(float X, float Y) const
{
  if (!RoadRaster.IsValid())
  {
    UE_LOG(LogCarla, Warning, TEXT("DumpRasterAt: raster not built"));
    return;
  }
  const int32 CX0 = FMath::FloorToInt((X - RoadRaster.OriginX) / RoadRaster.CellSize);
  const int32 CY0 = FMath::FloorToInt((Y - RoadRaster.OriginY) / RoadRaster.CellSize);
  for (int32 DY = -1; DY <= 1; ++DY)
  {
    for (int32 DX = -1; DX <= 1; ++DX)
    {
      const int32 CX = CX0 + DX;
      const int32 CY = CY0 + DY;
      if (CX < 0 || CX >= RoadRaster.Width || CY < 0 || CY >= RoadRaster.Height)
      {
        continue;
      }
      const auto &Cell = RoadRaster.Cells[CY * RoadRaster.Width + CX];
      UE_LOG(LogCarla, Log,
          TEXT("cell(%d,%d) ctr(%.1f,%.1f) drive=%d min=%.1f max=%.1f"),
          CX, CY,
          RoadRaster.OriginX + (CX + 0.5f) * RoadRaster.CellSize,
          RoadRaster.OriginY + (CY + 0.5f) * RoadRaster.CellSize,
          Cell.bDrive, Cell.DriveMinZ, Cell.DriveMaxZ);
    }
  }
}

const AOpenDriveGenerator::FRoadSurfaceRaster::FCell *
AOpenDriveGenerator::FRoadSurfaceRaster::CellAtWorld(float X, float Y) const
{
  if (!IsValid())
  {
    return nullptr;
  }
  const int32 CX = FMath::FloorToInt((X - OriginX) / CellSize);
  const int32 CY = FMath::FloorToInt((Y - OriginY) / CellSize);
  if (CX < 0 || CX >= Width || CY < 0 || CY >= Height)
  {
    return nullptr;
  }
  return &Cells[CY * Width + CX];
}

/// Dump the road-raster chunk table around a point (client metres):
/// `carla.DumpRasterAt <x> <y>` -- debugging aid for stacked-surface work.
static FAutoConsoleCommandWithWorldAndArgs GDumpRasterAtCommand(
    TEXT("carla.DumpRasterAt"),
    TEXT("Dump road-raster chunk info around x y (metres)"),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(
        [](const TArray<FString> &Args, UWorld *World)
    {
      if (Args.Num() < 2)
      {
        UE_LOG(LogCarla, Warning, TEXT("usage: carla.DumpRasterAt <x> <y>"));
        return;
      }
      const float CX = FCString::Atof(*Args[0]) * 100.0f;
      const float CY = FCString::Atof(*Args[1]) * 100.0f;
      for (TActorIterator<AOpenDriveGenerator> It(World); It; ++It)
      {
        It->DumpRasterAt(CX, CY);
        return;
      }
    }));

/// Re-run the generation QA on the placed generator from the console:
/// `carla.MapGenQA` (also runs automatically after GenerateAll).
static FAutoConsoleCommandWithWorld GMapGenQACommand(
    TEXT("carla.MapGenQA"),
    TEXT("Run the generated-map QA checks (road support, terrain invasion, objects on roadway)"),
    FConsoleCommandWithWorldDelegate::CreateStatic([](UWorld *World)
    {
      for (TActorIterator<AOpenDriveGenerator> It(World); It; ++It)
      {
        It->RunGenerationQA();
        return;
      }
      UE_LOG(LogCarla, Warning, TEXT("carla.MapGenQA: no AOpenDriveGenerator in world"));
    }));

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

  // Two-phase: first gather every material section (accumulating bounds --
  // the road raster needs the full footprint extent before any triangle
  // can be splatted), then rasterize + spawn.
  TArray<TPair<FProceduralCustomMesh, bool>> PendingSections; // mesh, bIsSidewalk

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
      FProceduralCustomMesh &SectionMesh = SectionsByMaterial[MatName];
      const bool bIsSidewalk = (MatName == TEXT("sidewalk"));
      for (const FVector &Vertex : SectionMesh.Vertices)
      {
        RoadMeshBounds += Vertex;
      }
      PendingSections.Emplace(MoveTemp(SectionMesh), bIsSidewalk);
    }
  }

  // Build the road-surface raster before spawning anything: every road and
  // sidewalk triangle splatted with barycentric-interpolated heights, then
  // the distance fields that terrain generation, furniture placement and
  // the generation QA all query.
  if (RoadMeshBounds.IsValid)
  {
    const float Pad = GroundPlanePadding + 2.0f * GroundSkirtWidth;
    RoadRaster.Initialize(
        RoadMeshBounds.ExpandBy(FVector(Pad, Pad, 0.0f)),
        GroundHeightSampleCellSize);
    for (const auto &Section : PendingSections)
    {
      const FProceduralCustomMesh &SectionMesh = Section.Key;
      const bool bIsDrive = !Section.Value;
      for (int32 T = 0; T + 2 < SectionMesh.Triangles.Num(); T += 3)
      {
        RoadRaster.RasterizeTriangle(
            SectionMesh.Vertices[SectionMesh.Triangles[T]],
            SectionMesh.Vertices[SectionMesh.Triangles[T + 1]],
            SectionMesh.Vertices[SectionMesh.Triangles[T + 2]],
            bIsDrive);
      }
    }
    RoadRaster.BuildDistanceFields();
  }

  for (const auto &Section : PendingSections)
  {
    const FProceduralCustomMesh &SectionMesh = Section.Key;
    const bool bIsSidewalk = Section.Value;

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

  // Terrain grid aligned 1:1 with the road raster, vertices at cell
  // centers, so every paved cell owns exactly one terrain vertex and the
  // footprint hug below cannot miss narrow strips between vertices (a 5m
  // grid over a 2.5m raster left boundary cells keyed to a neighbouring
  // road's blend height -- meters wrong near stacked roads). Decimated by
  // an integer factor only if a very large map would blow the vertex
  // budget.
  constexpr int32 MaxGridVerticesPerAxis = 1024;
  int32 Decimate = 1;
  while (RoadRaster.Width / Decimate > MaxGridVerticesPerAxis ||
         RoadRaster.Height / Decimate > MaxGridVerticesPerAxis)
  {
    ++Decimate;
  }
  const int32 NumX = FMath::Max(2, RoadRaster.Width / Decimate);
  const int32 NumY = FMath::Max(2, RoadRaster.Height / Decimate);
  const float StepX = RoadRaster.CellSize * static_cast<float>(Decimate);
  const float StepY = StepX;
  const float Vertex0X = RoadRaster.OriginX + 0.5f * RoadRaster.CellSize;
  const float Vertex0Y = RoadRaster.OriginY + 0.5f * RoadRaster.CellSize;

  if (!RoadRaster.IsValid())
  {
    UE_LOG(LogCarla, Warning, TEXT("AOpenDriveGenerator: road raster is empty, skipping ground plane"));
    return;
  }

  // Scattered fit points for the free-terrain spline: one per paved raster
  // cell, at the cell center, carrying the lowest surface z recorded there.
  TArray<FVector> FitPoints;
  for (int32 CY = 0; CY < RoadRaster.Height; ++CY)
  {
    for (int32 CX = 0; CX < RoadRaster.Width; ++CX)
    {
      const auto &Cell = RoadRaster.Cells[CY * RoadRaster.Width + CX];
      if (Cell.bPaved)
      {
        FitPoints.Add(FVector(
            RoadRaster.OriginX + (static_cast<float>(CX) + 0.5f) * RoadRaster.CellSize,
            RoadRaster.OriginY + (static_cast<float>(CY) + 0.5f) * RoadRaster.CellSize,
            Cell.PavedMinZ));
      }
    }
  }
  const float SampleCellSize = RoadRaster.CellSize;

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
  const float FinestControlSpacing = FMath::Max(4.0f * SampleCellSize, StepX);
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

  // Evaluate the terrain at each grid vertex from the raster's distance
  // field, by construction rather than by approximation:
  //   - inside the paved footprint: exactly GroundPlaneZOffset below the
  //     lowest surface there (the road demonstrably rests on the ground,
  //     and terrain can never rise through it);
  //   - within GroundSkirtWidth of pavement: blend from the nearest
  //     pavement's underside height out to the free spline surface, so
  //     shoulders stay attached instead of dropping away;
  //   - beyond the skirt: the free C2 spline surface.
  const float SkirtWidth = FMath::Max(GroundSkirtWidth, GroundHeightSampleCellSize);
  auto &CarlaMap = UCarlaStatics::GetGameMode(GetWorld())->GetMap();
  TArray<float> Heights;
  Heights.SetNumUninitialized(NumX * NumY);
  for (int32 IY = 0; IY < NumY; ++IY)
  {
    const float Y = Vertex0Y + static_cast<float>(IY) * StepY;
    for (int32 IX = 0; IX < NumX; ++IX)
    {
      const float X = Vertex0X + static_cast<float>(IX) * StepX;
      float GroundZ;
      const auto *Cell = RoadRaster.CellAtWorld(X, Y);
      // Median / junction gap zone (cells annotated by GenerateMedianFill):
      // loft the terrain flush to the surrounding road surface, so the
      // strip between carriageways reads as level grass instead of a
      // sunken pit. The two flanking carriageways are independent roads
      // whose heights can genuinely differ several dm, so the loft must
      // hug the NEAREST edge tightly and only blend mid-gap -- a gentle
      // average left a shadow ledge along the higher road. Sharp
      // inverse-distance^4 weighting over the genuinely-covered surface
      // samples around the vertex does both.
      float GapWeight = 0.0f;
      float GapZSum = 0.0f;
      bool bGapZone = false;
      const int32 VCellX = FMath::FloorToInt((X - RoadRaster.OriginX) / RoadRaster.CellSize);
      const int32 VCellY = FMath::FloorToInt((Y - RoadRaster.OriginY) / RoadRaster.CellSize);
      for (int32 DY = -1; DY <= 1 && !bGapZone; ++DY)
      {
        for (int32 DX = -1; DX <= 1; ++DX)
        {
          const int32 NX = VCellX + DX;
          const int32 NY = VCellY + DY;
          if (NX >= 0 && NX < RoadRaster.Width && NY >= 0 && NY < RoadRaster.Height &&
              RoadRaster.Cells[NY * RoadRaster.Width + NX].GapSurfaceZ != FLT_MAX)
          {
            bGapZone = true;
            break;
          }
        }
      }
      if (bGapZone)
      {
        for (int32 DY = -3; DY <= 3; ++DY)
        {
          for (int32 DX = -3; DX <= 3; ++DX)
          {
            const int32 NX = VCellX + DX;
            const int32 NY = VCellY + DY;
            if (NX < 0 || NX >= RoadRaster.Width || NY < 0 || NY >= RoadRaster.Height)
            {
              continue;
            }
            const auto &N = RoadRaster.Cells[NY * RoadRaster.Width + NX];
            if (!N.bDriveCovered)
            {
              continue;
            }
            const float CtrX = RoadRaster.OriginX + (NX + 0.5f) * RoadRaster.CellSize;
            const float CtrY = RoadRaster.OriginY + (NY + 0.5f) * RoadRaster.CellSize;
            const float D2 = (FMath::Square(X - CtrX) + FMath::Square(Y - CtrY))
                / FMath::Square(RoadRaster.CellSize);
            const float Weight = 1.0f / (D2 * D2 + 0.01f);
            GapWeight += Weight;
            GapZSum += Weight * N.DriveMinZ;
          }
        }
      }
      // The terrain shares space with two other surfaces near roads: the
      // road mesh itself and the median-fill quads. The fill used to be
      // flat per-cell plates, which forced a 3x (15cm) clearance to stop
      // the 5m terrain triangles from z-fighting through them mid-gap --
      // and that 15cm showed up as a dark ledge under every exposed road
      // edge (junction islands read as sunken pits). The fill is lofted
      // now and fill-adjacent vertices are clamped FillClearance below it
      // explicitly (see below), so the terrain can hug the pavement at
      // the plain offset and exposed edges sit nearly flush.
      const float NearPavementClearance = GroundPlaneZOffset;
      const float HoldDist = 300.0f;
      bool bGapHandled = false;
      if ((bGapZone || (Cell && Cell->bPaved)) && CarlaMap.has_value())
      {
        // Near-pavement vertices: the raster cannot resolve road edges at
        // sub-cell precision -- a 2.5m cell straddling an edge mixes both
        // flanking surfaces (or road and shoulder) in its min, and
        // hugging that min dug a shadow trench along medians and a ledge
        // along outer shoulders. Ask the road map itself: the closest
        // driving waypoint's transform gives the exact local surface
        // height of the nearest carriageway, and its lane width tells
        // whether this vertex is actually under the road at all.
        const FVector Probe(X, Y, Cell ? Cell->NearestPavedMinZ : 0.0f);
        const auto ClosestWp = CarlaMap->GetClosestWaypointOnRoad(carla::geom::Location(Probe));
        if (ClosestWp)
        {
          const FTransform LaneTransform = CarlaMap->ComputeTransform(*ClosestWp);
          const float LaneHalfWidth = 0.5f * 1e2f * static_cast<float>(CarlaMap->GetLaneWidth(*ClosestWp));
          const FVector LaneLoc = LaneTransform.GetLocation();
          const float Dist = FVector::Dist2D(Probe, LaneLoc);
          if (Dist < LaneHalfWidth + 60.0f)
          {
            // Actually under (or overhung by) the road: stay decisively
            // below the cell's lowest surface so the terrain can't poke
            // through a cross-sloped lane's low edge.
            GroundZ = ((Cell && Cell->bPaved) ? Cell->PavedMinZ
                                              : LaneLoc.Z)
                - NearPavementClearance;
          }
          else
          {
            // Between carriageways. "Closest waypoint" measures to the
            // LANE CENTER, ~2m inside the road, so a vertex hugging the
            // higher carriageway's edge can still be assigned the lower
            // road across the gap (a 2dm shadow trench along the median).
            // Blend between the two flanking edges instead: probe again
            // past the vertex, away from the first road, to find the
            // opposite carriageway, and interpolate by edge distance.
            const float EdgeDistA = FMath::Max(Dist - LaneHalfWidth, 1.0f);
            float ZA = LaneLoc.Z;
            float GapZ = ZA;
            FVector2D Away(X - LaneLoc.X, Y - LaneLoc.Y);
            if (Away.Normalize())
            {
              const FVector ProbeB(
                  X + Away.X * 250.0f, Y + Away.Y * 250.0f, Probe.Z);
              const auto WpB = CarlaMap->GetClosestWaypointOnRoad(carla::geom::Location(ProbeB));
              if (WpB)
              {
                const FTransform LaneB = CarlaMap->ComputeTransform(*WpB);
                const float HalfB = 0.5f * 1e2f * static_cast<float>(CarlaMap->GetLaneWidth(*WpB));
                const float EdgeDistB = FMath::Max(
                    FVector::Dist2D(Probe, LaneB.GetLocation()) - HalfB, 1.0f);
                const float Alpha = EdgeDistA / (EdgeDistA + EdgeDistB);
                GapZ = FMath::Lerp(ZA, LaneB.GetLocation().Z, Alpha);
              }
            }
            GroundZ = GapZ - GroundPlaneZOffset;
          }
          bGapHandled = true;
        }
      }
      if (bGapHandled)
      {
        // done
      }
      else if (GapWeight > 0.0f)
      {
        // Fallback loft when the map query fails: sharp inverse-distance
        // weighting hugs the nearest carriageway's samples.
        GroundZ = GapZSum / GapWeight - GroundPlaneZOffset;
      }
      else if (Cell && Cell->bPaved)
      {
        GroundZ = Cell->PavedMinZ - NearPavementClearance;
      }
      else if (Cell && Cell->DistToPaved >= 0.0f && Cell->DistToPaved < SkirtWidth)
      {
        const float Alpha = FMath::Clamp(
            (Cell->DistToPaved - HoldDist) / FMath::Max(SkirtWidth - HoldDist, 1.0f),
            0.0f, 1.0f);
        GroundZ = FMath::Lerp(
            Cell->NearestPavedMinZ - NearPavementClearance,
            EvalSpline(X, Y) - GroundPlaneZOffset,
            Alpha);
      }
      else
      {
        GroundZ = EvalSpline(X, Y) - GroundPlaneZOffset;
      }
      // Final invasion guards over the 3x3 raster neighbourhood:
      //  - discontinuous pavement (a deck sitting >50cm lower: stacked
      //    roads, embankment edges): drop below the lower surface or the
      //    terrain triangle would cut up through it. Continuous
      //    neighbours are exempt -- clamping to a laterally-lower
      //    neighbour on an 8% grade would open a same-size gap under the
      //    vertex's own cell, which is exactly what the QA support check
      //    flags as floating road.
      constexpr float ContinuousDropTolerance = 50.0f;
      const float OwnPavedZ = (Cell && Cell->bPaved) ? Cell->PavedMinZ : FLT_MAX;
      const int32 CellX = FMath::FloorToInt((X - RoadRaster.OriginX) / RoadRaster.CellSize);
      const int32 CellY = FMath::FloorToInt((Y - RoadRaster.OriginY) / RoadRaster.CellSize);
      for (int32 DY = -1; DY <= 1; ++DY)
      {
        for (int32 DX = -1; DX <= 1; ++DX)
        {
          const int32 NX = CellX + DX;
          const int32 NY = CellY + DY;
          if (NX < 0 || NX >= RoadRaster.Width || NY < 0 || NY >= RoadRaster.Height)
          {
            continue;
          }
          const auto &Neighbour = RoadRaster.Cells[NY * RoadRaster.Width + NX];
          if (!Neighbour.bPaved)
          {
            continue;
          }
          const bool bContinuous =
              OwnPavedZ != FLT_MAX &&
              (OwnPavedZ - Neighbour.PavedMinZ) <= ContinuousDropTolerance;
          // Gap-handled vertices already track the flanking carriageway
          // edges exactly (waypoint blend); clamping them below every
          // neighbouring cell's min re-dug the median trench, because a
          // straddle cell's PavedMinZ mixes the LOWER road's samples.
          // Only a genuinely lower deck (well below the blend) still
          // clamps, so medians can never bury an underpass.
          const bool bGenuinelyLowerDeck =
              Neighbour.PavedMinZ < GroundZ - ContinuousDropTolerance;
          if (!bContinuous && (!bGapHandled || bGenuinelyLowerDeck))
          {
            GroundZ = FMath::Min(GroundZ, Neighbour.PavedMinZ - GroundPlaneZOffset);
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
  GroundGridOrigin = FVector2D(Vertex0X, Vertex0Y);
  GroundGridStepX = StepX;
  GroundGridStepY = StepY;

  FProceduralCustomMesh MeshData;
  MeshData.Vertices.Reserve(NumX * NumY);
  MeshData.UV0.Reserve(NumX * NumY);
  const float UVTile = FMath::Max(GroundUVTileSize, 1.0f);
  for (int32 IY = 0; IY < NumY; ++IY)
  {
    const float Y = Vertex0Y + static_cast<float>(IY) * StepY;
    for (int32 IX = 0; IX < NumX; ++IX)
    {
      const float X = Vertex0X + static_cast<float>(IX) * StepX;
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

void AOpenDriveGenerator::GenerateMedianFill()
{
  if (!RoadRaster.IsValid())
  {
    return;
  }

  // Morphological closing of the driving footprint: dilate by K cells,
  // erode by K. Closes any gap narrower than ~2K cells regardless of
  // corridor orientation and produces a connected ribbon by construction.
  // (A previous per-axis flanking scan qualified cells alternately along
  // diagonal medians, perforating the fill into a checkerboard.)
  const int32 W = RoadRaster.Width;
  const int32 H = RoadRaster.Height;
  const int32 K = FMath::Max(1, FMath::CeilToInt(0.5f * MedianFillMaxWidth / RoadRaster.CellSize));
  TArray<uint8> Mask;
  Mask.SetNumZeroed(W * H);
  for (int32 Idx = 0; Idx < W * H; ++Idx)
  {
    // Strict coverage: cells that only grazing triangles claimed (bDrive
    // via the rasterizer's tolerance) are mostly bare and must count as
    // gap, or the ribbon skips them and leaves holes in the median.
    Mask[Idx] = RoadRaster.Cells[Idx].bDriveCovered ? 1 : 0;
  }
  const auto Morph = [&](uint8 Grow)
  {
    TArray<uint8> Out = Mask;
    for (int32 CY = 0; CY < H; ++CY)
    {
      for (int32 CX = 0; CX < W; ++CX)
      {
        if (Mask[CY * W + CX] == Grow)
        {
          continue;
        }
        for (int32 DY = -1; DY <= 1; ++DY)
        {
          for (int32 DX = -1; DX <= 1; ++DX)
          {
            const int32 NX = CX + DX;
            const int32 NY = CY + DY;
            if (NX >= 0 && NX < W && NY >= 0 && NY < H && Mask[NY * W + NX] == Grow)
            {
              Out[CY * W + CX] = Grow;
              DY = 2;
              break;
            }
          }
        }
      }
    }
    Mask = MoveTemp(Out);
  };
  for (int32 I = 0; I < K; ++I) { Morph(1); } // dilate
  for (int32 I = 0; I < K; ++I) { Morph(0); } // erode

  // Reduce to the cells the closing *added* (the gap ribbon), then grow
  // that ribbon by one cell. The driving footprint's rasterized edge is
  // stairstepped on diagonals and the erosion bites back into it,
  // perforating the ribbon with alternating holes; one dilation of the
  // ribbon alone patches those notches (the overlap lands under the road
  // mesh or 1cm below it) without laying an apron around every outer road
  // edge the way an extra dilation of the whole footprint would.
  for (int32 Idx = 0; Idx < W * H; ++Idx)
  {
    Mask[Idx] = (Mask[Idx] && !RoadRaster.Cells[Idx].bDriveCovered) ? 1 : 0;
  }
  Morph(1);


  // Per-grid-node road-surface height, lofted from the surrounding
  // driving surface samples so the estimate follows the corridor's grade
  // and cross-slope across the gap.
  TMap<int32, float> NodeZCache;
  const auto NodeZ = [&](int32 NX, int32 NY) -> float
  {
    const int32 Key = NY * (W + 1) + NX;
    if (const float *Found = NodeZCache.Find(Key))
    {
      return *Found;
    }
    // 1) Cells genuinely covered by road that touch this node: hug the
    //    lowest of their surface samples.
    float Result = FLT_MAX;
    float TouchMin = FLT_MAX;
    for (int32 DY = -1; DY <= 0; ++DY)
    {
      for (int32 DX = -1; DX <= 0; ++DX)
      {
        const int32 CX = NX + DX;
        const int32 CY = NY + DY;
        if (CX < 0 || CX >= W || CY < 0 || CY >= H)
        {
          continue;
        }
        const auto &N = RoadRaster.Cells[CY * W + CX];
        if (N.bDriveCovered)
        {
          TouchMin = FMath::Min(TouchMin, N.DriveMinZ);
        }
      }
    }
    if (TouchMin != FLT_MAX)
    {
      Result = TouchMin;
    }
    else
    {
      // 2) Inverse-distance-weighted loft over covered cells nearby: the
      //    node sits mid-gap, interpolate the two road edges smoothly.
      float WeightSum = 0.0f;
      float ZSum = 0.0f;
      for (int32 DY = -3; DY <= 2; ++DY)
      {
        for (int32 DX = -3; DX <= 2; ++DX)
        {
          const int32 CX = NX + DX;
          const int32 CY = NY + DY;
          if (CX < 0 || CX >= W || CY < 0 || CY >= H)
          {
            continue;
          }
          const auto &N = RoadRaster.Cells[CY * W + CX];
          if (!N.bDriveCovered)
          {
            continue;
          }
          const float DCX = static_cast<float>(DX) + 0.5f;
          const float DCY = static_cast<float>(DY) + 0.5f;
          const float Weight = 1.0f / (DCX * DCX + DCY * DCY);
          WeightSum += Weight;
          ZSum += Weight * N.DriveMinZ;
        }
      }
      if (WeightSum > 0.0f)
      {
        Result = ZSum / WeightSum;
      }
    }
    NodeZCache.Add(Key, Result);
    return Result;
  };

  int32 GapCells = 0;
  for (int32 CY = 0; CY < H; ++CY)
  {
    for (int32 CX = 0; CX < W; ++CX)
    {
      if (!Mask[CY * W + CX])
      {
        continue;
      }
      // Height guard: a gap under an overpass sees both decks. Two decks
      // land in the SAME cell (its DriveMinZ/DriveMaxZ split apart), so
      // test per cell -- a window-wide min/max spread also swallows the
      // corridor's own grade and skipped legitimate median cells on
      // sloped roads.
      bool bOverpass = false;
      for (int32 DY = -K - 1; DY <= K + 1 && !bOverpass; ++DY)
      {
        for (int32 DX = -K - 1; DX <= K + 1; ++DX)
        {
          const int32 NX = CX + DX;
          const int32 NY = CY + DY;
          if (NX < 0 || NX >= W || NY < 0 || NY >= H)
          {
            continue;
          }
          const auto &N = RoadRaster.Cells[NY * W + NX];
          if (N.bDrive && (N.DriveMaxZ - N.DriveMinZ) > MedianFillMaxHeightDelta)
          {
            bOverpass = true;
            break;
          }
        }
      }
      if (bOverpass)
      {
        continue;
      }
      // Lofted surface height at the cell center (mean of the four corner
      // nodes). GenerateGroundPlane raises the terrain flush to this, so
      // the median presents as grass level with the road edges instead of
      // asphalt quads (which hovered as floating plates wherever the gap
      // widened and the free terrain fell away underneath) or a sunken
      // pit (the old 15cm clearance ledge).
      const float Z00 = NodeZ(CX, CY);
      const float Z10 = NodeZ(CX + 1, CY);
      const float Z11 = NodeZ(CX + 1, CY + 1);
      const float Z01 = NodeZ(CX, CY + 1);
      if (Z00 == FLT_MAX || Z10 == FLT_MAX || Z11 == FLT_MAX || Z01 == FLT_MAX)
      {
        continue; // no driving surface anywhere near -- nothing to loft from
      }
      RoadRaster.Cells[CY * W + CX].GapSurfaceZ =
          0.25f * (Z00 + Z10 + Z11 + Z01);
      ++GapCells;
    }
  }
  UE_LOG(LogCarla, Log,
      TEXT("AOpenDriveGenerator: %d median gap cells lofted for the terrain"), GapCells);
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

  // Zebra bars built from the crosswalk outline polygons instead of one
  // texture-mapped cover slab. The only crosswalk-ish texture in the
  // content is the speed-bump set (red field + stripe band), which rendered
  // the slabs as pink; real bar geometry with the same clean white
  // marking material the lane markings use needs no texture at all and
  // scales to any crosswalk size. The road surface below stays visible
  // between the bars, exactly like the painted original.
  auto& CarlaMap = UCarlaStatics::GetGameMode(GetWorld())->GetMap();
  const std::vector<carla::geom::Location> Zones = CarlaMap->GetAllCrosswalkZones();
  if (Zones.empty())
  {
    return;
  }

  constexpr float BarWidth = 0.4f;   // m, along the crossing direction
  constexpr float BarGap = 0.4f;     // m, between bars
  constexpr float EdgeMargin = 0.1f; // m, kept clear along the bar ends

  FProceduralCustomMesh MeshData;

  auto AddZebraPolygon = [&MeshData](const std::vector<carla::geom::Location> &Poly, float ZOffsetCm)
  {
    if (Poly.size() < 3)
    {
      return;
    }
    // Local frame: U along the longest edge (the crossing direction), V
    // across it (the crosswalk's depth along the road).
    size_t Longest = 0;
    float LongestLen2 = -1.0f;
    for (size_t k = 0; k < Poly.size(); ++k)
    {
      const auto &P = Poly[k];
      const auto &Q = Poly[(k + 1) % Poly.size()];
      const float DX = Q.x - P.x, DY = Q.y - P.y;
      const float Len2 = DX * DX + DY * DY;
      if (Len2 > LongestLen2)
      {
        LongestLen2 = Len2;
        Longest = k;
      }
    }
    if (LongestLen2 < 1e-4f)
    {
      return;
    }
    const float InvLen = 1.0f / FMath::Sqrt(LongestLen2);
    const FVector2D U((Poly[(Longest + 1) % Poly.size()].x - Poly[Longest].x) * InvLen,
                      (Poly[(Longest + 1) % Poly.size()].y - Poly[Longest].y) * InvLen);
    const FVector2D V(-U.Y, U.X);

    struct FLocalPt { float A, B, Z; };
    TArray<FLocalPt> Ring;
    float AMin = FLT_MAX, AMax = -FLT_MAX, BMin = FLT_MAX, BMax = -FLT_MAX;
    for (const auto &P : Poly)
    {
      const float A = P.x * U.X + P.y * U.Y;
      const float B = P.x * V.X + P.y * V.Y;
      Ring.Add({A, B, P.z});
      AMin = FMath::Min(AMin, A); AMax = FMath::Max(AMax, A);
      BMin = FMath::Min(BMin, B); BMax = FMath::Max(BMax, B);
    }

    // Surface height at a local point: barycentric over the outline's
    // triangle fan (the corners already carry the road elevation at their
    // own s -- see Map::GetAllCrosswalkZones); nearest corner as fallback.
    auto SurfaceZ = [&Ring](float A, float B) -> float
    {
      for (int32 K = 1; K + 1 < Ring.Num(); ++K)
      {
        const FLocalPt &P0 = Ring[0], &P1 = Ring[K], &P2 = Ring[K + 1];
        const float Denom = (P1.B - P2.B) * (P0.A - P2.A) + (P2.A - P1.A) * (P0.B - P2.B);
        if (FMath::Abs(Denom) < 1e-9f)
        {
          continue;
        }
        const float W0 = ((P1.B - P2.B) * (A - P2.A) + (P2.A - P1.A) * (B - P2.B)) / Denom;
        const float W1 = ((P2.B - P0.B) * (A - P2.A) + (P0.A - P2.A) * (B - P2.B)) / Denom;
        const float W2 = 1.0f - W0 - W1;
        if (W0 >= -1e-3f && W1 >= -1e-3f && W2 >= -1e-3f)
        {
          return W0 * P0.Z + W1 * P1.Z + W2 * P2.Z;
        }
      }
      float Best = FLT_MAX, Z = Ring[0].Z;
      for (const FLocalPt &P : Ring)
      {
        const float D = FMath::Square(P.A - A) + FMath::Square(P.B - B);
        if (D < Best) { Best = D; Z = P.Z; }
      }
      return Z;
    };

    // Sutherland-Hodgman clip of one bar rectangle against the outline
    // (crosswalk outlines are rectangles, so convex clipping is exact).
    auto ClipBar = [&Ring](TArray<FVector2D> Subject) -> TArray<FVector2D>
    {
      for (int32 K = 0; K < Ring.Num(); ++K)
      {
        const FVector2D E0(Ring[K].A, Ring[K].B);
        const FVector2D E1(Ring[(K + 1) % Ring.Num()].A, Ring[(K + 1) % Ring.Num()].B);
        const FVector2D Edge = E1 - E0;
        // Ring orientation is unknown; use the ring centroid to pick the
        // inward side of each edge.
        FVector2D Centroid(0.0f, 0.0f);
        for (const FLocalPt &P : Ring) { Centroid += FVector2D(P.A, P.B); }
        Centroid /= static_cast<float>(Ring.Num());
        const float CentroidSide = Edge.X * (Centroid.Y - E0.Y) - Edge.Y * (Centroid.X - E0.X);
        const float Sign = CentroidSide >= 0.0f ? 1.0f : -1.0f;
        TArray<FVector2D> Out;
        for (int32 I = 0; I < Subject.Num(); ++I)
        {
          const FVector2D &P = Subject[I];
          const FVector2D &Q = Subject[(I + 1) % Subject.Num()];
          const float SideP = Sign * (Edge.X * (P.Y - E0.Y) - Edge.Y * (P.X - E0.X));
          const float SideQ = Sign * (Edge.X * (Q.Y - E0.Y) - Edge.Y * (Q.X - E0.X));
          if (SideP >= 0.0f)
          {
            Out.Add(P);
          }
          if ((SideP >= 0.0f) != (SideQ >= 0.0f))
          {
            const float T = SideP / (SideP - SideQ);
            Out.Add(P + T * (Q - P));
          }
        }
        Subject = MoveTemp(Out);
        if (Subject.Num() < 3)
        {
          return {};
        }
      }
      return Subject;
    };

    const float Extent = AMax - AMin;
    const int32 NumBars = FMath::Max(1, FMath::FloorToInt((Extent - BarWidth) / (BarWidth + BarGap)) + 1);
    const float Start = AMin + 0.5f * (Extent - (NumBars * (BarWidth + BarGap) - BarGap));
    for (int32 Bar = 0; Bar < NumBars; ++Bar)
    {
      const float A0 = Start + Bar * (BarWidth + BarGap);
      const float A1 = A0 + BarWidth;
      TArray<FVector2D> Quad;
      Quad.Add(FVector2D(A0, BMin + EdgeMargin));
      Quad.Add(FVector2D(A1, BMin + EdgeMargin));
      Quad.Add(FVector2D(A1, BMax - EdgeMargin));
      Quad.Add(FVector2D(A0, BMax - EdgeMargin));
      TArray<FVector2D> Clipped = ClipBar(MoveTemp(Quad));
      if (Clipped.Num() < 3)
      {
        continue;
      }
      const int32 First = MeshData.Vertices.Num();
      for (const FVector2D &L : Clipped)
      {
        const float X = L.X * U.X + L.Y * V.X;
        const float Y = L.X * U.Y + L.Y * V.Y;
        const float Z = SurfaceZ(L.X, L.Y);
        MeshData.Vertices.Add(FVector(1e2f * X, 1e2f * Y, 1e2f * Z + ZOffsetCm));
        MeshData.Normals.Add(FVector::UpVector);
        MeshData.UV0.Add(FVector2D((L.X - A0) / BarWidth, (L.Y - BMin) / FMath::Max(BMax - BMin, 0.1f)));
      }
      for (int32 K = 1; K + 1 < Clipped.Num(); ++K)
      {
        // Same winding convention as the converted LibCarla meshes
        // (flip the last two indices for Unreal's left-handed frame).
        MeshData.Triangles.Add(First);
        MeshData.Triangles.Add(First + K + 1);
        MeshData.Triangles.Add(First + K);
      }
    }
  };

  // Split the flat vertex list into outline polygons: each polygon ends
  // when its first vertex repeats (same convention as GetAllCrosswalkMesh).
  std::vector<carla::geom::Location> Current;
  size_t StartIdx = 0;
  for (size_t i = 0; i < Zones.size(); ++i)
  {
    if (i != StartIdx && Zones[StartIdx] == Zones[i])
    {
      AddZebraPolygon(Current, DecalZOffset);
      Current.clear();
      if (i >= Zones.size() - 1)
      {
        break;
      }
      StartIdx = i + 1;
      continue;
    }
    Current.push_back(Zones[i]);
  }

  if (MeshData.Vertices.Num() == 0)
  {
    return;
  }

  ACarlaProceduralMeshActor* TempActor = GetWorld()->SpawnActor<ACarlaProceduralMeshActor>();
  UProceduralMeshComponent *TempPMC = TempActor->MeshComponent;
  TempPMC->bUseAsyncCooking = true;
  TempPMC->bUseComplexAsSimpleCollision = true;
  // Markings are paint: the road mesh below carries the physics. Thin bar
  // meshes with collision would only add pointless micro-ledges.
  TempPMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  TempPMC->CreateMeshSection_LinearColor(
      0,
      MeshData.Vertices,
      MeshData.Triangles,
      MeshData.Normals,
      MeshData.UV0,
      TArray<FLinearColor>(), // VertexColor
      TArray<FProcMeshTangent>(), // Tangents
      false); // no collision

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

int32 AOpenDriveGenerator::GenerateFurnitureAnchors(const FName &Tag, float Spacing, float Offset, float RoadClearance)
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
    // spawned furniture then blocks traffic. The anchor is a point but the
    // content it spawns is not, so the required clearance is the content's
    // bounding-box half-extent (per category), measured against the
    // rasterized driving footprint -- the actual mesh, junction fans and
    // widenings included, not a centerline heuristic.
    const auto *Cell = RoadRaster.CellAtWorld(Location.X, Location.Y);
    if (Cell && Cell->DistToDrive >= 0.0f && Cell->DistToDrive < RoadClearance)
    {
      ++NumFilteredOnRoad;
      continue;
    }
    // Belt-and-braces centerline check for lanes whose mesh is missing
    // from the raster (some imported maps skip generating a driving
    // lane's geometry; the lane is still drivable).
    const auto ClosestWp = CarlaMap->GetClosestWaypointOnRoad(carla::geom::Location(Location));
    if (ClosestWp)
    {
      const FTransform LaneTransform = CarlaMap->ComputeTransform(*ClosestWp);
      const float LaneHalfWidth = 0.5f * 1e2f * static_cast<float>(CarlaMap->GetLaneWidth(*ClosestWp));
      if (FVector::Dist2D(Location, LaneTransform.GetLocation()) < LaneHalfWidth + RoadClearance)
      {
        ++NumFilteredOnRoad;
        continue;
      }
    }

    // Re-ground: the anchor inherits the road-edge elevation from
    // GetTreesTransform, but it stands offset onto pavement or terrain
    // that is generally lower (ground offset plus lateral grade), leaving
    // spawned furniture floating. If the anchor's raster cell saw road/
    // sidewalk geometry, snap to that surface; otherwise drop it onto the
    // ground heightfield generated by GenerateGroundPlane.
    if (Cell && Cell->bPaved)
    {
      Location.Z = Cell->PavedMinZ;
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

  const int32 NumLampAnchors = GenerateFurnitureAnchors(LampAnchorTag, LampAnchorSpacing, LampAnchorOffset, LampRoadClearance);
  const int32 NumVegetationAnchors = GenerateFurnitureAnchors(VegetationAnchorTag, VegetationAnchorSpacing, VegetationAnchorOffset, VegetationRoadClearance);
  const int32 NumSignageAnchors = GenerateFurnitureAnchors(SignageAnchorTag, SignageAnchorSpacing, SignageAnchorOffset, SignageRoadClearance);
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
  GenerateMedianFill();
  GenerateGroundPlane();
  GenerateCrosswalkMesh();
  GenerateLaneMarkings();
  GenerateSpawnPoints();
  GeneratePoles();

  // Deferred so the asynchronous PCG scatter has spawned its content
  // before the object-placement pass measures it.
  FTimerHandle QATimerHandle;
  GetWorldTimerManager().SetTimer(
      QATimerHandle, this, &AOpenDriveGenerator::RunGenerationQA, 3.0f, false);
}

void AOpenDriveGenerator::RunGenerationQA()
{
  if (!RoadRaster.IsValid() || GroundGridNumX < 2 || GroundGridNumY < 2)
  {
    UE_LOG(LogCarla, Warning, TEXT("MapGenQA: no generated map data to check"));
    return;
  }

  constexpr int32 MaxReportedLocations = 50;
  const auto AppendLoc = [](FString &Out, int32 &Count, float X, float Y, float Z, const TCHAR *Extra = nullptr)
  {
    if (Count >= MaxReportedLocations)
    {
      return;
    }
    if (!Out.IsEmpty())
    {
      Out += TEXT(",");
    }
    // Meters, CARLA client frame, directly usable to move the spectator.
    Out += FString::Printf(TEXT("{\"x\":%.1f,\"y\":%.1f,\"z\":%.1f%s%s}"),
        X / 100.0f, Y / 100.0f, Z / 100.0f,
        Extra ? TEXT(",") : TEXT(""), Extra ? Extra : TEXT(""));
    ++Count;
  };

  // --- A) road support / terrain invasion: every paved raster cell ---
  int32 PavedCells = 0;
  int32 FloatingCells = 0;
  int32 InvadedCells = 0;
  int32 StackedCells = 0;
  int32 BridgeEdgeCells = 0;
  float WorstGap = 0.0f;
  float WorstInvasion = 0.0f;
  FString FloatingLocs, InvadedLocs;
  int32 FloatingLocCount = 0, InvadedLocCount = 0;
  for (int32 CY = 0; CY < RoadRaster.Height; ++CY)
  {
    for (int32 CX = 0; CX < RoadRaster.Width; ++CX)
    {
      const auto &Cell = RoadRaster.Cells[CY * RoadRaster.Width + CX];
      if (!Cell.bPaved)
      {
        continue;
      }
      ++PavedCells;
      if (Cell.bDrive && (Cell.DriveMaxZ - Cell.DriveMinZ) > QAStackedRoadGap)
      {
        ++StackedCells; // upper deck is a bridge; only the lower is checked
      }
      // A cell whose 3x3 neighbourhood contains pavement a deck lower is a
      // bridge/embankment edge: one terrain surface cannot simultaneously
      // support this deck and stay below the lower one, so a support gap
      // here is structural (wants bridge skirt geometry), not a terrain
      // defect -- tally it separately from genuine floating.
      bool bBridgeEdge = false;
      bool bBoundary = false;
      for (int32 DY = -1; DY <= 1; ++DY)
      {
        for (int32 DX = -1; DX <= 1; ++DX)
        {
          const int32 NX = CX + DX;
          const int32 NY = CY + DY;
          if (NX < 0 || NX >= RoadRaster.Width || NY < 0 || NY >= RoadRaster.Height)
          {
            bBoundary = true;
            continue;
          }
          const auto &Neighbour = RoadRaster.Cells[NY * RoadRaster.Width + NX];
          if (!Neighbour.bPaved)
          {
            bBoundary = true;
          }
          else if ((Cell.PavedMinZ - Neighbour.PavedMinZ) > 50.0f)
          {
            bBridgeEdge = true;
          }
        }
      }
      const float X = RoadRaster.OriginX + (static_cast<float>(CX) + 0.5f) * RoadRaster.CellSize;
      const float Y = RoadRaster.OriginY + (static_cast<float>(CY) + 0.5f) * RoadRaster.CellSize;
      const float Gap = Cell.PavedMinZ - SampleGroundGridHeight(X, Y);
      if (Gap > QASupportGapMax)
      {
        if (bBridgeEdge)
        {
          ++BridgeEdgeCells;
        }
        else if (bBoundary)
        {
          // Only footprint-boundary cells can show visible daylight under a
          // road edge. Interior cells sit under an intact road mesh and the
          // terrain deliberately holds below the pavement -- a deep
          // interior gap there is construction, not a defect.
          ++FloatingCells;
          WorstGap = FMath::Max(WorstGap, Gap);
          AppendLoc(FloatingLocs, FloatingLocCount, X, Y, Cell.PavedMinZ);
        }
      }
      else if (Gap < -1.0f)
      {
        ++InvadedCells;
        WorstInvasion = FMath::Max(WorstInvasion, -Gap);
        AppendLoc(InvadedLocs, InvadedLocCount, X, Y, Cell.PavedMinZ);
      }
    }
  }

  // --- B) objects on the roadway: BB footprints vs driving footprint ---
  int32 ObjectsChecked = 0;
  int32 ObjectViolations = 0;
  FString ObjectLocs;
  int32 ObjectLocCount = 0;
  // Pivot-based test: an object stands *in* the roadway when its support
  // point (tree trunk, lamp pole, shelter structure -- the placed
  // transform's origin) is on or within a meter of the driving footprint
  // at road level. Bounding boxes are deliberately not used for the
  // verdict: a lamp arm or tree canopy legitimately overhangs the lanes
  // meters above traffic, and its BB reaches the ground via the off-road
  // pole/trunk, which made every mature roadside tree a false positive.
  const auto PivotBlocksRoad = [&](const FVector &Pivot) -> bool
  {
    const auto *Cell = RoadRaster.CellAtWorld(Pivot.X, Pivot.Y);
    if (!Cell)
    {
      return false;
    }
    const bool bOnFootprint =
        Cell->bDrive || (Cell->DistToDrive >= 0.0f && Cell->DistToDrive < 100.0f);
    // z gate: only content at road level counts -- furniture on a deck
    // above or below an overpass shares XY with the lanes legitimately.
    return bOnFootprint && FMath::Abs(Pivot.Z - Cell->NearestPavedMinZ) < 500.0f;
  };
  const auto IsExemptOwner = [](const AActor *Owner) -> bool
  {
    // Roads/terrain/markings are this generator's own output (identified
    // by component tags below); traffic signals legitimately stand at the
    // roadside per the OpenDRIVE data; spawn points and pawns are not
    // scenery. The sky sphere's bounds span the whole map -- size-gated.
    return Owner == nullptr
        || Owner->IsA<ATrafficSignBase>()
        || Owner->IsA<AVehicleSpawnPoint>()
        || Owner->IsA<APawn>();
  };
  for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
  {
    UStaticMeshComponent *Comp = *It;
    if (!IsValid(Comp) || Comp->GetWorld() != GetWorld() || !Comp->GetStaticMesh())
    {
      continue;
    }
    if (IsExemptOwner(Comp->GetOwner()))
    {
      continue;
    }
    const auto CheckOnePivot = [&](const FVector &Pivot)
    {
      ++ObjectsChecked;
      if (PivotBlocksRoad(Pivot))
      {
        ++ObjectViolations;
        const FString Extra = FString::Printf(TEXT("\"class\":\"%s\""),
            *Comp->GetOwner()->GetClass()->GetName());
        AppendLoc(ObjectLocs, ObjectLocCount, Pivot.X, Pivot.Y, Pivot.Z, *Extra);
      }
    };
    if (const UInstancedStaticMeshComponent *ISM = Cast<UInstancedStaticMeshComponent>(Comp))
    {
      for (int32 Inst = 0; Inst < ISM->GetInstanceCount(); ++Inst)
      {
        FTransform InstTransform;
        if (ISM->GetInstanceTransform(Inst, InstTransform, /*bWorldSpace=*/true))
        {
          CheckOnePivot(InstTransform.GetLocation());
        }
      }
    }
    else
    {
      // Size gate: skip map-spanning geometry (sky sphere & co).
      if (Comp->Bounds.GetBox().GetExtent().X > 50000.0f ||
          Comp->Bounds.GetBox().GetExtent().Y > 50000.0f)
      {
        continue;
      }
      CheckOnePivot(Comp->GetComponentLocation());
    }
  }

  UE_LOG(LogCarla, Log,
      TEXT("MapGenQA: paved cells %d (stacked %d, bridge-edge %d) | floating %d (worst gap %.0fcm) | ")
      TEXT("terrain invasion %d (worst %.0fcm) | objects checked %d, on roadway %d"),
      PavedCells, StackedCells, BridgeEdgeCells, FloatingCells, WorstGap,
      InvadedCells, WorstInvasion, ObjectsChecked, ObjectViolations);

  const FString Report = FString::Printf(
      TEXT("{\n")
      TEXT("  \"support\": {\"paved_cells\": %d, \"stacked_cells\": %d, \"bridge_edge_cells\": %d, ")
      TEXT("\"floating_cells\": %d, \"worst_gap_cm\": %.1f, \"floating_locations\": [%s]},\n")
      TEXT("  \"invasion\": {\"cells\": %d, \"worst_cm\": %.1f, \"locations\": [%s]},\n")
      TEXT("  \"objects\": {\"checked\": %d, \"on_roadway\": %d, \"items\": [%s]}\n")
      TEXT("}\n"),
      PavedCells, StackedCells, BridgeEdgeCells, FloatingCells, WorstGap, *FloatingLocs,
      InvadedCells, WorstInvasion, *InvadedLocs,
      ObjectsChecked, ObjectViolations, *ObjectLocs);
  const FString ReportPath = FPaths::ProjectSavedDir() / TEXT("mapgen_qa.json");
  FFileHelper::SaveStringToFile(Report, *ReportPath);
  UE_LOG(LogCarla, Log, TEXT("MapGenQA: report written to %s"), *ReportPath);
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
