// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "OpenDriveToMap.h"
#include "Components/Button.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

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
  if ( !IsValid(FileDownloader) ) 
  {
    FileDownloader = NewObject<UCustomFileDownloader>();
  }
  FileDownloader->ResultFileName = MapName;
  FileDownloader->Url = Url;
  FileDownloader->DownloadDelegate.BindUObject( this, &UOpenDriveToMap::ConvertOSMInOpenDrive );
  FileDownloader->StartDownload();

  RoadType.Empty();
  RoadMesh.Empty();
  MeshesToSpawn.Empty();
  ActorMeshList.Empty();
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
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Valid Map loaded"));
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
  opg_parameters.vertex_distance = 0.5f;
  opg_parameters.vertex_width_resolution = 8.0f;
  opg_parameters.simplification_percentage = 15.0f;

  double start = FPlatformTime::Seconds();
  const auto Meshes = CarlaMap->GenerateOrderedChunkedMesh(opg_parameters);
  double end = FPlatformTime::Seconds();
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" GenerateOrderedChunkedMesh code executed in %f seconds. Simplification percentage is %f"), end - start, opg_parameters.simplification_percentage);

  start = FPlatformTime::Seconds();
  for (const auto &PairMap : Meshes) 
  {
    for( const auto &Mesh : PairMap.second )
    {
      if (!Mesh->GetVertices().size())
      {
        continue;
      }
      if (!Mesh->IsValid()) {
        continue;
      }

      AProceduralMeshActor* TempActor = GetWorld()->SpawnActor<AProceduralMeshActor>();
      UProceduralMeshComponent *TempPMC = TempActor->MeshComponent;
      TempPMC->bUseAsyncCooking = true;
      TempPMC->bUseComplexAsSimpleCollision = true;
      TempPMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

      FVector MeshCentroid = FVector(0,0,0);
      for( auto Vertex : Mesh->GetVertices() )
      {
        MeshCentroid += Vertex.ToFVector();
      }

      MeshCentroid /= Mesh->GetVertices().size();

      for( auto& Vertex : Mesh->GetVertices() )
      {
       Vertex.x -= MeshCentroid.X;
       Vertex.y -= MeshCentroid.Y;
       Vertex.z -= MeshCentroid.Z;
      }

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
      TempActor->SetActorLocation(MeshCentroid * 100);
      ActorMeshList.Add(TempActor);

      RoadType.Add(LaneTypeToFString(PairMap.first));
      RoadMesh.Add(TempPMC);          
    }
  }
    
  end = FPlatformTime::Seconds();
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Mesh spawnning and translation code executed in %f seconds."), end - start);
 
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
    StaticMesh->Build(false);
    StaticMesh->PostEditChange();

    // Notify asset registry of new asset
    FAssetRegistryModule::AssetCreated(StaticMesh);
    UPackage::SavePackage(Package, StaticMesh, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *MeshName, GError, nullptr, true, true, SAVE_NoError);
    return StaticMesh;
  }
  return nullptr;
}

TArray<UStaticMesh*> UOpenDriveToMap::CreateStaticMeshAssets()
{
  double start = FPlatformTime::Seconds();
  double end = FPlatformTime::Seconds();

  IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
  TArray<UStaticMesh*> StaticMeshes;

  double BuildMeshDescriptionTime = 0.0f;
  double PackgaesCreatingTime = 0.0f;
  double MeshInitTime = 0.0f;
  double MatAndCollInitTime = 0.0f;
  double MeshBuildTime = 0.0f;
  double PackSaveTime = 0.0f;


  for (int i = 0; i < RoadMesh.Num(); ++i)
  {
    FString MeshName = RoadType[i] + FString::FromInt(i);
    FString PackageName = "/Game/CustomMaps/" + MapName + "/Static/" + RoadType[i] + "/" + MeshName;

    if (!PlatformFile.DirectoryExists(*PackageName))
    {
      PlatformFile.CreateDirectory(*PackageName);
    }

    UProceduralMeshComponent* ProcMeshComp = RoadMesh[i];
    start = FPlatformTime::Seconds();
    FMeshDescription MeshDescription = BuildMeshDescription(ProcMeshComp);
    end = FPlatformTime::Seconds();
    BuildMeshDescriptionTime += end - start;
    // If we got some valid data.
    if (MeshDescription.Polygons().Num() > 0)
    {
      start = FPlatformTime::Seconds();
      // Then find/create it.
      UPackage* Package = CreatePackage(*PackageName);
      check(Package);
      end = FPlatformTime::Seconds();
      PackgaesCreatingTime += end - start;

      start = FPlatformTime::Seconds();

      // Create StaticMesh object
      UStaticMesh* CurrentStaticMesh = NewObject<UStaticMesh>(Package, *MeshName, RF_Public | RF_Standalone);
      CurrentStaticMesh->InitResources();

      CurrentStaticMesh->LightingGuid = FGuid::NewGuid();

      // Add source to new StaticMesh
      FStaticMeshSourceModel& SrcModel = CurrentStaticMesh->AddSourceModel();
      SrcModel.BuildSettings.bRecomputeNormals = false;
      SrcModel.BuildSettings.bRecomputeTangents = false;
      SrcModel.BuildSettings.bRemoveDegenerates = false;
      SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
      SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
      SrcModel.BuildSettings.bGenerateLightmapUVs = true;
      SrcModel.BuildSettings.SrcLightmapIndex = 0;
      SrcModel.BuildSettings.DstLightmapIndex = 1;
      CurrentStaticMesh->CreateMeshDescription(0, MoveTemp(MeshDescription));
      CurrentStaticMesh->CommitMeshDescription(0);
      end = FPlatformTime::Seconds();
      MeshInitTime += end - start;
      start = FPlatformTime::Seconds();

      //// SIMPLE COLLISION
      if (!ProcMeshComp->bUseComplexAsSimpleCollision)
      {
        CurrentStaticMesh->CreateBodySetup();
        UBodySetup* NewBodySetup = CurrentStaticMesh->BodySetup;
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
        FProcMeshSection* ProcSection =
          ProcMeshComp->GetProcMeshSection(SectionIdx);
        UMaterialInterface* Material = ProcMeshComp->GetMaterial(SectionIdx);
        UniqueMaterials.Add(Material);
      }
      // Copy materials to new mesh
      for (auto* Material : UniqueMaterials)
      {
        CurrentStaticMesh->StaticMaterials.Add(FStaticMaterial(Material));
      }

      end = FPlatformTime::Seconds();
      MatAndCollInitTime += end - start;
      start = FPlatformTime::Seconds();
      //Set the Imported version before calling the build
      CurrentStaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;
      CurrentStaticMesh->Build(false);
      CurrentStaticMesh->PostEditChange();

      end = FPlatformTime::Seconds();
      MeshBuildTime += end - start;
      start = FPlatformTime::Seconds();

      FString MeshName = *(RoadType[i] + FString::FromInt(i));
      // Notify asset registry of new asset
      FAssetRegistryModule::AssetCreated(CurrentStaticMesh);
      UPackage::SavePackage(Package, CurrentStaticMesh, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *MeshName, GError, nullptr, true, true, SAVE_NoError);
      end = FPlatformTime::Seconds();
      PackSaveTime += end - start;


      StaticMeshes.Add( CurrentStaticMesh );
    }
  }
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" UOpenDriveToMap::CreateStaticMeshAssets total time in BuildMeshDescriptionTime %f. Time per mesh %f"), BuildMeshDescriptionTime, BuildMeshDescriptionTime/ RoadMesh.Num());
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" UOpenDriveToMap::CreateStaticMeshAssets total time in PackgaesCreatingTime %f. Time per mesh %f"), PackgaesCreatingTime, PackgaesCreatingTime / RoadMesh.Num());
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" UOpenDriveToMap::CreateStaticMeshAssets total time in MeshInitTime %f. Time per mesh %f"), MeshInitTime, MeshInitTime / RoadMesh.Num());
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" UOpenDriveToMap::CreateStaticMeshAssets total time in MatAndCollInitTime %f. Time per mesh %f"), MatAndCollInitTime, MatAndCollInitTime / RoadMesh.Num());
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" UOpenDriveToMap::CreateStaticMeshAssets total time in MeshBuildTime %f. Time per mesh %f"), MeshBuildTime, MeshBuildTime / RoadMesh.Num());
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" UOpenDriveToMap::CreateStaticMeshAssets total time in PackSaveTime %f. Time per mesh %f"), PackSaveTime, PackSaveTime / RoadMesh.Num());
  return StaticMeshes;
}

void UOpenDriveToMap::SaveMap()
{
  double start = FPlatformTime::Seconds();

  MeshesToSpawn = CreateStaticMeshAssets();

  double end = FPlatformTime::Seconds();
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" Meshes created static mesh code executed in %f seconds."), end - start);

  for (auto CurrentActor : ActorMeshList)
  {
    CurrentActor->Destroy();
  }

  start = FPlatformTime::Seconds();

  for (auto CurrentMesh : MeshesToSpawn)
  {
    AStaticMeshActor* TempActor = GetWorld()->SpawnActor<AStaticMeshActor>();
    // Build mesh from source
    TempActor->GetStaticMeshComponent()->SetStaticMesh(CurrentMesh);
    TempActor->SetActorLabel(FString("SM_") + CurrentMesh->GetName());
  }

  end = FPlatformTime::Seconds();
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" Spawning Static Meshes code executed in %f seconds."), end - start);

}