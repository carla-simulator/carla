// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "OpenDriveGenerator.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <compiler/enable-ue4-macros.h>

#include "Engine/Classes/Interfaces/Interface_CollisionDataProvider.h"
#include "PhysicsEngine/BodySetupEnums.h"
#include "ProceduralMeshComponent.h"

AOpenDriveGenerator::AOpenDriveGenerator(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
  RootComponent->Mobility = EComponentMobility::Static;

  RoadMesh = CreateDefaultSubobject<UProceduralMeshComponent>("RoadMesh");
  SetRootComponent(RoadMesh);
  RoadMesh->bUseAsyncCooking = true;
  RoadMesh->bUseComplexAsSimpleCollision = true;
  RoadMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

bool AOpenDriveGenerator::LoadOpenDrive(const FString &OpenDrive)
{
  using OpenDriveLoader = carla::opendrive::OpenDriveParser;

  if (OpenDrive.IsEmpty())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive is empty"));
    return false;
  }

  CarlaMap = OpenDriveLoader::Load(carla::rpc::FromFString(OpenDrive));
  if (!CarlaMap.has_value())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive is invalid or not supported"));
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
  return CarlaMap.has_value();
}

void AOpenDriveGenerator::GenerateRoadMesh()
{
  if (!IsOpenDriveValid())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive has not been loaded"));
    return;
  }

  const auto MeshData = CarlaMap->GenerateGeometry(2);

  // Build the mesh
  TArray<FVector> Vertices;
  for (const auto vertex : MeshData.GetVertices())
  {
    // From meters to centimeters
    Vertices.Add(FVector(vertex.x, vertex.y, vertex.z) * 1e2f);
  }

  const auto Indexes = MeshData.GetIndexes();
  TArray<int32> Triangles;
  TArray<FTriIndices> TriIndices;
  FTriIndices Triangle;
  for (auto i = 0u; i < Indexes.size(); i += 3)
  {
    // "-1" since mesh indexes in Unreal starts from index 0.
    Triangles.Add(Indexes[i]     - 1);
    // Since Unreal's coords are left handed, invert the last 2 indices.
    Triangles.Add(Indexes[i + 2] - 1);
    Triangles.Add(Indexes[i + 1] - 1);

    Triangle.v0 = Indexes[i]     - 1;
    Triangle.v1 = Indexes[i + 2] - 1;
    Triangle.v2 = Indexes[i + 1] - 1;
    TriIndices.Add(Triangle);
  }

  RoadMesh->CreateMeshSection_LinearColor(
      0,
      Vertices,
      Triangles,
      TArray<FVector>(),
      TArray<FVector2D>(),
      TArray<FLinearColor>(),
      TArray<FProcMeshTangent>(),
      true);

  // Build collision data
  FTriMeshCollisionData CollisitonData;
  CollisitonData.bDeformableMesh = false;
  CollisitonData.bDisableActiveEdgePrecompute = false;
  CollisitonData.bFastCook = false;
  CollisitonData.bFlipNormals = false;
  CollisitonData.Indices = TriIndices;
  CollisitonData.Vertices = Vertices;

  RoadMesh->ContainsPhysicsTriMeshData(true);
  bool Success = RoadMesh->GetPhysicsTriMeshData(&CollisitonData, true);
  if (!Success)
  {
    UE_LOG(LogCarla, Error, TEXT("The road collision mesh could not be generated!"));
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
  GenerateSpawnPoints();
  GeneratePoles();
}

void AOpenDriveGenerator::BeginPlay()
{
  Super::BeginPlay();

  // Search for "{project_content_folder}/Carla/Maps/OpenDrive/{current_map_name}.xodr"
  const FString XodrContent = UOpenDrive::LoadXODR(GetWorld()->GetMapName());
  LoadOpenDrive(XodrContent);

  GenerateAll();
}
