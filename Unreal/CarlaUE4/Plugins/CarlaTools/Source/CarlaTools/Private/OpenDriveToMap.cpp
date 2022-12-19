// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "OpenDriveToMap.h"
#include "Components/Button.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FileHelper.h"

#include "Carla/Game/CarlaStatics.h"
#include "Traffic/TrafficLightManager.h"
#include "Online/CustomFileDownloader.h"
#include "Util/ProceduralCustomMesh.h"

#include "OpenDrive/OpenDriveGenerator.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>
#include <carla/rpc/String.h>
#include <OSM2ODR.h>
#include <compiler/enable-ue4-macros.h>

#include "Engine/Classes/Interfaces/Interface_CollisionDataProvider.h"
#include "PhysicsCore/Public/BodySetupEnums.h"
#include "RawMesh.h"
#include "AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "MeshDescription.h"
#include "ProceduralMeshConversion.h"


FString LaneTypeToFString(carla::road::Lane::LaneType LaneType)
{
  switch (LaneType)
  {
  case carla::road::Lane::LaneType::Driving:
    return FString("Driving");
    break;
  case carla::road::Lane::LaneType::Stop:
    return FString("Stop");
    break;
  case carla::road::Lane::LaneType::Shoulder:
    return FString("Shoulder");
    break;  
  case carla::road::Lane::LaneType::Biking:
    return FString("Biking");
    break;  
  case carla::road::Lane::LaneType::Sidewalk:
    return FString("Sidewalk");
    break;  
  case carla::road::Lane::LaneType::Border:
    return FString("Border");
    break;
  case carla::road::Lane::LaneType::Restricted:
    return FString("Restricted");
    break;
  case carla::road::Lane::LaneType::Parking:
    return FString("Parking");
    break;
  case carla::road::Lane::LaneType::Bidirectional:
    return FString("Bidirectional");
    break;
  case carla::road::Lane::LaneType::Median:
    return FString("Median");
    break;  
  case carla::road::Lane::LaneType::Special1:
    return FString("Special1");
    break;
  case carla::road::Lane::LaneType::Special2:
    return FString("Special2");
    break;
  case carla::road::Lane::LaneType::Special3:
    return FString("Special3");
    break;
  case carla::road::Lane::LaneType::RoadWorks:
    return FString("RoadWorks");
    break;
  case carla::road::Lane::LaneType::Tram:
    return FString("Tram");
    break;
  case carla::road::Lane::LaneType::Rail:
    return FString("Rail");
    break;
  case carla::road::Lane::LaneType::Entry:
    return FString("Entry");
    break;
  case carla::road::Lane::LaneType::Exit:
    return FString("Exit");
    break;
  case carla::road::Lane::LaneType::OffRamp:
    return FString("OffRamp");
    break;
  case carla::road::Lane::LaneType::OnRamp:
    return FString("OnRamp");
    break;
  case carla::road::Lane::LaneType::Any:
    return FString("Any");
    break;
  }

  return FString("Empty");
}

void UOpenDriveToMap::ConvertOSMInOpenDrive()
{
  FilePath = FPaths::ProjectContentDir() + "CustomMaps/" + MapName + "/" + MapName + ".osm";
  FileDownloader->ConvertOSMInOpenDrive( FilePath );
  FilePath.RemoveFromEnd(".osm", ESearchCase::Type::IgnoreCase);
  FilePath += ".xodr";

  LoadMap();
}

void UOpenDriveToMap::NativeConstruct()
{
  if( !IsValid(FileDownloader) ){
    FileDownloader = NewObject<UCustomFileDownloader>();
  }
}

void UOpenDriveToMap::NativeDestruct()
{
  Super::NativeDestruct();
  if( IsValid(FileDownloader) ){
    // UObjects are not being destroyed, they are collected by GarbageCollector
    // Should we force garbage collection here?
  }
}

void UOpenDriveToMap::CreateMap()
{
  if( MapName.IsEmpty() )
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Map Name Is Empty") );
    return;
  }
  FileDownloader->ResultFileName = MapName;
  FileDownloader->Url = Url;
  FileDownloader->DownloadDelegate.BindUObject( this, &UOpenDriveToMap::ConvertOSMInOpenDrive );
  FileDownloader->StartDownload();
}

void UOpenDriveToMap::OpenFileDialog()
{
  TArray<FString> OutFileNames;
  void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
  IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
  if (DesktopPlatform)
  {
    DesktopPlatform->OpenFileDialog(ParentWindowPtr, "Select xodr file", FPaths::ProjectDir(), FString(""), ".xodr", 1, OutFileNames);
  }
  for(FString& CurrentString : OutFileNames)
  {
    FilePath = CurrentString;
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("FileObtained %s"), *CurrentString );
  }
}

void UOpenDriveToMap::LoadMap()
{
  FString FileContent;
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("UOpenDriveToMap::LoadMap(): File to load %s"), *FilePath );
  FFileHelper::LoadFileToString(FileContent, *FilePath);
  std::string opendrive_xml = carla::rpc::FromLongFString(FileContent);
  boost::optional<carla::road::Map> CarlaMap = carla::opendrive::OpenDriveParser::Load(opendrive_xml);

  if (!CarlaMap.has_value()) 
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Invalid Map"));
  }else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Valid Map loaded"));
  }
  MapName = FPaths::GetCleanFilename(FilePath);
  MapName.RemoveFromEnd(".xodr", ESearchCase::Type::IgnoreCase);
  UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("MapName %s"), *MapName);

  GenerateAll(CarlaMap);
}

void UOpenDriveToMap::GenerateAll(const boost::optional<carla::road::Map>& CarlaMap )
{
  if (!CarlaMap.has_value()) 
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Invalid Map"));
  }else
  {
    GenerateRoadMesh(CarlaMap);
    GenerateSpawnPoints(CarlaMap);
  }
}

void UOpenDriveToMap::GenerateRoadMesh( const boost::optional<carla::road::Map>& CarlaMap )
{
  const auto Meshes = CarlaMap->GenerateOrderedChunkedMesh(opg_parameters);
  TArray<AActor*> ActorMeshList;
  TArray<UStaticMesh*> MeshesToSpawn;
  int32 Index = 0;
  for (const auto &PairMap : Meshes) 
  {
    for( const auto &Mesh : PairMap.second )
    {
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
      UStaticMesh* GeneratedMesh = CreateStaticMeshAsset(TempPMC, Index, LaneTypeToFString(PairMap.first));
      if( GeneratedMesh != nullptr)
      {
        MeshesToSpawn.Add(GeneratedMesh);
      }
      Index++;
    }
  }

  for( auto CurrentActor : ActorMeshList )
  {
    CurrentActor->Destroy();
  }
  for(auto CurrentMesh : MeshesToSpawn )
  {
    AStaticMeshActor* TempActor = GetWorld()->SpawnActor<AStaticMeshActor>();
    TempActor->GetStaticMeshComponent()->SetStaticMesh(CurrentMesh);
    TempActor->SetActorLabel(FString("SM_") + CurrentMesh->GetName());
  }
/*
  if(!Parameters.enable_mesh_visibility)
  {
    for(AActor * actor : ActorMeshList)
    {
      actor->SetActorHiddenInGame(true);
    }
  }
*/
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

void UOpenDriveToMap::GenerateSpawnPoints( const boost::optional<carla::road::Map>& CarlaMap )
{
  float SpawnersHeight = 300.f;
  const auto Waypoints = CarlaMap->GenerateWaypointsOnRoadEntries();
  for (const auto &Wp : Waypoints)
  {
    const FTransform Trans = CarlaMap->ComputeTransform(Wp);
    AVehicleSpawnPoint *Spawner = GetWorld()->SpawnActor<AVehicleSpawnPoint>();
    Spawner->SetActorRotation(Trans.GetRotation());
    Spawner->SetActorLocation(Trans.GetTranslation() + FVector(0.f, 0.f, SpawnersHeight));
    //VehicleSpawners.Add(Spawner);
  }
}

UStaticMesh* UOpenDriveToMap::CreateStaticMeshAsset( UProceduralMeshComponent* ProcMeshComp, int32 MeshIndex, FString FolderName )
{
  FMeshDescription MeshDescription = BuildMeshDescription(ProcMeshComp);

  IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

  // If we got some valid data.
  if (MeshDescription.Polygons().Num() > 0)
  {
    FString MeshName = *(FolderName + FString::FromInt(MeshIndex) );
    FString PackageName = "/Game/CustomMaps/" + MapName + "/Static/" + FolderName + "/" + MeshName;
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("PackageName %s"), *PackageName );

    if( !PlatformFile.DirectoryExists(*PackageName) )
    {
      PlatformFile.CreateDirectory(*PackageName);
    }

    // Then find/create it.
    UPackage* Package = CreatePackage(*PackageName);
    check(Package);
    // Create StaticMesh object
    UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, *MeshName, RF_Public | RF_Standalone);
    StaticMesh->InitResources();

    StaticMesh->LightingGuid = FGuid::NewGuid();

    // Add source to new StaticMesh
    FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
    SrcModel.BuildSettings.bRecomputeNormals = false;
    SrcModel.BuildSettings.bRecomputeTangents = false;
    SrcModel.BuildSettings.bRemoveDegenerates = false;
    SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
    SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
    SrcModel.BuildSettings.bGenerateLightmapUVs = true;
    SrcModel.BuildSettings.SrcLightmapIndex = 0;
    SrcModel.BuildSettings.DstLightmapIndex = 1;
    StaticMesh->CreateMeshDescription(0, MoveTemp(MeshDescription));
    StaticMesh->CommitMeshDescription(0);

    //// SIMPLE COLLISION
    if (!ProcMeshComp->bUseComplexAsSimpleCollision )
    {
      StaticMesh->CreateBodySetup();
      UBodySetup* NewBodySetup = StaticMesh->BodySetup;
      NewBodySetup->BodySetupGuid = FGuid::NewGuid();
      NewBodySetup->AggGeom.ConvexElems = ProcMeshComp->ProcMeshBodySetup->AggGeom.ConvexElems;
      NewBodySetup->bGenerateMirroredCollision = false;
      NewBodySetup->bDoubleSidedGeometry = true;
      NewBodySetup->CollisionTraceFlag = CTF_UseDefault;
      NewBodySetup->CreatePhysicsMeshes();
    }

    //// MATERIALS
    TSet<UMaterialInterface*> UniqueMaterials;
    const int32 NumSections = ProcMeshComp->GetNumSections();
    for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
    {
      FProcMeshSection *ProcSection =
        ProcMeshComp->GetProcMeshSection(SectionIdx);
      UMaterialInterface *Material = ProcMeshComp->GetMaterial(SectionIdx);
      UniqueMaterials.Add(Material);
    }
    // Copy materials to new mesh
    for (auto* Material : UniqueMaterials)
    {
      StaticMesh->StaticMaterials.Add(FStaticMaterial(Material));
    }

    //Set the Imported version before calling the build
    StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;

    // Build mesh from source
    StaticMesh->Build(false);
    StaticMesh->PostEditChange();

    // Notify asset registry of new asset
    FAssetRegistryModule::AssetCreated(StaticMesh);
    UPackage::SavePackage(Package, StaticMesh, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *MeshName, GError, nullptr, true, true, SAVE_NoError);
    return StaticMesh;
  }
  return nullptr;
}