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

AProceduralMeshActor::AProceduralMeshActor()
{
  PrimaryActorTick.bCanEverTick = false;
  MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RootComponent"));
  RootComponent = MeshComponent;
}

AOpenDriveGenerator::AOpenDriveGenerator(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
  SetRootComponent(RootComponent);
  RootComponent->Mobility = EComponentMobility::Static;
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

      AProceduralMeshActor* TempActor = GetWorld()->SpawnActor<AProceduralMeshActor>();
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

  AProceduralMeshActor* TempActor = GetWorld()->SpawnActor<AProceduralMeshActor>();
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

    AProceduralMeshActor* TempActor = GetWorld()->SpawnActor<AProceduralMeshActor>();
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

void AOpenDriveGenerator::GeneratePoles()
{
  if (!IsOpenDriveValid())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive has not been loaded"));
    return;
  }
  /// TODO: To implement
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
