// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "OpenDriveGenerator.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla.h"
#include "Traffic/TrafficLightManager.h"
#include "Util/ProceduralCustomMesh.h"

#include <util/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <util/enable-ue4-macros.h>

// #include "Engine/Classes/Interfaces/Interface_CollisionDataProvider.h"
// #include "PhysicsCore/Public/BodySetupEnums.h"

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
  for (const auto &Mesh : Meshes) {
    if (!Mesh->GetVertices().size())
    {
      continue;
    }
    AProceduralMeshActor* TempActor = GetWorld()->SpawnActor<AProceduralMeshActor>();
    UProceduralMeshComponent *TempPMC = TempActor->MeshComponent;
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
