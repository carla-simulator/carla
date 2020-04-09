// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "OpenDriveGenerator.h"
#include "Traffic/TrafficLightManager.h"
#include "Util/ProceduralCustomMesh.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <compiler/enable-ue4-macros.h>

#include "Engine/Classes/Interfaces/Interface_CollisionDataProvider.h"
#include "PhysicsEngine/BodySetupEnums.h"

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

  static const FString ConfigFilePath =
      FPaths::ProjectContentDir() + "Carla/Maps/OpenDrive/OpenDriveMap.conf";
  float Resolution = 2.f;
  float MaxRoadLength = 50.0f;
  float WallHeight = 1.f;
  float AdditionalWidth = .6f;
  bool MeshVisibility = true;
  if (FPaths::FileExists(ConfigFilePath)) {
    FString ConfigData;
    TArray<FString> Lines;
    FFileHelper::LoadFileToString(ConfigData, *ConfigFilePath);
    ConfigData.ParseIntoArray(Lines, TEXT("\n"), true);
    for (const FString &Line : Lines) {
      FString Key, Value;
      Line.Split(TEXT("="), &Key, &Value);
      if (Key == "resolution")
      {
        Resolution = FCString::Atof(*Value);
      }
      else if (Key == "wall_height")
      {
        WallHeight = FCString::Atof(*Value);
      }
      else if (Key == "max_road_length")
      {
        MaxRoadLength = FCString::Atof(*Value);
      }
      else if (Key == "additional_width")
      {
        AdditionalWidth = FCString::Atof(*Value);
      }
      else if (Key == "mesh_visibility")
      {
        MeshVisibility = Value.ToBool();
      }
    }
  }

  const auto Meshes = CarlaMap->GenerateChunkedMesh(
      Resolution, MaxRoadLength, AdditionalWidth);
  for (const auto &Mesh : Meshes) {
    AActor *TempActor = GetWorld()->SpawnActor<AActor>();
    UProceduralMeshComponent *TempPMC = NewObject<UProceduralMeshComponent>(TempActor);
    TempPMC->RegisterComponent();
    TempPMC->AttachToComponent(
        TempActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    TempPMC->bUseAsyncCooking = true;
    TempPMC->bUseComplexAsSimpleCollision = true;
    TempPMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    const FProceduralCustomMesh MeshData = *Mesh;
    TempPMC->CreateMeshSection_LinearColor(
        0,
        MeshData.Vertices,
        MeshData.Triangles,
        MeshData.Normals,
        TArray<FVector2D>(), // UV0
        TArray<FLinearColor>(), // VertexColor
        TArray<FProcMeshTangent>(), // Tangents
        true); // Create collision

    ActorMeshList.Add(TempActor);
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

  // Autogenerate signals
  GetWorld()->SpawnActor<ATrafficLightManager>();
}
