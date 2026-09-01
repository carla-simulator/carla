// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the
// terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "OpenDriveToMap.h"

#include <util/disable-ue4-macros.h>
#include <carla/geom/BoundingBox.h>
#include <carla/geom/Location.h>
#include <carla/geom/Transform.h>
#include <carla/road/Junction.h>
#include <carla/road/Road.h>
#include <carla/road/RoadTypes.h>
#include <carla/road/element/Waypoint.h>
#include <util/enable-ue4-macros.h>
#include "Containers/ContainersFwd.h"
#include "CoreGlobals.h"
#include "EngineUtils.h"
#include "Logging/LogMacros.h"
#include "Logging/LogVerbosity.h"
#include "Math/MathFwd.h"
#include "TrafficLights/TLModule.h"
#if ENGINE_MAJOR_VERSION < 5
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"
#else
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#endif
#include "Engine/LevelBounds.h"
#include "Engine/SceneCapture2D.h"
#include "FileHelpers.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "KismetProceduralMeshLibrary.h"
#include "Misc/FileHelper.h"
#include "Online/CustomFileDownloader.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "StaticMeshAttributes.h"
#if ENGINE_MAJOR_VERSION < 5
#include "Engine/Classes/Interfaces/Interface_CollisionDataProvider.h"
#endif
#include "Engine/AssetManager.h"
#include "Engine/TriggerBox.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#if ENGINE_MAJOR_VERSION < 5
#include "PhysicsCore/Public/BodySetupEnums.h"
#endif
#include "PhysicsEngine/BodySetup.h"
#include "RawMesh.h"
#if ENGINE_MAJOR_VERSION < 5
#include "AssetRegistryModule.h"
#endif
#include "EditorLevelLibrary.h"
#include "LevelEditorSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MeshDescription.h"
#include "ProceduralMeshConversion.h"
#if ENGINE_MAJOR_VERSION > 4
#include "Editor/Transactor.h"
#include "Subsystems/UnrealEditorSubsystem.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#endif
#include "Carla/MapGen/LargeMapManager.h"
#include "Carla/Util/ProceduralCustomMesh.h"
#include "BlueprintUtilFunctions.h"
#include <util/disable-ue4-macros.h>
#include <carla/road/Deformation.h>
#include <carla/geom/Simplification.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <carla/road/element/RoadInfoSignal.h>
#include <util/enable-ue4-macros.h>
#include "CarlaTools.h"
#include "ContentBrowserModule.h"
#include "DrawDebugHelpers.h"
#include "EditorLevelUtils.h"
#include "GameFramework/Actor.h"
#include "Carla/BlueprintLibary/MapGenFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Math/Vector.h"
#include "GenerationPathsHelper.h"
#include "StreetMapActor.h"

#if WITH_EDITOR
#include <util/disable-ue4-macros.h>
#include <carla/road/InformationSet.h>
#include <carla/road/Signal.h>
#include <carla/road/SignalType.h>
#include <util/enable-ue4-macros.h>
#include "OpenDriveToMap.h"
#include "TrafficLights/TLMeshFactory.h"
#include "TrafficLights/TLPole.h"
#include "TrafficLights/TrafficLightActor.h"
#if WITH_EDITOR
#include "FileHelpers.h"
#endif
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "HighResScreenshot.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "IMeshMergeUtilities.h"
#include "ImageUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "MeshMergeModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Online/CustomFileDownloader.h"
#include "RHICommandList.h"
#include "RenderUtils.h"
#include "Runtime/ImageWriteQueue/Public/ImagePixelData.h"
#include "Runtime/ImageWriteQueue/Public/ImageWriteQueue.h"
#include "Runtime/ImageWriteQueue/Public/ImageWriteTask.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SoftObjectPath.h"
// #include "Utils/GoogleStreetViewManager.h"
#include "Carla/Util/GeometryImporter.h"

struct FTerrainMeshData
{
	int32 MeshIndex;
	FVector2D Offset;
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector2D> UVs;
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;
};

UOpenDriveToMap::UOpenDriveToMap()
{
	AddToRoot();
	bRoadsFinished = false;
	bHasStarted = false;
	bMapLoaded = false;
#ifdef _WIN32
	PythonBinPath = TEXT("python");
#else
	PythonBinPath = TEXT("/usr/bin/python3");
#endif
}

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
	FilePath = UGenerationPathsHelper::GetRawMapDirectoryPath(MapName) + "OpenDrive/" + MapName + ".osm";
	FileDownloader->ConvertOSMInOpenDrive(FilePath, OriginGeoCoordinates.X, OriginGeoCoordinates.Y, OpenDriveGenParams);
	FilePath.RemoveFromEnd(".osm", ESearchCase::Type::IgnoreCase);
	FilePath += ".xodr";

	DownloadFinished();
	UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	LoadMap();
}

void UOpenDriveToMap::CreateMap()
{
	if (MapName.IsEmpty())
	{
		UE_LOG(LogCarlaTools, Error, TEXT("Map Name Is Empty"));
		return;
	}

	if (!Url.IsEmpty())
	{
		if (!IsValid(FileDownloader))
		{
			FileDownloader = NewObject<UCustomFileDownloader>();
		}

		FileDownloader->ResultFileName = MapName;
		FileDownloader->Url = Url;

		FileDownloader->DownloadDelegate.BindUObject(this, &UOpenDriveToMap::ConvertOSMInOpenDrive);
		FileDownloader->StartDownload();
	}
	else if (LocalFilePath.EndsWith(".xodr"))
	{
		ImportXODR();
	}
	else if (LocalFilePath.EndsWith(".osm"))
	{
		ImportOSM();
	}
	else
	{
		UE_LOG(LogCarlaTools, Error, TEXT("URL and Local FilePath are Empty. URL: %s  Local FilePath: %s"), *Url,
			*LocalFilePath);
	}
}

void UOpenDriveToMap::CreateTerrain(const int NumberOfTerrainX, const int NumberOfTerrainY, const float MeshGridResolution)
{
	if (NumberOfTerrainX <= 0 || NumberOfTerrainY <= 0 || MeshGridResolution <= 0)
		return;

	float TileSizeX = TileSize / NumberOfTerrainX;
	float TileSizeY = TileSize / NumberOfTerrainY;

	FVector MinBox(MinPosition.X, MaxPosition.Y, 0);

	TArray<FTerrainMeshData> AllMeshData;
	AllMeshData.SetNum((NumberOfTerrainX) * (NumberOfTerrainY));

	for (int32 y = 0; y < NumberOfTerrainY; ++y)
	{
		for (int32 x = 0; x < NumberOfTerrainX; ++x)
		{
			int32 Index = x + y * NumberOfTerrainX;

			FVector2D Offset(MinBox.X + x * TileSizeX, MinBox.Y + y * TileSizeY);

			FTerrainMeshData& MeshData = AllMeshData[Index];
			MeshData.MeshIndex = Index;
			MeshData.Offset = Offset;

			const int32 VertsX = MeshGridResolution + 1;
			const int32 VertsY = MeshGridResolution + 1;
			const float StepX = TileSizeX / MeshGridResolution;
			const float StepY = TileSizeY / MeshGridResolution;

			TArray<FVector>& Vertices = MeshData.Vertices;
			TArray<int32>& Triangles = MeshData.Triangles;
			TArray<FVector2D>& UVs = MeshData.UVs;
			TArray<FVector>& Normals = MeshData.Normals;
			TArray<FProcMeshTangent>& Tangents = MeshData.Tangents;

			Vertices.Reserve(VertsX * VertsY);
			UVs.Reserve(VertsX * VertsY);
			Triangles.Reserve((VertsX - 1) * (VertsY - 1) * 6);

			for (int32 iy = 0; iy < VertsY; ++iy)
			{
				for (int32 ix = 0; ix < VertsX; ++ix)
				{
					float X = ix * StepX;
					float Y = iy * StepY;
					float Height = GetHeightForLandscape(FVector(Offset.X + X, Offset.Y + Y, 0));
					Vertices.Add(FVector(X, Y, Height));
					UVs.Add(FVector2D(static_cast<float>(ix) / MeshGridResolution, static_cast<float>(iy) / MeshGridResolution));
				}
			}

			for (int32 iy = 0; iy < VertsY - 1; ++iy)
			{
				for (int32 ix = 0; ix < VertsX - 1; ++ix)
				{
					int32 i0 = ix + iy * VertsX;
					int32 i1 = (ix + 1) + iy * VertsX;
					int32 i2 = ix + (iy + 1) * VertsX;
					int32 i3 = (ix + 1) + (iy + 1) * VertsX;

					Triangles.Add(i0);
					Triangles.Add(i2);
					Triangles.Add(i1);

					Triangles.Add(i3);
					Triangles.Add(i1);
					Triangles.Add(i2);
				}
			}
		}
	}

	for (const FTerrainMeshData& MeshData : AllMeshData)
	{
		TArray<FVector> Normals;
		TArray<FProcMeshTangent> Tangents;

		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
			MeshData.Vertices, MeshData.Triangles, MeshData.UVs, Normals, Tangents);

		FProceduralCustomMesh ProcMeshData;
		ProcMeshData.Vertices = MeshData.Vertices;
		ProcMeshData.Triangles = MeshData.Triangles;
		ProcMeshData.Normals = Normals;
		ProcMeshData.UV0 = MeshData.UVs;

		UObject* DuplicatedMaterialObject = UBlueprintUtilFunctions::CopyAssetToPlugin(DefaultLandscapeMaterial, MapName);
		UMaterialInstance* DuplicatedLandscapeMaterial = Cast<UMaterialInstance>(DuplicatedMaterialObject);

		UStaticMesh* StaticMesh = UMapGenFunctionLibrary::CreateMesh(ProcMeshData, Tangents, DuplicatedLandscapeMaterial, MapName,
			"Terrain", FName(*FString::Printf(TEXT("SM_LandscapeMesh_%d%s"), MeshData.MeshIndex, *GetStringForCurrentTile())));

		if (!StaticMesh)
			continue;

		UWorld* World = GetEditorWorld();

		if (!World)
			continue;

		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(), FVector(MeshData.Offset.X, MeshData.Offset.Y, 0), FRotator::ZeroRotator);
		if (!Actor)
			continue;

		UStaticMeshComponent* MeshComp = Actor->GetStaticMeshComponent();

		MeshComp->SetStaticMesh(StaticMesh);

		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetCollisionObjectType(ECC_WorldStatic);
		MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
		MeshComp->SetMobility(EComponentMobility::Static);

		MeshComp->SetGenerateOverlapEvents(true);
		MeshComp->bReturnMaterialOnMove = true;
		Actor->SetActorLabel(FString::Printf(TEXT("LandscapeActor_%d%s"), MeshData.MeshIndex, *GetStringForCurrentTile()));
		Actor->Tags.Add("LandscapeToMove");
		MeshComp->CastShadow = false;

#if ENGINE_MAJOR_VERSION > 4
		Actor->SetIsSpatiallyLoaded(true);
#endif
		Landscapes.Add(Actor);
	}
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (World)
	{
		FString CurrentMapName = World->GetMapName();
		CurrentMapName.RemoveFromStart(World->StreamingLevelsPrefix);
		UGameplayStatics::OpenLevel(World, FName(*CurrentMapName));
	}
}

void UOpenDriveToMap::CreateTerrainMesh(
	const int MeshIndex, const FVector2D Offset, const int TileSizeX, const int TileSizeY, const float MeshResolution)
{
	// const float GridSectionSize = 100.0f; // In cm
	const float HeightScale = 3.0f;

	UWorld* World = GetEditorWorld();
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

	int VerticesInLineX = MeshResolution + 1;
	int VerticesInLineY = MeshResolution + 1;
	float SpaceBetweenVerticesX = static_cast<float>(TileSizeX) / MeshResolution;
	float SpaceBetweenVerticesY = static_cast<float>(TileSizeY) / MeshResolution;
	static int StaticMeshIndex = 0;
	for (int i = 0; i < VerticesInLineX; i++)
	{
		float X = (i * SpaceBetweenVerticesX);
		for (int j = 0; j < VerticesInLineY; j++)
		{
			float Y = (j * SpaceBetweenVerticesY);
			float HeightValue = GetHeightForLandscape(FVector((Offset.X + X), (Offset.Y + Y), 0));
			Vertices.Add(FVector(X, Y, HeightValue));
			UVs.Add(FVector2D(i, j));
		}
	}

	//// Triangles formation. 2 triangles per section.
	for (int i = 0; i < VerticesInLineX - 1; i++)
	{
		for (int j = 0; j < VerticesInLineY - 1; j++)
		{
			Triangles.Add(j + (i * VerticesInLineX));
			Triangles.Add((j + 1) + (i * VerticesInLineX));
			Triangles.Add(j + ((i + 1) * VerticesInLineX));

			Triangles.Add((j + 1) + (i * VerticesInLineX));
			Triangles.Add((j + 1) + ((i + 1) * VerticesInLineX));
			Triangles.Add(j + ((i + 1) * VerticesInLineX));
		}
	}

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVs, Normals, Tangents);

	FProceduralCustomMesh MeshData;
	MeshData.Vertices = Vertices;
	MeshData.Triangles = Triangles;
	MeshData.Normals = Normals;
	MeshData.UV0 = UVs;

	UObject* DuplicatedMaterialObject = UBlueprintUtilFunctions::CopyAssetToPlugin(DefaultLandscapeMaterial, MapName);
	UMaterialInstance* DuplicatedLandscapeMaterial = Cast<UMaterialInstance>(DuplicatedMaterialObject);

	UStaticMesh* MeshToSet = UMapGenFunctionLibrary::CreateMesh(MeshData, Tangents, DuplicatedLandscapeMaterial, MapName, "Terrain",
		FName(TEXT("SM_LandscapeMesh" + FString::FromInt(StaticMeshIndex) + GetStringForCurrentTile())));
	Mesh->SetStaticMesh(MeshToSet);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldStatic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetMobility(EComponentMobility::Static);
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->bReturnMaterialOnMove = true;

	MeshActor->SetActorLabel("SM_LandscapeActor" + FString::FromInt(StaticMeshIndex) + GetStringForCurrentTile());
	MeshActor->Tags.Add(FName("LandscapeToMove"));
#if ENGINE_MAJOR_VERSION > 4
	MeshActor->SetIsSpatiallyLoaded(true);
#endif
	Mesh->CastShadow = false;
	Landscapes.Add(MeshActor);
	StaticMeshIndex++;
}

AActor* UOpenDriveToMap::SpawnActorWithCheckNoCollisions(UClass* ActorClassToSpawn, FTransform Transform)
{
	UWorld* World = GetEditorWorld();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.bNoFail = true;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Creation of the procedural mesh
	return World->SpawnActor<AActor>(ActorClassToSpawn, Transform, SpawnParameters);
}
void UOpenDriveToMap::GenerateTileStandalone()
{
	UE_LOG(LogCarlaTools, Log, TEXT("UOpenDriveToMap::GenerateTileStandalone Function called"));

#if PLATFORM_WINDOWS
	GenerateTile();
#else
	GenerateTile();
#endif
	UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
	GEditor->GetEditorSubsystem<ULevelEditorSubsystem>()->SaveCurrentLevel();
}

void UOpenDriveToMap::GenerateTile()
{
	if (FilePath.IsEmpty())
	{
		UE_LOG(LogCarlaTools, Warning, TEXT("UOpenDriveToMap::GenerateTile(): Failed to load %s"), *FilePath);
		return;
	}

	FString FileContent;
	FFileHelper::LoadFileToString(FileContent, *FilePath);
	std::string opendrive_xml = carla::rpc::FromLongFString(FileContent);
	UE_LOG(LogCarlaTools, Warning, TEXT("UOpenDriveToMap::GenerateTile() Loading File..... "));
	CarlaMap = carla::opendrive::OpenDriveParser::Load(opendrive_xml);

	if (!CarlaMap.has_value())
	{
		UE_LOG(LogCarlaTools, Error, TEXT("Invalid Map"));
	}
	else
	{
		UE_LOG(LogCarlaTools, Warning, TEXT("Valid Map loaded"));
		MapName = FPaths::GetCleanFilename(FilePath);
		MapName.RemoveFromEnd(".xodr", ESearchCase::Type::IgnoreCase);
		UE_LOG(LogCarlaTools, Warning, TEXT("MapName %s"), *MapName);

#if ENGINE_MAJOR_VERSION < 5
		GEditor->GetEditorSubsystem<ULevelEditorSubsystem>()->LoadLevel(*BaseLevelName);
		AActor* QueryActor = UGameplayStatics::GetActorOfClass(GetEditorWorld(), ALargeMapManager::StaticClass());
		if (QueryActor != nullptr)
		{
			ALargeMapManager* LmManager = Cast<ALargeMapManager>(QueryActor);
			LmManager->GenerateMap_Editor();
			NumTilesInXY = LmManager->GetNumTilesInXY();
			TileSize = LmManager->GetTileSize();
			Tile0Offset = LmManager->GetTile0Offset();
			GEditor->GetEditorSubsystem<ULevelEditorSubsystem>()->SaveCurrentLevel();
			LmManager->GetCarlaMapTile(CurrentTilesInXY);
			FCarlaMapTile& CarlaTile = LmManager->GetCarlaMapTile(CurrentTilesInXY);

			UE_LOG(LogCarlaTools, Warning, TEXT("Current Tile is %s"), *(CurrentTilesInXY.ToString()));
			UE_LOG(LogCarlaTools, Warning, TEXT("NumTilesInXY is %s"), *(NumTilesInXY.ToString()));
			UE_LOG(LogCarlaTools, Warning, TEXT("TileSize is %f"), (TileSize));
			UE_LOG(LogCarlaTools, Warning, TEXT("Tile0Offset is %s"), *(Tile0Offset.ToString()));
			UE_LOG(LogCarlaTools, Warning, TEXT("Tile Name is %s"), *(CarlaTile.Name));

			GEditor->GetEditorSubsystem<ULevelEditorSubsystem>()->LoadLevel(CarlaTile.Name);
#endif
			MinPosition = FVector(CurrentTilesInXY.X * TileSize, CurrentTilesInXY.Y * -TileSize, 0.0f);
			MaxPosition = FVector((CurrentTilesInXY.X + 1.0f) * TileSize, (CurrentTilesInXY.Y + 1.0f) * -TileSize, 0.0f);

			WorldOriginPosition = FVector(0, 0, 0);
			WorldEndPosition = FVector(UMapGenFunctionLibrary::GetTransversemercProjection(FinalGeoCoordinates.X,
										   FinalGeoCoordinates.Y, OriginGeoCoordinates.X, OriginGeoCoordinates.Y),
				0);

			GenerateAll(CarlaMap, MinPosition, MaxPosition);

			bHasStarted = true;
			bRoadsFinished = true;
			bMapLoaded = true;
			bTileFinished = false;
#if ENGINE_MAJOR_VERSION < 5
		}
		else
		{
			UE_LOG(LogCarlaTools, Error, TEXT("Largemapmanager not found "));
		}
#endif
		UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
		GEditor->GetEditorSubsystem<ULevelEditorSubsystem>()->SaveCurrentLevel();

#if ENGINE_MAJOR_VERSION < 5
#if PLATFORM_LINUX
		RemoveFromRoot();
#endif
#endif
	}
}

bool UOpenDriveToMap::GoNextTile()
{
	CurrentTilesInXY.X++;
	if (CurrentTilesInXY.X >= NumTilesInXY.X)
	{
		CurrentTilesInXY.X = 0;
		CurrentTilesInXY.Y++;
		if (CurrentTilesInXY.Y >= NumTilesInXY.Y)
		{
			return false;
		}
	}
	return true;
}

void UOpenDriveToMap::ReturnToMainLevel()
{
	FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
	GEditor->GetEditorSubsystem<ULevelEditorSubsystem>()->LoadLevel(*BaseLevelName);
}

void UOpenDriveToMap::CorrectPositionForAllActorsInCurrentTile()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetEditorWorld(), AActor::StaticClass(), FoundActors);
	for (AActor* Current : FoundActors)
	{
		Current->AddActorWorldOffset(-MinPosition, false);
		if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Current))
		{
			UStaticMesh* StaticMesh = MeshActor->GetStaticMeshComponent()->GetStaticMesh();
			if (StaticMesh)
				StaticMesh->ClearFlags(RF_Standalone);
		}
	}
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	GEngine->PerformGarbageCollectionAndCleanupActors();
}

FString UOpenDriveToMap::GetStringForCurrentTile()
{
	return FString("_X_") + FString::FromInt(CurrentTilesInXY.X) + FString("_Y_") + FString::FromInt(CurrentTilesInXY.Y);
}

AActor* UOpenDriveToMap::SpawnActorInEditorWorld(UClass* Class, FVector Location, FRotator Rotation)
{
	return GetEditorWorld()->SpawnActor<AActor>(Class, Location, Rotation);
}

void UOpenDriveToMap::OpenFileDialog()
{
	TArray<FString> OutFileNames;
	void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		DesktopPlatform->OpenFileDialog(
			ParentWindowPtr, "Select xodr file", FPaths::ProjectDir(), FString(""), ".xodr", 1, OutFileNames);
	}
	for (FString& CurrentString : OutFileNames)
	{
		FilePath = CurrentString;
		UE_LOG(LogCarlaTools, Log, TEXT("FileObtained %s"), *CurrentString);
	}
}

UWorld* UOpenDriveToMap::GetEditorWorld()
{
	UUnrealEditorSubsystem* UnrealEditorSubsystem = GEditor->GetEditorSubsystem<UUnrealEditorSubsystem>();

	// Check if the world is valid
	if (UnrealEditorSubsystem)
	{
		return UnrealEditorSubsystem->GetEditorWorld();
	}
	UE_LOG(LogCarlaTools, Error, TEXT("Not Editor subsystem found"));
	return nullptr;
}

UWorld* UOpenDriveToMap::GetGameWorld()
{
	UUnrealEditorSubsystem* UnrealEditorSubsystem = GEditor->GetEditorSubsystem<UUnrealEditorSubsystem>();

	// Check if the world is valid
	if (UnrealEditorSubsystem)
	{
		return UnrealEditorSubsystem->GetGameWorld();
	}
	UE_LOG(LogCarlaTools, Error, TEXT("Not Editor subsystem found"));
	return nullptr;
}

void UOpenDriveToMap::LoadMap()
{
	if (FilePath.IsEmpty())
	{
		return;
	}

	FilePath = FPaths::ConvertRelativePathToFull(FilePath);

	FString FileContent;
	UE_LOG(LogCarlaTools, Log, TEXT("UOpenDriveToMap::LoadMap(): File to load %s"), *FilePath);
	FFileHelper::LoadFileToString(FileContent, *FilePath);
	std::string opendrive_xml = carla::rpc::FromLongFString(FileContent);
	CarlaMap = carla::opendrive::OpenDriveParser::Load(opendrive_xml);

	if (!CarlaMap.has_value())
	{
		UE_LOG(LogCarlaTools, Error, TEXT("Invalid Map"));
	}
	else
	{
		UE_LOG(LogCarlaTools, Warning, TEXT("Valid Map loaded"));
		MapName = FPaths::GetCleanFilename(FilePath);
		MapName.RemoveFromEnd(".xodr", ESearchCase::Type::IgnoreCase);
		UE_LOG(LogCarlaTools, Warning, TEXT("MapName %s"), *MapName);

		AActor* QueryActor = UGameplayStatics::GetActorOfClass(GetEditorWorld(), ALargeMapManager::StaticClass());
#if ENGINE_MAJOR_VERSION < 5
		if (QueryActor != nullptr)
		{
			ALargeMapManager* LargeMapManager = Cast<ALargeMapManager>(QueryActor);
			NumTilesInXY = LargeMapManager->GetNumTilesInXY();
			TileSize = LargeMapManager->GetTileSize();
			Tile0Offset = LargeMapManager->GetTile0Offset();
			CurrentTilesInXY = FIntVector(0, 0, 0);
			ULevel* PersistantLevel = GetEditorWorld()->PersistentLevel;
			BaseLevelName = LargeMapManager->LargeMapTilePath + "/" + LargeMapManager->LargeMapName;
			do
			{
				GenerateTileStandalone();
			} while (GoNextTile());
			ReturnToMainLevel();
		}
#else
		UWorld* World = GetEditorWorld();

		StreetMapActorReference = Cast<AStreetMapActor>(UGameplayStatics::GetActorOfClass(World, AStreetMapActor::StaticClass()));
		if (!IsValid(StreetMapActorReference))
		{
			UE_LOG(LogCarlaTools, Error, TEXT("StreetMapActorReference is not valid"));
		}

		if (DefaultHeightmap)
		{
			HeightmapCopy = DefaultHeightmap->GetCPUCopy();
			HeightmapPixels = HeightmapCopy->AsG16();
		}

		for (const TSubclassOf<UDGTImplementable>& ToolClass : ToolsClasses)
		{
			if (!ToolClass)
			{
				UE_LOG(LogCarlaTools, Error, TEXT("ToolToInstantiate is null at index %d"),
					ToolsClasses.IndexOfByKey(ToolClass));
				continue;
			}

			// Log the class name before instantiation
			UE_LOG(LogCarlaTools, Log, TEXT("Instantiating tool of class: %s"), *ToolClass->GetName());

			UDGTImplementable* Tool = NewObject<UDGTImplementable>(this, ToolClass);
			if (!Tool)
			{
				UE_LOG(LogCarlaTools, Error, TEXT("Failed to instantiate tool of class: %s"), *ToolClass->GetName());
				continue;
			}

			ToolInstances.Add(Tool);

			UE_LOG(LogCarlaTools, Log, TEXT("Tool instance created: %s"), *Tool->GetName());
		}

		do
		{
			GenerateTileStandalone();
		} while (GoNextTile());

		if (IsValid(StreetMapActorReference))
		{
			GeneratedSplines.Append(StreetMapActorReference->SpawnTaggedTerrainSplines());
		}
		else
		{
			UE_LOG(LogCarlaTools, Error, TEXT("StreetMapActorReference is not valid"));
		}

		if (World)
		{
			FString CurrentMapName = World->GetMapName();
			CurrentMapName.RemoveFromStart(World->StreamingLevelsPrefix);
			UGameplayStatics::OpenLevel(World, FName(*CurrentMapName));
		}

		GenerateCurbSplinesFromRoadRenders();

		for (UDGTImplementable* Tool : ToolInstances)
		{
			if (Tool)
			{
				Tool->RunUtilityFunction(World, this);
				Tool->RunUtilityFunctionWithSplinesParamters(World, this, GeneratedSplines);
			}
		}

		RemoveFromRoot();
#endif
		Landscapes.Empty();
	}
}

void UOpenDriveToMap::GenerateCurbSplinesFromRoadRenders()
{
	UE_LOG(LogCarlaTools, Log, TEXT("Generating road renders and splines for curbs"));

	CurrentTilesInXY.X = 0;
	CurrentTilesInXY.Y = 0;

	do
	{
		MinPosition = FVector(CurrentTilesInXY.X * TileSize, CurrentTilesInXY.Y * -TileSize, 0.0f);
		MaxPosition = FVector((CurrentTilesInXY.X + 1.0f) * TileSize, (CurrentTilesInXY.Y + 1.0f) * -TileSize, 0.0f);

		RenderRoadToTexture(MinPosition, MaxPosition);

	} while (GoNextTile());

	GenerateCurbSplines();
}

TArray<AActor*> UOpenDriveToMap::GenerateMiscActors(float Offset, FVector MinLocation, FVector MaxLocation)
{
	carla::geom::Vector3D CarlaMinLocation(MinLocation.X / 100, MinLocation.Y / 100, MinLocation.Z / 100);
	carla::geom::Vector3D CarlaMaxLocation(MaxLocation.X / 100, MaxLocation.Y / 100, MaxLocation.Z / 100);

	std::vector<std::pair<carla::geom::Transform, std::string>> Locations =
		CarlaMap->GetTreesTransform(CarlaMinLocation, CarlaMaxLocation, DistanceBetweenTrees, DistanceFromRoadEdge, Offset);
	TArray<AActor*> Returning;
	static int i = 0;
	for (auto& cl : Locations)
	{
		const FVector scale{1.0f, 1.0f, 1.0f};
		cl.first.location.z = GetHeight(cl.first.location.x, cl.first.location.y) / 100.0f;
		FTransform NewTransform(FRotator(cl.first.rotation), FVector(cl.first.location), scale);

		NewTransform = GetSnappedPosition(NewTransform);

		AActor* Spawner = GetEditorWorld()->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(), NewTransform.GetLocation(), NewTransform.Rotator());
		Spawner->Tags.Add(FName("MiscSpawnPosition"));
		Spawner->Tags.Add(FName(cl.second.c_str()));
		Spawner->SetActorLabel("MiscSpawnPosition" + FString::FromInt(i));
#if ENGINE_MAJOR_VERSION > 4
		Spawner->SetIsSpatiallyLoaded(true);
#endif
		++i;
		Returning.Add(Spawner);
	}
	return Returning;
}

void UOpenDriveToMap::GenerateAll(const std::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation)
{
	UE_LOG(LogCarlaTools, Log, TEXT("UOpenDriveToMap::GenerateAll() Generating Roads..... "));
	GenerateRoadMesh(ParamCarlaMap, MinLocation, MaxLocation);

	UE_LOG(LogCarlaTools, Log, TEXT("UOpenDriveToMap::GenerateAll() Generating Lane Marks..... "));
	GenerateLaneMarks(ParamCarlaMap, MinLocation, MaxLocation);

	UE_LOG(LogCarlaTools, Log, TEXT("UOpenDriveToMap::GenerateAll() Generating Terrain..... "));
	CreateTerrain(5, 5, 64);

	UE_LOG(LogCarlaTools, Log, TEXT("UOpenDriveToMap::GenerateAll() Generating Traffic Lights..... "));
	GenerateTrafficLights(ParamCarlaMap, MinLocation, MaxLocation);

	UE_LOG(LogCarlaTools, Log, TEXT("UOpenDriveToMap::GenerateAll() Generating Misc stuff..... "));
	GenerationFinished(MinLocation, MaxLocation);

#if PLATFORM_LINUX
	if (bUseMitsuba)
	{
		MitsubaMeshOptimization();
	}
#endif
}

void UOpenDriveToMap::GenerateRoadMesh(
	const std::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation)
{
	opg_parameters.vertex_distance = 0.5f;
	opg_parameters.vertex_width_resolution = 8.0f;
#if ENGINE_MAJOR_VERSION < 5
	opg_parameters.simplification_percentage = 50.0f;
#else
	opg_parameters.simplification_percentage = 0.0f;
#endif
	double start = FPlatformTime::Seconds();

	carla::geom::Vector3D CarlaMinLocation(MinLocation.X / 100, MinLocation.Y / 100, MinLocation.Z / 100);
	carla::geom::Vector3D CarlaMaxLocation(MaxLocation.X / 100, MaxLocation.Y / 100, MaxLocation.Z / 100);
	UE_LOG(LogCarlaTools, Log, TEXT(" Generating roads between %s  and %s"), *(CarlaMinLocation.ToFVector().ToString()),
		*(CarlaMaxLocation.ToFVector().ToString()));
	const auto Meshes = ParamCarlaMap->GenerateOrderedChunkedMeshInLocations(opg_parameters, CarlaMinLocation, CarlaMaxLocation);
	double end = FPlatformTime::Seconds();
	UE_LOG(LogCarlaTools, Log,
		TEXT(" GenerateOrderedChunkedMesh code executed in %f seconds. Simplification percentage is %f"), end - start,
		opg_parameters.simplification_percentage);

	start = FPlatformTime::Seconds();
	static int index = 0;

	struct FPreparedMeshData
	{
		FProceduralCustomMesh MeshData;
		FVector MeshCentroid;
		carla::road::Lane::LaneType LaneType;
		int32 Index;
	};

	TArray<FPreparedMeshData> PreparedMeshes;
	FCriticalSection Mutex;
	int32 LocalIndex = 0;

	for (const auto& PairMap : Meshes)
	{
		const auto& LaneType = PairMap.first;
		const auto& MeshList = PairMap.second;

		ParallelFor(MeshList.size(),
			[&](int32 i)
			{
				const auto& Mesh = MeshList[i];
				if (!Mesh->IsValid() || (Mesh->GetVertices().size() == 0))
					return;

				auto& Vertices = Mesh->GetVertices();

				if (LaneType == carla::road::Lane::LaneType::Driving)
				{
					for (auto& Vertex : Vertices)
					{
						FVector FV = Vertex.ToFVector();
						Vertex.z +=
							GetHeight(Vertex.x * 100.0f, Vertex.y * 100.0f, DistanceToLaneBorder(ParamCarlaMap, FV) > 65.0f) /
							100.0f;
					}
#if ENGINE_MAJOR_VERSION < 5
					carla::geom::Simplification Simplify(0.15);
					Simplify.Simplificate(Mesh);
#endif
				}
				else
				{
					for (auto& Vertex : Vertices)
					{
						Vertex.z += (GetHeight(Vertex.x * 100.0f, Vertex.y * 100.0f, false) + 15.0f) / 100.0f;
					}
				}

				FVector Centroid(0);
				for (const auto& V : Vertices)
					Centroid += V.ToFVector();
				Centroid /= Vertices.size();

				for (auto& V : Vertices)
				{
					V.x -= Centroid.X;
					V.y -= Centroid.Y;
					V.z -= Centroid.Z;
				}

				FPreparedMeshData Data;
				Data.MeshData = *Mesh;
				Data.MeshCentroid = Centroid;
				Data.LaneType = LaneType;

				int32 AssignedIndex;
				{
					FScopeLock Lock(&Mutex);
					AssignedIndex = LocalIndex++;
					Data.Index = AssignedIndex;
					PreparedMeshes.Add(Data);
				}
			});
	}

	for (FPreparedMeshData& Entry : PreparedMeshes)
	{
		const FProceduralCustomMesh& Mesh = Entry.MeshData;
		const FVector& Centroid = Entry.MeshCentroid;
		const int32 Index = Entry.Index;
		const carla::road::Lane::LaneType LaneType = Entry.LaneType;

		TArray<FProcMeshTangent> Tangents;
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
			Entry.MeshData.Vertices, Entry.MeshData.Triangles, Entry.MeshData.UV0, Entry.MeshData.Normals, Tangents);

		AStaticMeshActor* TempActor = GetEditorWorld()->SpawnActor<AStaticMeshActor>();
		UStaticMeshComponent* StaticMeshComponent = TempActor->GetStaticMeshComponent();
		TempActor->SetActorLabel(FString("SM_Lane_") + FString::FromInt(Index));

		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		StaticMeshComponent->SetCollisionObjectType(ECC_WorldStatic);
		StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
		StaticMeshComponent->SetMobility(EComponentMobility::Static);

		StaticMeshComponent->SetGenerateOverlapEvents(true);
		StaticMeshComponent->bReturnMaterialOnMove = true;

		if (LaneType == carla::road::Lane::LaneType::Driving && DefaultRoadMaterial)
		{
			UObject* DuplicatedMaterialObject = UBlueprintUtilFunctions::CopyAssetToPlugin(DefaultRoadMaterial, MapName);
			UMaterialInstance* DuplicatedRoadMaterial = Cast<UMaterialInstance>(DuplicatedMaterialObject);

			StaticMeshComponent->SetMaterial(0, DuplicatedRoadMaterial);
			StaticMeshComponent->CastShadow = false;
			TempActor->SetActorLabel(FString("SM_DrivingLane_") + FString::FromInt(Index));
		}

		UStaticMesh* FinalMesh = nullptr;

		if (LaneType == carla::road::Lane::LaneType::Driving)
		{
			UObject* DuplicatedMaterialObject = UBlueprintUtilFunctions::CopyAssetToPlugin(DefaultRoadMaterial, MapName);
			UMaterialInstance* DuplicatedRoadMaterial = Cast<UMaterialInstance>(DuplicatedMaterialObject);

			FinalMesh = UMapGenFunctionLibrary::CreateMesh(Entry.MeshData, Tangents, DuplicatedRoadMaterial, MapName, "DrivingLane",
				FName(TEXT("SM_DrivingLaneMesh" + FString::FromInt(Index) + GetStringForCurrentTile())));
		}

		StaticMeshComponent->SetStaticMesh(FinalMesh);
		TempActor->SetActorLocation(Centroid * 100);
		TempActor->Tags.Add(FName("RoadLane"));
		TempActor->SetActorEnableCollision(true);

#if ENGINE_MAJOR_VERSION > 4
		TempActor->SetIsSpatiallyLoaded(true);
#endif
	}

	end = FPlatformTime::Seconds();
	UE_LOG(LogCarlaTools, Log, TEXT("Mesh spawnning and translation code executed in %f seconds."), end - start);

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (World)
	{
		FString CurrentMapName = World->GetMapName();
		CurrentMapName.RemoveFromStart(World->StreamingLevelsPrefix);
		UGameplayStatics::OpenLevel(World, FName(*CurrentMapName));
	}
}

void UOpenDriveToMap::GenerateLaneMarks(
	const std::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation)
{
	opg_parameters.vertex_distance = 0.5f;
	opg_parameters.vertex_width_resolution = 8.0f;
	opg_parameters.simplification_percentage = 15.0f;
	std::vector<std::string> lanemarkinfo;
	carla::geom::Vector3D CarlaMinLocation(MinLocation.X / 100, MinLocation.Y / 100, MinLocation.Z / 100);
	carla::geom::Vector3D CarlaMaxLocation(MaxLocation.X / 100, MaxLocation.Y / 100, MaxLocation.Z / 100);
	auto MarkingMeshes = ParamCarlaMap->GenerateLineMarkings(opg_parameters, CarlaMinLocation, CarlaMaxLocation, lanemarkinfo);
	TArray<AActor*> LaneMarkerActorList;
	static int meshindex = 0;
	int index = 0;
	for (const auto& Mesh : MarkingMeshes)
	{
		if (!Mesh->GetVertices().size())
		{
			index++;
			continue;
		}
		if (!Mesh->IsValid())
		{
			index++;
			continue;
		}

		FVector MeshCentroid = FVector(0, 0, 0);
		for (auto& Vertex : Mesh->GetVertices())
		{
			FVector VertexFVector = Vertex.ToFVector();
			Vertex.z +=
				GetHeight(Vertex.x * 100.0f, Vertex.y * 100.0f, DistanceToLaneBorder(ParamCarlaMap, VertexFVector) > 65.0f) /
					100.0f +
				0.01f;
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
		for (auto SpawnedActor : LaneMarkerActorList)
		{
			float VectorDistance = FVector::Distance(MeshCentroid * 100, SpawnedActor->GetActorLocation());
			if (VectorDistance < MinDistance)
			{
				MinDistance = VectorDistance;
			}
		}

		if (MinDistance < 250)
		{
			UE_LOG(LogCarlaTools, VeryVerbose, TEXT("Skkipped is %f."), MinDistance);
			index++;
			continue;
		}

		AStaticMeshActor* TempActor = GetEditorWorld()->SpawnActor<AStaticMeshActor>();
		UStaticMeshComponent* StaticMeshComponent = TempActor->GetStaticMeshComponent();
		TempActor->SetActorLabel(FString("SM_LaneMark_") + FString::FromInt(meshindex));
		StaticMeshComponent->CastShadow = false;
		if (lanemarkinfo[index].find("yellow") != std::string::npos)
		{
			if (DefaultLaneMarksYellowMaterial)
			{
				UObject* DuplicatedMaterialObject =
					UBlueprintUtilFunctions::CopyAssetToPlugin(DefaultLaneMarksYellowMaterial, MapName);
				UMaterialInstance* DuplicatedLaneMarksYellowMaterial = Cast<UMaterialInstance>(DuplicatedMaterialObject);

				StaticMeshComponent->SetMaterial(0, DuplicatedLaneMarksYellowMaterial);
			}
		}
		else
		{
			if (DefaultLaneMarksWhiteMaterial)
			{
				UObject* DuplicatedMaterialObject =
					UBlueprintUtilFunctions::CopyAssetToPlugin(DefaultLaneMarksWhiteMaterial, MapName);
				UMaterialInstance* DuplicatedLaneMarksWhiteMaterial = Cast<UMaterialInstance>(DuplicatedMaterialObject);

				StaticMeshComponent->SetMaterial(0, DuplicatedLaneMarksWhiteMaterial);
			}
		}

		const FProceduralCustomMesh MeshData = *Mesh;
		TArray<FVector> Normals;
		TArray<FProcMeshTangent> Tangents;
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
			MeshData.Vertices, MeshData.Triangles, MeshData.UV0, Normals, Tangents);

		UObject* DuplicatedMaterialObject = UBlueprintUtilFunctions::CopyAssetToPlugin(DefaultLandscapeMaterial, MapName);
		UMaterialInstance* DuplicatedLandscapeMaterial = Cast<UMaterialInstance>(DuplicatedMaterialObject);

		UStaticMesh* MeshToSet = UMapGenFunctionLibrary::CreateMesh(MeshData, Tangents, DuplicatedLandscapeMaterial, MapName,
			"LaneMark", FName(TEXT("SM_LaneMarkMesh" + FString::FromInt(meshindex) + GetStringForCurrentTile())));
		StaticMeshComponent->SetStaticMesh(MeshToSet);

		TempActor->SetActorLocation(MeshCentroid * 100);
		TempActor->Tags.Add(*FString(lanemarkinfo[index].c_str()));
		TempActor->Tags.Add(FName("RoadLane"));
#if ENGINE_MAJOR_VERSION > 4
		TempActor->SetIsSpatiallyLoaded(true);
#endif
		LaneMarkerActorList.Add(TempActor);
		index++;
		meshindex++;
		TempActor->SetActorEnableCollision(false);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (World)
	{
		FString CurrentMapName = World->GetMapName();
		CurrentMapName.RemoveFromStart(World->StreamingLevelsPrefix);
		UGameplayStatics::OpenLevel(World, FName(*CurrentMapName));
	}
}

void UOpenDriveToMap::GenerateTreePositions(
	const std::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation)
{
	carla::geom::Vector3D CarlaMinLocation(MinLocation.X / 100, MinLocation.Y / 100, MinLocation.Z / 100);
	carla::geom::Vector3D CarlaMaxLocation(MaxLocation.X / 100, MaxLocation.Y / 100, MaxLocation.Z / 100);

	std::vector<std::pair<carla::geom::Transform, std::string>> Locations =
		ParamCarlaMap->GetTreesTransform(CarlaMinLocation, CarlaMaxLocation, DistanceBetweenTrees, DistanceFromRoadEdge);
	int i = 0;
	for (auto& cl : Locations)
	{
		const FVector scale{1.0f, 1.0f, 1.0f};
		cl.first.location.z = GetHeight(cl.first.location.x, cl.first.location.y) / 100.0f;
		FTransform NewTransform(FRotator(cl.first.rotation), FVector(cl.first.location), scale);
		NewTransform = GetSnappedPosition(NewTransform);

		AActor* Spawner = GetEditorWorld()->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(), NewTransform.GetLocation(), NewTransform.Rotator());

		Spawner->Tags.Add(FName("TreeSpawnPosition"));
		Spawner->Tags.Add(FName(cl.second.c_str()));
		Spawner->SetActorLabel("TreeSpawnPosition" + FString::FromInt(i) + GetStringForCurrentTile());
#if ENGINE_MAJOR_VERSION > 4
		Spawner->SetIsSpatiallyLoaded(true);
#endif
		++i;
	}
}

void UOpenDriveToMap::GenerateTrafficLights(
	const std::optional<carla::road::Map>& ParamCarlaMap, FVector MinLocation, FVector MaxLocation)
{
	if (!ParamCarlaMap)
	{
		UE_LOG(LogCarlaTools, Warning, TEXT("GenerateTrafficLights: Map is not valid, skipping."));
		return;
	}
	const FString BPPath{TEXT("/CarlaDigitalTwinsTool/Blueprints/TrafficLight/BP_TrafficLightActor.BP_TrafficLightActor_C")};
	const carla::geom::Vector3D CarlaMinLocation(MinLocation.X / 100.0, MinLocation.Y / 100.0, MinLocation.Z / 100.0);
	const carla::geom::Vector3D CarlaMaxLocation(MaxLocation.X / 100.0, MaxLocation.Y / 100.0, MaxLocation.Z / 100.0);
	const double MinX{FMath::Min(CarlaMinLocation.x, CarlaMaxLocation.x)};
	const double MaxX{FMath::Max(CarlaMinLocation.x, CarlaMaxLocation.x)};
	const double MinY{FMath::Min(CarlaMinLocation.y, CarlaMaxLocation.y)};
	const double MaxY{FMath::Max(CarlaMinLocation.y, CarlaMaxLocation.y)};

	auto InChunk2D = [&](const carla::geom::Vector3D& Position) -> bool
	{ return (Position.x >= MinX && Position.x <= MaxX && Position.y >= MinY && Position.y <= MaxY); };

	auto MakeStableLabel = [&](const carla::road::Signal& S) -> FString
	{
		const FString Sid{UTF8_TO_TCHAR(S.GetSignalId().c_str())};
		const FString Key{FString::Printf(TEXT("%s|%s"), *MapName, *Sid)};
		const uint32 Hash{FCrc::StrCrc32(*Key)};
		return FString::Printf(TEXT("TrafficLight_%08X"), Hash);
	};

	UClass* TrafficLightBPClass{LoadObject<UClass>(nullptr, *BPPath)};
	if (!IsValid(TrafficLightBPClass))
	{
		UE_LOG(LogCarlaTools, Error, TEXT("Unabled to load BP in %s"), *BPPath);
		return;
	}

	std::vector<const carla::road::element::RoadInfoSignal*> Signals{ParamCarlaMap->GetAllSignalReferences()};

	TSet<const carla::road::Signal*> SeenSignals;

	for (const carla::road::element::RoadInfoSignal* Info : Signals)
	{
		if (!Info)
		{
			continue;
		}

		const carla::road::Signal* Signal = Info->GetSignal();
		if (!Signal)
		{
			continue;
		}

		if (!carla::road::SignalType::IsTrafficLight(Signal->GetType()))
		{
			continue;
		}

		if (SeenSignals.Contains(Signal))
		{
			continue;
		}

		const carla::road::RoadId SignalRoadId = Signal->GetRoadId();
		double SignalS = Signal->GetS();
		const double SignalT = Signal->GetT();
		const double OriginalS = SignalS;

		carla::geom::Transform SignalTransform = Signal->GetTransform();
		SignalTransform.rotation.yaw += 90.0f;

		if (carla::road::SignalType::IsTrafficLight(Signal->GetType())) {
			SignalTransform.location = SignalTransform.location +
				carla::geom::Location(SignalTransform.GetForwardVector() * 0.25f);
		}

		if (!InChunk2D(SignalTransform.location))
		{
			continue;
		}

		SeenSignals.Add(Signal);
		const FString Label{MakeStableLabel(*Signal)};
		ATrafficLightActor* TL{GetEditorWorld()->SpawnActorDeferred<ATrafficLightActor>(
			TrafficLightBPClass, SignalTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding)};
		if (!IsValid(TL))
		{
			continue;
		}
		const FString Rel{TEXT("Carla/Static/TrafficLight/TrafficLights2025/Presets/Default.json")};
		FString JsonAbs;
		if (!TryResolveContentFileAnywhere(Rel, JsonAbs))
		{
			UE_LOG(LogCarlaTools, Warning,
				TEXT("Default traffic light JSON not found under any Content: %s. Using backup."), *Rel);
			TL->PopulateDefault();
		}
		else
		{
			FString JSONString;
			if (FFileHelper::LoadFileToString(JSONString, *JsonAbs))
			{
				UE_LOG(LogCarlaTools, Log, TEXT("Loading JSON file at %s"), *JsonAbs);
				TL->BuildFromJSONString(JSONString);
			}
			else
			{
				UE_LOG(LogCarlaTools, Warning, TEXT("Failed to read JSON file at %s. Using backup."), *JsonAbs);
				TL->PopulateDefault();
			}
		}
		TL->FinishSpawning(SignalTransform);
		if (!IsValid(TL))
		{
			continue;
		}
		// Assign JunctionID and LaneIDs to traffic light poles and modules
		const carla::road::JuncId JunctionId = ParamCarlaMap->GetJunctionId(SignalRoadId);

        TL->JunctionID = JunctionId;

        // Assign SignalID from OpenDRIVE (unique identifier)
        TL->SignalID = FString(UTF8_TO_TCHAR(Signal->GetSignalId().c_str()));

        // Assign TrafficLightGroupID based on Signal Controllers
        const auto& Controllers = Signal->GetControllers();
        if (!Controllers.empty()) {
            const std::string& ControllerId = *Controllers.begin();
            TL->TrafficLightGroupID = FString(UTF8_TO_TCHAR(ControllerId.c_str()));
        }

		UE_LOG(LogCarlaTools, Log,
			TEXT("Processing Signal %s: RoadId=%d, JunctionId=%d, Position=(%f,%f,%f)"),
			UTF8_TO_TCHAR(Signal->GetSignalId().c_str()),
			SignalRoadId,
			JunctionId,
			SignalTransform.location.x,
			SignalTransform.location.y,
			SignalTransform.location.z);

		// Get lane validities for this signal
		const std::vector<carla::road::LaneValidity>& Validities = Info->GetValidities();
		const carla::road::SignalOrientation SignalOrientation = Signal->GetOrientation();

		UE_LOG(LogCarlaTools, Log,
			TEXT("Signal has %d lane validities, orientation: %d"),
			static_cast<int32>(Validities.size()),
			static_cast<int32>(SignalOrientation));

		// Log road information
		UE_LOG(LogCarlaTools, Log,
			TEXT("Processing road %d for signal analysis"), SignalRoadId);

		// Assign JunctionID and LaneIDs to all poles and modules
		for (FTLPole& Pole : TL->Poles)
		{

			UE_LOG(LogCarlaTools, Log,
				TEXT("Assigned JunctionID %d to pole"), JunctionId);

			// Process each head in the pole
			for (FTLHead& Head : Pole.Heads)
			{
				// Process each module in the head
				for (FTLModule& Module : Head.Modules)
				{
					// Clear existing lane IDs
					Module.LaneIds.Empty();

					// Add affected lane IDs based on signal validities and orientation
					for (const auto& Validity : Validities)
					{
						UE_LOG(LogCarlaTools, Log,
							TEXT("Processing validity: from_lane=%d, to_lane=%d"),
							Validity._from_lane, Validity._to_lane);

						// Add all lanes in the validity range
						for (carla::road::LaneId LaneId = Validity._from_lane; LaneId <= Validity._to_lane; ++LaneId)
						{
							if (LaneId != 0) // Skip center lane (lane 0)
							{
								// Filter lanes based on signal orientation
								bool bShouldAddLane = false;

								switch (SignalOrientation)
								{
									case carla::road::SignalOrientation::Positive:
										// Positive orientation affects negative lane IDs (lanes going in positive s direction)
										bShouldAddLane = (LaneId < 0);
										break;
									case carla::road::SignalOrientation::Negative:
										// Negative orientation affects positive lane IDs (lanes going in negative s direction)
										bShouldAddLane = (LaneId > 0);
										break;
									case carla::road::SignalOrientation::Both:
										// Both orientations affect all lanes
										bShouldAddLane = true;
										break;
								}

								if (bShouldAddLane)
								{
									Module.LaneIds.Add(LaneId);
									UE_LOG(LogCarlaTools, Log,
										TEXT("Added lane %d to module (orientation filter passed)"), LaneId);
								}
								else
								{
									UE_LOG(LogCarlaTools, Log,
										TEXT("Skipped lane %d (orientation filter failed)"), LaneId);
								}
							}
						}
					}

					UE_LOG(LogCarlaTools, Log,
						TEXT("Final: Assigned %d lane IDs to module"), Module.LaneIds.Num());
				}
			}
		}

		TL->Build();
		TL->Bake(MapName, Label);

		UE_LOG(LogCarlaTools, Verbose,
			TEXT("Baked traffic light: Signal %s -> Actor '%s'"),
			UTF8_TO_TCHAR(Signal->GetSignalId().c_str()),
			*Label);

		TL->Destroy();
	}

}

float UOpenDriveToMap::GetHeight(float PosX, float PosY, bool bDrivingLane)
{
	if (DefaultHeightmap && HeightmapPixels.Num() > 0)
	{
		int32 TextureSizeX = HeightmapCopy->GetWidth();
		int32 TextureSizeY = HeightmapCopy->GetHeight();

		// Normalize world coordinates to [0, 1]
		float NormalizedX = (PosX - WorldOriginPosition.X) / (WorldEndPosition.X - WorldOriginPosition.X);
		float NormalizedY = (PosY - WorldOriginPosition.Y) / (WorldEndPosition.Y - WorldOriginPosition.Y);

		NormalizedX = FMath::Clamp(NormalizedX, 0.0f, 1.0f);
		NormalizedY = FMath::Clamp(NormalizedY, 0.0f, 1.0f);

		// Convert to texture coordinates
		float TexX = NormalizedX * (TextureSizeX);
		float TexY = NormalizedY * (TextureSizeY);

		float SmoothedValue = UMapGenFunctionLibrary::BicubicSampleG16(HeightmapPixels, TextureSizeX, TextureSizeY, TexX, TexY);
		// Convert to world height
		float LandscapeHeight = SmoothedValue * (MaxHeight - MinHeight) + MinHeight;

		if (bDrivingLane)
		{
			return LandscapeHeight - carla::geom::deformation::GetBumpDeformation(PosX, PosY);
		}
		else
		{
			return LandscapeHeight - 5.0f;
		}
	}
	else
	{
		if (bDrivingLane)
		{
			return carla::geom::deformation::GetZPosInDeformation(PosX, PosY) +
				   (carla::geom::deformation::GetZPosInDeformation(PosX, PosY) * -0.3f) -
				   carla::geom::deformation::GetBumpDeformation(PosX, PosY);
		}
		else
		{
			return carla::geom::deformation::GetZPosInDeformation(PosX, PosY) +
				   (carla::geom::deformation::GetZPosInDeformation(PosX, PosY) * -0.3f);
		}
	}
}



FTransform UOpenDriveToMap::GetSnappedPosition(FTransform Origin)
{
	FTransform ToReturn = Origin;
	FVector Start = Origin.GetLocation() + FVector(0, 0, MaxHeight + 10000.0f);
	FVector End = Origin.GetLocation() - FVector(0, 0, MinHeight - 10000.0f);
	FHitResult HitResult;
	FCollisionQueryParams CollisionQuery;
	CollisionQuery.bTraceComplex = true;
	FCollisionResponseParams CollisionParams;

	if (GetEditorWorld()->LineTraceSingleByChannel(
			HitResult, Start, End, ECollisionChannel::ECC_WorldStatic, CollisionQuery, CollisionParams))
	{
		UE_LOG(LogCarlaTools, Error, TEXT("Hit Actor %s at location %s"), *HitResult.GetActor()->GetName(),
			*HitResult.Location.ToString());
		ToReturn.SetLocation(HitResult.Location);
	}
	else
	{
		DrawDebugLine(GetEditorWorld(), Start, End, FColor::Red, false, 50.0f, 0, 1.0f);
		UE_LOG(LogCarlaTools, Warning, TEXT("No Hit Actor at location %s"), *Start.ToString());
		ToReturn.SetLocation(FVector(Start.X, Start.Y, GetHeight(Start.X, Start.Y, false)));
	}
	return ToReturn;
}

float UOpenDriveToMap::GetHeightForLandscape(FVector Origin)
{
	FVector Start = Origin + FVector(0, 0, MaxHeight + 5000.0f);
	FVector End = Origin - FVector(0, 0, MinHeight - 5000.0f);
	FHitResult HitResult;
	FCollisionQueryParams CollisionQuery;
	CollisionQuery.bTraceComplex = true;
	CollisionQuery.AddIgnoredActors(Landscapes);
	FCollisionResponseParams CollisionParams;

	// if( GetEditorWorld()->LineTraceSingleByChannel(
	//   HitResult,
	//   Start,
	//   End,
	//   ECollisionChannel::ECC_WorldStatic,
	//   CollisionQuery,
	//   CollisionParams) )
	// {
	//   return (HitResult.Location.Z) -1.0f;
	// }

	// If no hit, return the height based on the origin coordinates
	return GetHeight(Origin.X, Origin.Y, false) - 2.0f;
}

float UOpenDriveToMap::DistanceToLaneBorder(
	const std::optional<carla::road::Map>& ParamCarlaMap, FVector& location, int32_t lane_type) const
{
	carla::geom::Location cl(location);
	// wp = GetClosestWaypoint(pos). if distance wp - pos == lane_width --> estas al borde de la carretera
	auto wp = ParamCarlaMap->GetClosestWaypointOnRoad(cl, lane_type);
	if (wp)
	{
		carla::geom::Transform ct = ParamCarlaMap->ComputeTransform(*wp);
		double LaneWidth = ParamCarlaMap->GetLaneWidth(*wp);
		return cl.Distance(ct.location) - LaneWidth;
	}
	return 100000.0f;
}

bool UOpenDriveToMap::IsInRoad(const std::optional<carla::road::Map>& ParamCarlaMap, FVector& location) const
{
	int32_t start = static_cast<int32_t>(carla::road::Lane::LaneType::Driving);
	int32_t end = static_cast<int32_t>(carla::road::Lane::LaneType::Sidewalk);
	for (int32_t i = start; i < end; ++i)
	{
		if (ParamCarlaMap->GetWaypoint(location, i))
		{
			return true;
		}
	}
	return false;
}

void UOpenDriveToMap::ImportXODR()
{
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString MyFileDestination =
		UGenerationPathsHelper::GetRawMapDirectoryPath(MapName) + MapName + "OpenDrive/" + MapName + ".xodr";

	if (FileManager.CopyFile(*MyFileDestination, *LocalFilePath, EPlatformFileRead::None, EPlatformFileWrite::None))
	{
		UE_LOG(LogCarlaTools, Verbose, TEXT("FilePaths: File Copied!"));
		FilePath = MyFileDestination;
		LoadMap();
	}
	else
	{
		UE_LOG(LogCarlaTools, Error, TEXT("FilePaths local xodr file not copied: File not Copied!"));
	}
}

void UOpenDriveToMap::ImportOSM()
{
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString MyFileDestination = UGenerationPathsHelper::GetRawMapDirectoryPath(MapName) + MapName + "OpenDrive/" + MapName + ".osm";

	if (FileManager.CopyFile(*MyFileDestination, *LocalFilePath, EPlatformFileRead::None, EPlatformFileWrite::None))
	{
		UE_LOG(LogCarlaTools, Verbose, TEXT("FilePaths: File Copied!"));
		ConvertOSMInOpenDrive();
	}
	else
	{
		UE_LOG(LogCarlaTools, Error, TEXT("FilePaths local osm file not copied: File not Copied!"));
	}
}

void UOpenDriveToMap::MoveActorsToSubLevels(TArray<AActor*> ActorsToMove)
{
	AActor* QueryActor = UGameplayStatics::GetActorOfClass(GetEditorWorld(), ALargeMapManager::StaticClass());

	if (QueryActor != nullptr)
	{
		ALargeMapManager* LmManager = Cast<ALargeMapManager>(QueryActor);
		if (LmManager)
		{
			GEditor->GetEditorSubsystem<ULevelEditorSubsystem>()->SaveCurrentLevel();
			UOpenDriveToMap::MoveActorsToSubLevelWithLargeMap(ActorsToMove, LmManager);
		}
	}
}

void UOpenDriveToMap::MoveActorsToSubLevelWithLargeMap(TArray<AActor*> Actors, ALargeMapManager* LargeMapManager)
{
	TMap<FCarlaMapTile*, TArray<AActor*>> ActorsToMove;
	for (AActor* Actor : Actors)
	{
		if (Actor == nullptr)
		{
			continue;
		}
		UHierarchicalInstancedStaticMeshComponent* Component = Cast<UHierarchicalInstancedStaticMeshComponent>(
			Actor->GetComponentByClass(UHierarchicalInstancedStaticMeshComponent::StaticClass()));
		FVector ActorLocation = Actor->GetActorLocation();
		if (Component)
		{
			ActorLocation = FVector(0);
			for (int32 i = 0; i < Component->GetInstanceCount(); ++i)
			{
				FTransform Transform;
				Component->GetInstanceTransform(i, Transform, true);
				ActorLocation = ActorLocation + Transform.GetTranslation();
			}
			ActorLocation = ActorLocation / Component->GetInstanceCount();
		}
		UE_LOG(LogCarlaTools, Log, TEXT("Actor at location %s"), *ActorLocation.ToString());
		FCarlaMapTile* Tile = LargeMapManager->GetCarlaMapTile(ActorLocation);
		if (!Tile)
		{
			UE_LOG(LogCarlaTools, Error, TEXT("Error: actor %s in location %s is outside the map"), *Actor->GetName(),
				*ActorLocation.ToString());
			continue;
		}

		if (Component)
		{
			UpdateInstancedMeshCoordinates(Component, Tile->Location);
		}
		else
		{
			UpdateGenericActorCoordinates(Actor, Tile->Location);
		}
		ActorsToMove.FindOrAdd(Tile).Add(Actor);
	}

	for (auto& Element : ActorsToMove)
	{
		FCarlaMapTile* Tile = Element.Key;
		TArray<AActor*> ActorList = Element.Value;
		if (!ActorList.Num())
		{
			continue;
		}

		UWorld* World = GetEditorWorld();
		ULevelStreamingDynamic* StreamingLevel = Tile->StreamingLevel;
		StreamingLevel->bShouldBlockOnLoad = true;
		StreamingLevel->SetShouldBeVisible(true);
		StreamingLevel->SetShouldBeLoaded(true);
		ULevelStreaming* Level =
			UEditorLevelUtils::AddLevelToWorld(World, *Tile->Name, ULevelStreamingDynamic::StaticClass(), FTransform());
		int MovedActors = UEditorLevelUtils::MoveActorsToLevel(ActorList, Level, false, false);
		// StreamingLevel->SetShouldBeLoaded(false);
		UE_LOG(LogCarlaTools, Log, TEXT("Moved %d actors"), MovedActors);
		FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
		UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
	}

	GEngine->PerformGarbageCollectionAndCleanupActors();
	FText TransResetText(FText::FromString("Clean up after Move actors to sublevels"));
	if (GEditor->Trans)
	{
#if ENGINE_MAJOR_VERSION < 5
		GEditor->Trans->Reset(TransResetText);
#else
		GEditor->Trans->Reset(TransResetText);
#endif
		GEditor->Cleanse(true, true, TransResetText);
	}
}

void UOpenDriveToMap::UpdateInstancedMeshCoordinates(UHierarchicalInstancedStaticMeshComponent* Component, FVector TileOrigin)
{
	TArray<FTransform> NewTransforms;
	for (int32 i = 0; i < Component->GetInstanceCount(); ++i)
	{
		FTransform Transform;
		Component->GetInstanceTransform(i, Transform, true);
		Transform.AddToTranslation(-TileOrigin);
		NewTransforms.Add(Transform);
		UE_LOG(LogCarlaTools, Log, TEXT("New instance location %s"), *Transform.GetTranslation().ToString());
	}
	Component->BatchUpdateInstancesTransforms(0, NewTransforms, true, true, true);
}

void UOpenDriveToMap::UpdateGenericActorCoordinates(AActor* Actor, FVector TileOrigin)
{
	FVector LocalLocation = Actor->GetActorLocation() - TileOrigin;
	Actor->SetActorLocation(LocalLocation);
	UE_LOG(LogCarlaTools, Log, TEXT("%s New location %s"), *Actor->GetName(), *LocalLocation.ToString());
}

void UOpenDriveToMap::UnloadWorldPartitionRegion(const FBox& RegionBox)
{
	UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		UWorldPartitionSubsystem* WorldPartitionSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>();
		if (WorldPartitionSubsystem)
		{
			// Call UnloadRegion with a bounding box
			// WorldPartitionSubsystem->UnloadRegion(World, RegionBox);
		}
	}
}

void UOpenDriveToMap::RenderRoadToTexture(FVector MinLocation, FVector MaxLocation)
{
	UE_LOG(LogCarlaTools, Log, TEXT("Render road for curbs generation"));

	UWorld* World = GetEditorWorld();

	FBox Bounds(EForceInit::ForceInitToZero);
	Bounds += MinLocation;
	Bounds += MaxLocation;

	FString RoadLabel = "DrivingLane";
#if PLATFORM_LINUX
	if (bUseMitsuba)
	{
		RoadLabel = "UpdatedRoad";
	}
#endif

	TArray<AActor*> HiddenActors;
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
		HiddenActors.Reserve(Actors.Num());
		for (auto& Actor : Actors)
		{
			auto Name = Actor->GetActorLabel();

			if (!Name.Contains(RoadLabel, ESearchCase::CaseSensitive))
			{
				Actor->SetActorHiddenInGame(true);
				HiddenActors.Add(Actor);
				continue;
			}
		}
		HiddenActors.Shrink();
	}

	auto Center = Bounds.GetCenter();
	auto Extent = FVector2D(Bounds.Max) - FVector2D(Bounds.Min);
	auto RenderTargetScale = UE_CM_TO_M * 8;
	auto RenderTargetExtent = Extent * RenderTargetScale;
	auto RenderTargetSize = FIntPoint((int32) std::round(RenderTargetExtent.X), (int32) std::round(RenderTargetExtent.Y));

	UE_LOG(LogCarlaTools, Log, TEXT("RenderTargetSize: %i, %i"), RenderTargetSize.X, RenderTargetSize.Y);

	auto RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->AddToRoot();
	RenderTarget->ClearColor = FLinearColor::Black;
	RenderTarget->OverrideFormat = PF_FloatRGB;
	RenderTarget->bForceLinearGamma = true;
	RenderTarget->SizeX = RenderTargetSize.X;
	RenderTarget->SizeY = RenderTargetSize.Y;
	RenderTarget->UpdateResource();

	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActorSpawnParameters.Name = FName(*(TEXT("Camera") + GetStringForCurrentTile()));

	auto Camera = World->SpawnActor<ASceneCapture2D>(ASceneCapture2D::StaticClass(), ActorSpawnParameters);

	auto CaptureComponent = Camera->GetCaptureComponent2D();
	CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	CaptureComponent->OrthoWidth = Extent.X;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_BaseColor;
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;
	CaptureComponent->TextureTarget = RenderTarget;

	CaptureComponent->ShowFlags.SetLumenGlobalIllumination(false);
	CaptureComponent->ShowFlags.SetLumenReflections(false);
	CaptureComponent->ShowFlags.SetGlobalIllumination(false);
	CaptureComponent->ShowFlags.SetScreenSpaceReflections(false);
	CaptureComponent->ShowFlags.SetDistanceFieldAO(false);
	CaptureComponent->ShowFlags.SetTemporalAA(false);
	CaptureComponent->ShowFlags.SetMotionBlur(false);
	CaptureComponent->ShowFlags.SetBloom(false);
	CaptureComponent->ShowFlags.SetVolumetricFog(false);
	CaptureComponent->ShowFlags.SetDynamicShadows(false);

	auto CameraZ = std::max(Bounds.Max.X, std::max(Bounds.Max.Y, Bounds.Max.Z));
	auto Location = FVector(Center.X, Center.Y, CameraZ);

	auto Rotation = FRotationMatrix::MakeFromXZ((Center - Location).GetSafeNormal(), FVector::YAxisVector).ToQuat();

	Camera->SetActorLocation(Location);
	Camera->SetActorRotation(Rotation);

	CaptureComponent->CaptureScene();

	TArray<FColor> Pixels;
	RenderTarget->GameThread_GetRenderTargetResource()->ReadPixels(Pixels);

	auto ImageTask = MakeUnique<FImageWriteTask>();
	auto PixelData = MakeUnique<TImagePixelData<FColor>>(RenderTargetSize);
	PixelData->Pixels = Pixels;
	ImageTask->PixelData = MoveTemp(PixelData);

	FString OutPath = UGenerationPathsHelper::GetPythonIntermediatePath(MapName);
	FString ImagePath = OutPath / TEXT("road_render") + GetStringForCurrentTile() + TEXT(".png");

	ImageTask->Filename = ImagePath;
	ImageTask->Format = EImageFormat::PNG;
	ImageTask->CompressionQuality = (int32) EImageCompressionQuality::Default;
	ImageTask->bOverwriteFile = true;
	ImageTask->PixelPreProcessors.Add(TAsyncAlphaWrite<FColor>(255));

	auto& HighResScreenshotConfig = GetHighResScreenshotConfig();
	auto Task = HighResScreenshotConfig.ImageWriteQueue->Enqueue(MoveTemp(ImageTask));

	for (auto& HiddenActor : HiddenActors)
		HiddenActor->SetActorHiddenInGame(false);

	RenderTarget->RemoveFromRoot();
	Camera->Destroy();

	Task.Wait();
}

void UOpenDriveToMap::GenerateCurbSplines()
{
	UE_LOG(LogCarlaTools, Log, TEXT("Create splines for curbs generation"));

	UWorld* World = GetEditorWorld();

	FString OutPath = UGenerationPathsHelper::GetPythonIntermediatePath(MapName);

	RunPythonMergeTiles();

	RunPythonRoadEdges();

	auto JsonPath = OutPath / TEXT("contours.json");

	MinPosition = FVector(0.0f, 0.0f, 0.0f);
	MaxPosition = FVector(NumTilesInXY.X * TileSize, NumTilesInXY.Y * -TileSize, 0.0f);

	FBox Bounds(EForceInit::ForceInitToZero);
	Bounds += MinPosition;
	Bounds += MaxPosition;

	auto Center = Bounds.GetCenter();
	auto Extent = FVector2D(Bounds.Max) - FVector2D(Bounds.Min);
	auto RenderTargetScale = UE_CM_TO_M * 8;
	auto RenderTargetExtent = Extent * RenderTargetScale;
	auto RenderTargetSize = FIntPoint((int32) std::round(RenderTargetExtent.X), (int32) std::round(RenderTargetExtent.Y));

	auto RoadSplines =
		UGeometryImporter::CreateSplinesFromJson(World, JsonPath, FVector2D(Center.X, Center.Y), Extent, RenderTargetSize);

	UE_LOG(LogCarlaTools, Log, TEXT("Number of road splines: %i"), RoadSplines.Num());

	// Project spline points to the floor
	for (auto Spline : RoadSplines)
	{
		int32 NumPoints = Spline->GetNumberOfSplinePoints();
		for (int32 i = 0; i < NumPoints; ++i)
		{
			FVector Pos = Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);

			Pos.Z = GetHeight(Pos.X, Pos.Y, false);

			Spline->SetLocationAtSplinePoint(i, Pos, ESplineCoordinateSpace::Local, false);

			Spline->UpdateSpline();

			UE_LOG(LogCarlaTools, Log, TEXT("Spline updated"));
		}
	}

	GeneratedSplines.Append(RoadSplines);
}

void UOpenDriveToMap::RunPythonScript(FString ScriptPath, FString Args)
{
	FString PythonExe = PythonBinPath;

	void* ReadPipe = nullptr;
	void* WritePipe = nullptr;
	FPlatformProcess::CreatePipe(ReadPipe, WritePipe);

	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*PythonExe, *Args,
		true,	  // bLaunchDetached
		false,	  // bLaunchHidden
		false,	  // bLaunchReallyHidden
		nullptr, 0, nullptr,
		WritePipe,	  // Pipe for stdout/stderr
		WritePipe);

	if (!ProcHandle.IsValid())
	{
		UE_LOG(LogCarlaTools, Error, TEXT("Failed to launch Python script."));
		FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
		return;
	}

	FString Output;
	while (FPlatformProcess::IsProcRunning(ProcHandle))
	{
		FString NewOutput = FPlatformProcess::ReadPipe(ReadPipe);
		Output += NewOutput;
		FPlatformProcess::Sleep(0.01f);
	}

	Output += FPlatformProcess::ReadPipe(ReadPipe);

	FPlatformProcess::CloseProc(ProcHandle);
	FPlatformProcess::ClosePipe(ReadPipe, WritePipe);

	UE_LOG(LogCarlaTools, Display, TEXT("Python Output:\n%s"), *Output);
}

void UOpenDriveToMap::RunPythonMergeTiles()
{
	UE_LOG(LogCarlaTools, Log, TEXT("Running Python merge rendered tiles script..."));

	FString PluginPath = UGenerationPathsHelper::GetDigitalTwinsPluginPath();
	FString ScriptPath = PluginPath / TEXT("Content/Python/merge_images.py");
	FString OutPath = UGenerationPathsHelper::GetPythonIntermediatePath(MapName);

	FString Args;
	Args += FString::Printf(TEXT("\"%s\" "), *ScriptPath);
	Args += FString::Printf(TEXT("--folder_path=\"%s\" "), *OutPath);

	RunPythonScript(ScriptPath, Args);
}

void UOpenDriveToMap::RunPythonRoadEdges()
{
	UE_LOG(LogCarlaTools, Log, TEXT("Running Python road edges extraction script..."));

	FString PluginPath = UGenerationPathsHelper::GetDigitalTwinsPluginPath();
	FString ScriptPath = PluginPath / TEXT("Content/Python/road_edge_detection.py");
	FString OutPath = UGenerationPathsHelper::GetPythonIntermediatePath(MapName);

	FString Args;
	Args += FString::Printf(TEXT("\"%s\" "), *ScriptPath);
	Args += FString::Printf(TEXT("--folder_path=\"%s\" "), *OutPath);

	RunPythonScript(ScriptPath, Args);
}

TArray<FRoadSignInfo> UOpenDriveToMap::GetAllRoadSignsInfo()
{
	TArray<FRoadSignInfo> RoadSigns;
	if (!CarlaMap.has_value())
	{
		return RoadSigns;
	}

	const carla::road::Map& Map = CarlaMap.value();
	auto Signals = Map.GetAllSignalReferences();

	for (int32 i = 0; i < Signals.size(); ++i)
	{
		const auto& SignalRef = Signals[i];
		if (!SignalRef)
		{
			continue;
		}

		const auto* Signal = SignalRef->GetSignal();
		if (!Signal)
		{
			continue;
		}

		FRoadSignInfo Info;
		Info.SignId = FString(SignalRef->GetSignalId().c_str());
		Info.RoadId = FString::FromInt(SignalRef->GetRoadId());
		Info.S = SignalRef->GetS();
		Info.T = SignalRef->GetT();

		auto signalOrientation = SignalRef->GetOrientation();
		switch (signalOrientation)
		{
			case carla::road::SignalOrientation::Positive:
				Info.Orientation = TEXT("Positive");
				break;

			case carla::road::SignalOrientation::Negative:
				Info.Orientation = TEXT("Negative");
				break;

			case carla::road::SignalOrientation::Both:
				Info.Orientation = TEXT("Both");
				break;

			default:
				Info.Orientation = TEXT("Unknown");
				break;
		}

		const auto SignalTransform = Signal->GetTransform();
		Info.Transform = SignalTransform;

		RoadSigns.Add(Info);
	}

	return RoadSigns;
}

void UOpenDriveToMap::ExportStaticMeshToOBJ(UStaticMesh* StaticMesh, const FString& OutputPath)
{
	UE_LOG(LogTemp, Log, TEXT("Exporting mesh as OBJ to path: %s"), *OutputPath);

	if (!StaticMesh || !StaticMesh->GetRenderData())
	{
		UE_LOG(LogTemp, Warning, TEXT("StaticMesh is null or has no render data."));
		return;
	}

	FString ObjData;
	ObjData += FString::Printf(TEXT("Exported OBJ from %s\n"), *StaticMesh->GetName());

	const FStaticMeshLODResources& LOD = StaticMesh->GetRenderData()->LODResources[0];

	// Vertices
	const FPositionVertexBuffer& PositionVertexBuffer = LOD.VertexBuffers.PositionVertexBuffer;
	const FStaticMeshVertexBuffer& VertexBuffer = LOD.VertexBuffers.StaticMeshVertexBuffer;
	const int32 VertexCount = PositionVertexBuffer.GetNumVertices();

	for (int32 i = 0; i < VertexCount; ++i)
	{
		FVector Pos = (FVector) PositionVertexBuffer.VertexPosition(i);
		Pos.Z = GetHeight(Pos.X, Pos.Y) / 100.0f;
		ObjData += FString::Printf(TEXT("v %f %f %f\n"), Pos.X, Pos.Z, Pos.Y);
	}

	// Normals
	for (int32 i = 0; i < VertexCount; ++i)
	{
		FVector Normal = (FVector) VertexBuffer.VertexTangentZ(i);
		ObjData += FString::Printf(TEXT("vn %f %f %f\n"), Normal.X, Normal.Z, Normal.Y);
	}

	// UVs
	for (int32 i = 0; i < VertexCount; ++i)
	{
		FVector2D UV = (FVector2D) VertexBuffer.GetVertexUV(i, 0);
		ObjData += FString::Printf(TEXT("vt %f %f\n"), UV.X, 1.0f - UV.Y);	  // Flip V
	}

	// Faces (triangles)
	const FIndexArrayView Indices = LOD.IndexBuffer.GetArrayView();
	const int32 NumTriangles = Indices.Num() / 3;

	for (int32 i = 0; i < NumTriangles; ++i)
	{
		int32 i0 = Indices[i * 3 + 0] + 1;
		int32 i1 = Indices[i * 3 + 1] + 1;
		int32 i2 = Indices[i * 3 + 2] + 1;
		ObjData += FString::Printf(TEXT("f %d/%d/%d %d/%d/%d %d/%d/%d\n"), i0, i0, i0, i1, i1, i1, i2, i2, i2);
	}

	// Save to file
	if (FFileHelper::SaveStringToFile(ObjData, *OutputPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully exported to OBJ: %s"), *OutputPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save OBJ file."));
	}
}

void UOpenDriveToMap::MergeRoads()
{
	UE_LOG(LogTemp, Log, TEXT("Merging roads to single mesh..."));

	UWorld* World = GetEditorWorld();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), FoundActors);

	TArray<UPrimitiveComponent*> ComponentsToMerge;

	for (AActor* Actor : FoundActors)
	{
		if (Actor->GetActorLabel().StartsWith("SM_DrivingLane_"))
		{
			if (UStaticMeshComponent* SMC =
					Cast<UStaticMeshComponent>(Actor->GetComponentByClass(UStaticMeshComponent::StaticClass())))
			{
				ComponentsToMerge.Add(SMC);
			}
		}
	}

	if (ComponentsToMerge.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No components to merge"));
		return;
	}

	FMeshMergingSettings MergeSettings;
	MergeSettings.bMergeMaterials = true;
	MergeSettings.PivotType = EMeshMergePivotType::WorldOrigin; // UE5.8: replaces bPivotPointAtZero

	FString OutPath = UGenerationPathsHelper::GetPythonIntermediatePath(MapName);
	FString AssetName = TEXT("MergedRoad");
	FString PackageName = OutPath / AssetName;
	UPackage* Package = CreatePackage(*PackageName);

	IMeshMergeUtilities& MeshMergeUtilities =
		FModuleManager::LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities").GetUtilities();

	TArray<UObject*> AssetsToSync;
	FVector MergedActorLocation = FVector::ZeroVector;

	MeshMergeUtilities.MergeComponentsToStaticMesh(ComponentsToMerge, World, MergeSettings,
		nullptr,	// Base material (optional)
		Package,	// Outer
		AssetName, AssetsToSync, MergedActorLocation,
		1.0f,	 // Screen size
		false	 // bSilent
	);

	UE_LOG(LogTemp, Log, TEXT("Merged mesh saved to: %s"), *PackageName);

	UStaticMesh* MergedMesh = nullptr;

	for (UObject* Asset : AssetsToSync)
	{
		MergedMesh = Cast<UStaticMesh>(Asset);
		if (MergedMesh)
		{
			FString ObjPath = OutPath / FString::Printf(TEXT("MergedRoad_%.4f_%.4f.obj"), WorldEndPosition.X, WorldEndPosition.Y);
			ExportStaticMeshToOBJ(MergedMesh, ObjPath);
			break;
		}
	}
}

void UOpenDriveToMap::RunPythonRoadSegmentation()
{
	FString PluginPath = UGenerationPathsHelper::GetDigitalTwinsPluginPath();
	FString ScriptPath = PluginPath / TEXT("Content/Python/road_segmentation.py");
	FString OutPath = UGenerationPathsHelper::GetPythonIntermediatePath(MapName);

	FString Args;
	Args += FString::Printf(TEXT("\"%s\" "), *ScriptPath);
	Args += FString::Printf(TEXT("--lon_min=%.8f "), OriginGeoCoordinates.Y);
	Args += FString::Printf(TEXT("--lat_min=%.8f "), OriginGeoCoordinates.X);
	Args += FString::Printf(TEXT("--lon_max=%.8f "), FinalGeoCoordinates.Y);
	Args += FString::Printf(TEXT("--lat_max=%.8f "), FinalGeoCoordinates.X);
	Args += FString::Printf(TEXT("--output_path=\"%s\" "), *OutPath);

	RunPythonScript(ScriptPath, Args);
}

void UOpenDriveToMap::RunPythonMitsubaOptimization()
{
	FString PluginPath = UGenerationPathsHelper::GetDigitalTwinsPluginPath();
	FString ScriptPath = PluginPath / TEXT("Content/Python/mitsuba_road_refiner.py");
	FString OutPath = UGenerationPathsHelper::GetPythonIntermediatePath(MapName);

	FString Args;
	Args += FString::Printf(TEXT("\"%s\" "), *ScriptPath);
	Args += FString::Printf(TEXT("--folder_path=\"%s\" "), *OutPath);

	RunPythonScript(ScriptPath, Args);
}

void UOpenDriveToMap::MitsubaMeshOptimization()
{
	UE_LOG(LogCarlaTools, Log, TEXT("Running road mesh optimization with Mitsuba..."));

	MergeRoads();

	RunPythonRoadSegmentation();

	RunPythonMitsubaOptimization();

	FString OutPath = UGenerationPathsHelper::GetPythonIntermediatePath(MapName);
	FString ObjPath = OutPath / TEXT("updated_road.obj");
	UGeometryImporter::ImportObj(ObjPath, GEditor->GetEditorWorldContext().World(), DefaultRoadMaterial);

	// Hide DrivingLane actors
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetEditorWorld(), AActor::StaticClass(), Actors);
	for (auto& Actor : Actors)
	{
		auto Name = Actor->GetActorLabel();

		if (Name.Contains("DrivingLane", ESearchCase::CaseSensitive))
		{
			Actor->SetActorHiddenInGame(true);
			Actor->SetIsTemporarilyHiddenInEditor(true);
		}
	}
}
bool UOpenDriveToMap::TryResolveContentFileAnywhere(const FString& RelativePathUnderContent, FString& OutAbsPath)
{
	if (RelativePathUnderContent.IsEmpty())
	{
		return false;
	}

	IPluginManager& PM{IPluginManager::Get()};
	const TArray<TSharedRef<IPlugin>> Plugins{PM.GetEnabledPlugins()};
	for (const TSharedRef<IPlugin>& Plugin : Plugins)
	{
		if (!Plugin->CanContainContent())
		{
			continue;
		}

		const FString Candidate{FPaths::Combine(Plugin->GetContentDir(), RelativePathUnderContent)};
		if (FPaths::FileExists(Candidate))
		{
			OutAbsPath = Candidate;
			return true;
		}
	}

	const FString ProjectCandidate{FPaths::Combine(FPaths::ProjectContentDir(), RelativePathUnderContent)};
	if (FPaths::FileExists(ProjectCandidate))
	{
		OutAbsPath = ProjectCandidate;
		return true;
	}

	return false;
}
#endif

FVector UOpenDriveToMap::DisplaceLocationOutsideNeighboringRoads(const UObject* WorldContextObject, FVector InLocation, float SteppingPercentage, float RoadLimitPadding)
{
	FString FileContent;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	FString LevelName = FPackageName::GetShortName(World->GetMapName());
	FString file_path = UGenerationPathsHelper::GetRawMapDirectoryPath(LevelName) + "OpenDrive/" + LevelName + ".xodr";
	FFileHelper::LoadFileToString(FileContent, *file_path);
	std::string opendrive_xml = carla::rpc::FromLongFString(FileContent);

	std::optional<carla::road::Map> current_carla_map;
	current_carla_map = carla::opendrive::OpenDriveParser::Load(opendrive_xml);

	int32 check_shoulder_or_driving =
		static_cast<int32_t>(carla::road::Lane::LaneType::Shoulder) |
		static_cast<int32_t>(carla::road::Lane::LaneType::Driving);

	std::optional<carla::road::element::Waypoint> closest_waypoint =
		current_carla_map->GetClosestWaypointOnRoad(InLocation, check_shoulder_or_driving);

	FVector out_location = InLocation;

	if (closest_waypoint)
	{
		carla::geom::Transform road_transform = current_carla_map->ComputeTransform(closest_waypoint.value());

		float distance_to_road = FVector(road_transform.location.ToFVector() * 100.0f - out_location).Length();
		float lane_width = current_carla_map->GetLaneWidth(closest_waypoint.value());

		float displacement_direction = 1.0f;
		int max_displacement_iterations = 10;

		for (int counter = 0; counter < max_displacement_iterations; counter++)
		{
			if (displacement_direction == 0.0f) break;
			if (distance_to_road > (lane_width * 100.0f + RoadLimitPadding)) break;

			std::optional<carla::road::element::Waypoint> right_waypoint = current_carla_map->GetRight(closest_waypoint.value());
			carla::road::Lane::LaneType right_lane_type = (right_waypoint) ?
				current_carla_map->GetLaneType(right_waypoint.value()) :
				carla::road::Lane::LaneType::None;

			std::optional<carla::road::element::Waypoint> left_waypoint = current_carla_map->GetLeft(closest_waypoint.value());
			carla::road::Lane::LaneType left_lane_type = (left_waypoint) ?
				current_carla_map->GetLaneType(left_waypoint.value()) :
				carla::road::Lane::LaneType::None;

			if (right_lane_type != carla::road::Lane::LaneType::Driving)
			{
				displacement_direction = 1.0f;
			}
			else if (left_lane_type != carla::road::Lane::LaneType::Driving)
			{
				displacement_direction = -1.0f;
			}
			else {
				displacement_direction = 0.0f;
			}

			FVector displacement_diff = road_transform.GetRightVector().ToFVector() * static_cast<float>(abs(lane_width)) * 100.0f * SteppingPercentage;
			out_location += displacement_diff * displacement_direction;

			closest_waypoint = current_carla_map->GetClosestWaypointOnRoad(out_location, check_shoulder_or_driving);
			road_transform = current_carla_map->ComputeTransform(closest_waypoint.value());
			distance_to_road = FVector(road_transform.location.ToFVector() * 100.0f - out_location).Length();
			lane_width = current_carla_map->GetLaneWidth(closest_waypoint.value());
		}

	}

	return out_location;
}
