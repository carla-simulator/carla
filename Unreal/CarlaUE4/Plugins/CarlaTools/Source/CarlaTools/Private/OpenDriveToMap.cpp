// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "OpenDriveToMap.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "Engine/LevelBounds.h"
#include "Engine/SceneCapture2D.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "KismetProceduralMeshLibrary.h"
#include "StaticMeshAttributes.h"

#include "Traffic/TrafficLightManager.h"
#include "Online/CustomFileDownloader.h"
#include "Util/ProceduralCustomMesh.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/BlueprintLibary/MapGenFunctionLibrary.h"
#include "OpenDrive/OpenDriveGenerator.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>
#include <carla/geom/Simplification.h>
#include <carla/road/Deformation.h>
#include <carla/rpc/String.h>
#include <OSM2ODR.h>
#include <compiler/enable-ue4-macros.h>

#include "Engine/Classes/Interfaces/Interface_CollisionDataProvider.h"
#include "Engine/TriggerBox.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "PhysicsCore/Public/BodySetupEnums.h"
#include "PhysicsEngine/BodySetup.h"
#include "RawMesh.h"
#include "AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "MeshDescription.h"
#include "EditorLevelLibrary.h"
#include "ProceduralMeshConversion.h"
#include "EditorLevelLibrary.h"

#include "ContentBrowserModule.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Math/Vector.h"
#include "GameFramework/Actor.h"

#include "DrawDebugHelpers.h"



UOpenDriveToMap::~UOpenDriveToMap()
{

}

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
  FilePath = FPaths::ProjectContentDir() + "CustomMaps/" + MapName + "/OpenDrive/" + MapName + ".osm";
  FileDownloader->ConvertOSMInOpenDrive( FilePath , OriginGeoCoordinates.X, OriginGeoCoordinates.Y);
  FilePath.RemoveFromEnd(".osm", ESearchCase::Type::IgnoreCase);
  FilePath += ".xodr";

  LoadMap();
}

void UOpenDriveToMap::CreateMap()
{
  AddToRoot();
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

  UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("Map Name Is %s"), *MapName );

  FileDownloader->XodrToMap = this;
  FileDownloader->StartDownload();
}

void UOpenDriveToMap::CreateTerrain( const int MeshGridSize, const float MeshGridSectionSize, const class UTexture2D* HeightmapTexture)
{
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(UEditorLevelLibrary::GetEditorWorld(), AStaticMeshActor::StaticClass(), FoundActors);
  FVector BoxOrigin;
  FVector BoxExtent;
  UGameplayStatics::GetActorArrayBounds(FoundActors, false, BoxOrigin, BoxExtent);
  FVector MinBox = BoxOrigin - BoxExtent;

  int NumI = ( BoxExtent.X * 2.0f ) / MeshGridSize;
  int NumJ = ( BoxExtent.Y * 2.0f ) / MeshGridSize;
  ASceneCapture2D* SceneCapture = Cast<ASceneCapture2D>(UEditorLevelLibrary::GetEditorWorld()->SpawnActor(ASceneCapture2D::StaticClass()));
  SceneCapture->SetActorRotation(FRotator(-90,90,0));
  SceneCapture->GetCaptureComponent2D()->ProjectionType = ECameraProjectionMode::Type::Orthographic;
  SceneCapture->GetCaptureComponent2D()->OrthoWidth = MeshGridSize;
  SceneCapture->GetCaptureComponent2D()->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
  SceneCapture->GetCaptureComponent2D()->CompositeMode = ESceneCaptureCompositeMode::SCCM_Overwrite;
  SceneCapture->GetCaptureComponent2D()->bCaptureEveryFrame = false;
  SceneCapture->GetCaptureComponent2D()->bCaptureOnMovement = false;
  //UTextureRenderTarget2D* RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(UEditorLevelLibrary::GetEditorWorld(), 256, 256,
  //                                                          ETextureRenderTargetFormat::RTF_RGBA8, FLinearColor(0,0,0), false );
  //SceneCapture->GetCaptureComponent2D()->TextureTarget = RenderTarget;

  /* Blueprint darfted code should be here */
  for( int i = 0; i < NumI; i++ )
  {
    for( int j = 0; j < NumJ; j++ )
    {
      // Offset that each procedural mesh is displaced to accomodate all the tiles
      FVector2D Offset( MinBox.X + i * MeshGridSize, MinBox.Y + j * MeshGridSize);
      SceneCapture->SetActorLocation(FVector(Offset.X + MeshGridSize/2, Offset.Y + MeshGridSize/2, 500));
      //SceneCapture->GetCaptureComponent2D()->CaptureScene();
      CreateTerrainMesh(i * NumJ + j, Offset, MeshGridSize, MeshGridSectionSize, HeightmapTexture, nullptr );
    }
  }
}

void UOpenDriveToMap::CreateTerrainMesh(const int MeshIndex, const FVector2D Offset, const int GridSize, const float GridSectionSize, const UTexture2D* HeightmapTexture, UTextureRenderTarget2D* RoadMask)
{
  // const float GridSectionSize = 100.0f; // In cm
  const float HeightScale = 3.0f;

  UWorld* World = UEditorLevelLibrary::GetEditorWorld();

  // Creation of the procedural mesh
  AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>();
  MeshActor->SetActorLocation(FVector(Offset.X, Offset.Y, 0));
  UStaticMeshComponent* Mesh = MeshActor->GetStaticMeshComponent();

  TArray<FVector> Vertices;
  TArray<int32> Triangles;

  TArray<FVector> Normals;
  TArray<FLinearColor> Colors;
  TArray<FProcMeshTangent> Tangents;
  TArray<FVector2D> UVs;

  //// Procedural mesh default parameters
  //// Get Heightmap data from texture, Loading first mip and getting a pointer to the color of the first pixel
  //FByteBulkData* RawHeightmap = &HeightmapTexture->PlatformData->Mips[0].BulkData;
  //FColor* FormatedHeightmap = StaticCast<FColor*>(RawHeightmap->Lock(LOCK_READ_ONLY));
//
  //// Road mask
	//int32 Width = RoadMask->SizeX, Height = RoadMask->SizeY;
	//TArray<FFloat16Color> ImageData;
	//FTextureRenderTargetResource* RenderTargetResource;
	//ImageData.AddUninitialized(Width * Height);
	//RenderTargetResource = RoadMask->GameThread_GetRenderTargetResource();
	//RenderTargetResource->ReadFloat16Pixels(ImageData);
//
  //// check(FormatedHeightmap != nullptr);
  //// check(FormatedRoadMask != nullptr);
//
  int VerticesInLine = (GridSize / GridSectionSize) + 1.0f;
  for( int i = 0; i < VerticesInLine; i++ )
  {
    float X = (i * GridSectionSize);
    const int RoadMapX = i * 255 / VerticesInLine;
    for( int j = 0; j < VerticesInLine; j++ )
    {
      float Y = (j * GridSectionSize);
      const int RoadMapY = j * 255 / VerticesInLine;
      const int CellIndex = RoadMapY + 255 * RoadMapX;
      float HeightValue = GetHeightForLandscape( FVector( (Offset.X + X),
                                                          (Offset.Y + Y),
                                                          0));
      //if( ImageData[CellIndex].R > 0.5 ) // Small Threshold  /* Uncomment to apply road mask */
      //{
      //  // Getting the value for the height in this vertex.
      //  // If the road mask is higher that 0, there is road so height value = 0
      //  HeightValue -= 500.0f;
      //}
      //UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" i %d, j %d, X %f, RoadMapX %d, Y %f, RoadMapY %d, CellIndex %d"), i, j, X, RoadMapX, Y, RoadMapY, CellIndex );

      Vertices.Add(FVector( X, Y, HeightValue));
      UVs.Add(FVector2D(i, j));
    }
  }

  //RawHeightmap->Unlock();
  //RawRoadMask->Unlock();  /* Uncomment to apply road mask */

  //// Triangles formation. 2 triangles per section.

  for(int i = 0; i < VerticesInLine - 1; i++)
  {
    for(int j = 0; j < VerticesInLine - 1; j++)
    {
      Triangles.Add(   j       + (   i       * VerticesInLine ) );
      Triangles.Add( ( j + 1 ) + (   i       * VerticesInLine ) );
      Triangles.Add(   j       + ( ( i + 1 ) * VerticesInLine ) );

      Triangles.Add( ( j + 1 ) + (   i       * VerticesInLine ) );
      Triangles.Add( ( j + 1 ) + ( ( i + 1 ) * VerticesInLine ) );
      Triangles.Add(   j       + ( ( i + 1 ) * VerticesInLine ) );
    }
  }

  UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
    Vertices,
    Triangles,
    UVs,
    Normals,
    Tangents
  );

  FProceduralCustomMesh MeshData;
  MeshData.Vertices = Vertices;
  MeshData.Triangles = Triangles;
  MeshData.Normals = Normals;
  MeshData.UV0 = UVs;
  UStaticMesh* MeshToSet = UMapGenFunctionLibrary::CreateMesh(MeshData,  Tangents, DefaultLandscapeMaterial, MapName, "Terrain", FName(TEXT("SM_LandscapeMesh" + FString::FromInt(MeshIndex) )));
  Mesh->SetStaticMesh(MeshToSet);
  MeshActor->SetActorLabel("SM_LandscapeActor" + FString::FromInt(MeshIndex) );
  Mesh->CastShadow = false;
  Landscapes.Add(MeshActor);
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
  CarlaMap = carla::opendrive::OpenDriveParser::Load(opendrive_xml);

  if (!CarlaMap.has_value())
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Invalid Map"));
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Valid Map loaded"));
  }
  MapName = FPaths::GetCleanFilename(FilePath);
  MapName.RemoveFromEnd(".xodr", ESearchCase::Type::IgnoreCase);
  UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("MapName %s"), *MapName);

  GenerateAll(CarlaMap);
  GenerationFinished();
  RemoveFromRoot();
}

TArray<AActor*> UOpenDriveToMap::GenerateMiscActors(float Offset)
{
  std::vector<std::pair<carla::geom::Transform, std::string>>
    Locations = CarlaMap->GetTreesTransform(DistanceBetweenTrees, DistanceFromRoadEdge, Offset);
  TArray<AActor*> Returning;
  int i = 0;
  for (auto& cl : Locations)
  {
    const FVector scale{ 1.0f, 1.0f, 1.0f };
    cl.first.location.z = GetHeight(cl.first.location.x, cl.first.location.y) + 0.3f;
    FTransform NewTransform ( FRotator(cl.first.rotation), FVector(cl.first.location), scale );

    NewTransform = GetSnappedPosition(NewTransform);

    AActor* Spawner = UEditorLevelLibrary::GetEditorWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
      NewTransform.GetLocation(), NewTransform.Rotator());
    Spawner->Tags.Add(FName("MiscSpawnPosition"));
    Spawner->Tags.Add(FName(cl.second.c_str()));
    Spawner->SetActorLabel("MiscSpawnPosition" + FString::FromInt(i));
    ++i;
    Returning.Add(Spawner);
  }
  return Returning;
}
void UOpenDriveToMap::GenerateAll(const boost::optional<carla::road::Map>& ParamCarlaMap )
{
  if (!ParamCarlaMap.has_value())
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Invalid Map"));
  }else
  {
    if(DefaultHeightmap && !Heightmap){
      Heightmap = DefaultHeightmap;
    }

    GenerateRoadMesh(ParamCarlaMap);
    GenerateLaneMarks(ParamCarlaMap);
    GenerateSpawnPoints(ParamCarlaMap);
    CreateTerrain(12800, 256, nullptr);
    GenerateTreePositions(ParamCarlaMap);
  }
}

void UOpenDriveToMap::GenerateRoadMesh( const boost::optional<carla::road::Map>& ParamCarlaMap )
{
  opg_parameters.vertex_distance = 0.5f;
  opg_parameters.vertex_width_resolution = 8.0f;
  opg_parameters.simplification_percentage = 50.0f;

  double start = FPlatformTime::Seconds();
  const auto Meshes = ParamCarlaMap->GenerateOrderedChunkedMesh(opg_parameters);
  double end = FPlatformTime::Seconds();
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT(" GenerateOrderedChunkedMesh code executed in %f seconds. Simplification percentage is %f"), end - start, opg_parameters.simplification_percentage);

  start = FPlatformTime::Seconds();
  int index = 0;
  for (const auto &PairMap : Meshes)
  {
    for( auto& Mesh : PairMap.second )
    {
      if (!Mesh->GetVertices().size())
      {
        continue;
      }
      if (!Mesh->IsValid()) {
        continue;
      }

      if(PairMap.first == carla::road::Lane::LaneType::Driving)
      {
        for( auto& Vertex : Mesh->GetVertices() )
        {
          FVector VertexFVector = Vertex.ToFVector();
          Vertex.z += GetHeight(Vertex.x, Vertex.y, DistanceToLaneBorder(ParamCarlaMap,VertexFVector) > 65.0f );
        }
        carla::geom::Simplification Simplify(0.15);
        Simplify.Simplificate(Mesh);
      }else{
        for( auto& Vertex : Mesh->GetVertices() )
        {
          Vertex.z += GetHeight(Vertex.x, Vertex.y, false) + 0.15f;
        }
      }

      AStaticMeshActor* TempActor = UEditorLevelLibrary::GetEditorWorld()->SpawnActor<AStaticMeshActor>();
      UStaticMeshComponent* StaticMeshComponent = TempActor->GetStaticMeshComponent();
      TempActor->SetActorLabel(FString("SM_Lane_") + FString::FromInt(index));

      StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

      if(DefaultRoadMaterial && PairMap.first == carla::road::Lane::LaneType::Driving)
      {
        StaticMeshComponent->SetMaterial(0, DefaultRoadMaterial);
        StaticMeshComponent->CastShadow = false;
        TempActor->SetActorLabel(FString("SM_DrivingLane_") + FString::FromInt(index));
      }
      if(DefaultSidewalksMaterial && PairMap.first == carla::road::Lane::LaneType::Sidewalk)
      {
        StaticMeshComponent->SetMaterial(0, DefaultSidewalksMaterial);
        TempActor->SetActorLabel(FString("SM_Sidewalk_") + FString::FromInt(index));
      }
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
      TArray<FVector> Normals;
      TArray<FProcMeshTangent> Tangents;

      UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
        MeshData.Vertices,
        MeshData.Triangles,
        MeshData.UV0,
        Normals,
        Tangents
      );

      if(PairMap.first == carla::road::Lane::LaneType::Sidewalk)
      {
        UStaticMesh* MeshToSet = UMapGenFunctionLibrary::CreateMesh(MeshData,  Tangents, DefaultSidewalksMaterial, MapName, "DrivingLane", FName(TEXT("SM_SidewalkMesh" + FString::FromInt(index) )));
        StaticMeshComponent->SetStaticMesh(MeshToSet);
      }

      if(PairMap.first == carla::road::Lane::LaneType::Driving)
      {
        UStaticMesh* MeshToSet = UMapGenFunctionLibrary::CreateMesh(MeshData,  Tangents, DefaultRoadMaterial, MapName, "DrivingLane", FName(TEXT("SM_DrivingLaneMesh" + FString::FromInt(index) )));
        StaticMeshComponent->SetStaticMesh(MeshToSet);
      }
      TempActor->SetActorLocation(MeshCentroid * 100);
      TempActor->Tags.Add(FName("RoadLane"));
      // ActorMeshList.Add(TempActor);
      StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
      TempActor->SetActorEnableCollision(true);
      index++;
    }
  }

  end = FPlatformTime::Seconds();
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Mesh spawnning and translation code executed in %f seconds."), end - start);
}

void UOpenDriveToMap::GenerateLaneMarks(const boost::optional<carla::road::Map>& ParamCarlaMap)
{
  opg_parameters.vertex_distance = 0.5f;
  opg_parameters.vertex_width_resolution = 8.0f;
  opg_parameters.simplification_percentage = 15.0f;
  std::vector<std::string> lanemarkinfo;
  auto MarkingMeshes = ParamCarlaMap->GenerateLineMarkings(opg_parameters, lanemarkinfo);
  TArray<AActor*> LaneMarkerActorList;
  int index = 0;
  for (const auto& Mesh : MarkingMeshes)
  {

    if ( !Mesh->GetVertices().size() )
    {
      index++;
      continue;
    }
    if ( !Mesh->IsValid() ) {
      index++;
      continue;
    }

    FVector MeshCentroid = FVector(0, 0, 0);
    for (auto& Vertex : Mesh->GetVertices())
    {
      FVector VertexFVector = Vertex.ToFVector();
      Vertex.z += GetHeight(Vertex.x, Vertex.y, DistanceToLaneBorder(ParamCarlaMap,VertexFVector) > 65.0f ) + 0.0001f;
      MeshCentroid += Vertex.ToFVector();
    }

    MeshCentroid /= Mesh->GetVertices().size();

    for (auto& Vertex : Mesh->GetVertices())
    {
      Vertex.x -= MeshCentroid.X;
      Vertex.y -= MeshCentroid.Y;
      Vertex.z -= MeshCentroid.Z;
    }

    // TODO: Improve this code
    float MinDistance = 99999999.9f;
    for(auto SpawnedActor : LaneMarkerActorList)
    {
      float VectorDistance = FVector::Distance(MeshCentroid*100, SpawnedActor->GetActorLocation());
      if(VectorDistance < MinDistance)
      {
        MinDistance = VectorDistance;
      }
    }

    if(MinDistance < 250)
    {
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("Skkipped is %f."), MinDistance);
      index++;
      continue;
    }

    AStaticMeshActor* TempActor = UEditorLevelLibrary::GetEditorWorld()->SpawnActor<AStaticMeshActor>();
    UStaticMeshComponent* StaticMeshComponent = TempActor->GetStaticMeshComponent();
    TempActor->SetActorLabel(FString("SM_LaneMark_") + FString::FromInt(index));
    StaticMeshComponent->CastShadow = false;
    if (lanemarkinfo[index].find("yellow") != std::string::npos) {
      if(DefaultLaneMarksYellowMaterial)
        StaticMeshComponent->SetMaterial(0, DefaultLaneMarksYellowMaterial);
    }else{
      if(DefaultLaneMarksWhiteMaterial)
        StaticMeshComponent->SetMaterial(0, DefaultLaneMarksWhiteMaterial);

    }

    const FProceduralCustomMesh MeshData = *Mesh;
    TArray<FVector> Normals;
    TArray<FProcMeshTangent> Tangents;
    UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
      MeshData.Vertices,
      MeshData.Triangles,
      MeshData.UV0,
      Normals,
      Tangents
    );

    UStaticMesh* MeshToSet = UMapGenFunctionLibrary::CreateMesh(MeshData,  Tangents, DefaultLandscapeMaterial, MapName, "LaneMark", FName(TEXT("SM_LaneMarkMesh" + FString::FromInt(index) )));
    StaticMeshComponent->SetStaticMesh(MeshToSet);
    TempActor->SetActorLocation(MeshCentroid * 100);
    TempActor->Tags.Add(*FString(lanemarkinfo[index].c_str()));
    TempActor->Tags.Add(FName("RoadLane"));
    LaneMarkerActorList.Add(TempActor);
    index++;
    TempActor->SetActorEnableCollision(false);
    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  }
}

void UOpenDriveToMap::GenerateSpawnPoints( const boost::optional<carla::road::Map>& ParamCarlaMap )
{
  float SpawnersHeight = 300.f;
  const auto Waypoints = ParamCarlaMap->GenerateWaypointsOnRoadEntries();
  for (const auto &Wp : Waypoints)
  {
    const FTransform Trans = ParamCarlaMap->ComputeTransform(Wp);
    AVehicleSpawnPoint *Spawner = UEditorLevelLibrary::GetEditorWorld()->SpawnActor<AVehicleSpawnPoint>();
    Spawner->SetActorRotation(Trans.GetRotation());
    Spawner->SetActorLocation(Trans.GetTranslation() + FVector(0.f, 0.f, SpawnersHeight));
  }
}

void UOpenDriveToMap::GenerateTreePositions( const boost::optional<carla::road::Map>& ParamCarlaMap )
{
  std::vector<std::pair<carla::geom::Transform, std::string>> Locations =
    ParamCarlaMap->GetTreesTransform(DistanceBetweenTrees, DistanceFromRoadEdge );
  int i = 0;
  for (auto &cl : Locations)
  {
    const FVector scale{ 1.0f, 1.0f, 1.0f };
    cl.first.location.z  = GetHeight(cl.first.location.x, cl.first.location.y) + 0.3f;
    FTransform NewTransform ( FRotator(cl.first.rotation), FVector(cl.first.location), scale );
    NewTransform = GetSnappedPosition(NewTransform);

    AActor* Spawner = UEditorLevelLibrary::GetEditorWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
      NewTransform.GetLocation(), NewTransform.Rotator());

    Spawner->Tags.Add(FName("TreeSpawnPosition"));
    Spawner->Tags.Add(FName(cl.second.c_str()));
    Spawner->SetActorLabel("TreeSpawnPosition" + FString::FromInt(i) );
    ++i;
  }
}

float UOpenDriveToMap::GetHeight(float PosX, float PosY, bool bDrivingLane){
  if( bDrivingLane ){
    return carla::geom::deformation::GetZPosInDeformation(PosX, PosY) +
      (carla::geom::deformation::GetZPosInDeformation(PosX, PosY) * -0.3f) -
      carla::geom::deformation::GetBumpDeformation(PosX,PosY);
  }else{
    return carla::geom::deformation::GetZPosInDeformation(PosX, PosY) + (carla::geom::deformation::GetZPosInDeformation(PosX, PosY) * -0.3f);
  }
}

FTransform UOpenDriveToMap::GetSnappedPosition( FTransform Origin ){
  FTransform ToReturn = Origin;
  FVector Start = Origin.GetLocation() + FVector( 0, 0, 1000);
  FVector End = Origin.GetLocation() - FVector( 0, 0, 1000);
  FHitResult HitResult;
  FCollisionQueryParams CollisionQuery;
  CollisionQuery.bTraceComplex = true;
  FCollisionResponseParams CollisionParams;

  if( UEditorLevelLibrary::GetEditorWorld()->LineTraceSingleByChannel(
    HitResult,
    Start,
    End,
    ECollisionChannel::ECC_WorldStatic,
    CollisionQuery,
    CollisionParams
  ) )
  {
    ToReturn.SetLocation(HitResult.Location);
  }
  return ToReturn;
}

float UOpenDriveToMap::GetHeightForLandscape( FVector Origin ){
  FVector Start = Origin + FVector( 0, 0, 10000);
  FVector End = Origin - FVector( 0, 0, 10000);
  FHitResult HitResult;
  FCollisionQueryParams CollisionQuery;
  CollisionQuery.AddIgnoredActors(Landscapes);
  FCollisionResponseParams CollisionParams;

  if( UEditorLevelLibrary::GetEditorWorld()->LineTraceSingleByChannel(
    HitResult,
    Start,
    End,
    ECollisionChannel::ECC_WorldStatic,
    CollisionQuery,
    CollisionParams
  ) )
  {
    return GetHeight(Origin.X * 0.01f, Origin.Y * 0.01f, true) * 100.0f - 50.0f;
  }else{
    return GetHeight(Origin.X * 0.01f, Origin.Y * 0.01f, true) * 100.0f;
  }
  return 0.0f;
}

float UOpenDriveToMap::DistanceToLaneBorder(const boost::optional<carla::road::Map>& ParamCarlaMap,
        FVector &location, int32_t lane_type ) const
{
  carla::geom::Location cl(location);
  //wp = GetClosestWaypoint(pos). if distance wp - pos == lane_width --> estas al borde de la carretera
  auto wp = ParamCarlaMap->GetClosestWaypointOnRoad(cl, lane_type);
  if(wp)
  {
    carla::geom::Transform ct = ParamCarlaMap->ComputeTransform(*wp);
    double LaneWidth = ParamCarlaMap->GetLaneWidth(*wp);
    return cl.Distance(ct.location) - LaneWidth;
  }
  return 100000.0f;
}

bool UOpenDriveToMap::IsInRoad(
  const boost::optional<carla::road::Map>& ParamCarlaMap,
  FVector &location)
{
  int32_t start = static_cast<int32_t>(carla::road::Lane::LaneType::Driving);
  int32_t end = static_cast<int32_t>(carla::road::Lane::LaneType::Sidewalk);
  for( int32_t i = start; i < end; ++i)
  {
    if(ParamCarlaMap->GetWaypoint(location, i))
    {
      return true;
    }
  }
  return false;
}

