// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "CookAssetsCommandlet.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformFile.h"

#include "UObject/MetaData.h"

// #include "CommandletPluginPrivate.h"

UCookAssetsCommandlet::UCookAssetsCommandlet()
{
  IsClient = false;
  IsEditor = true;
  IsServer = false;
  LogToConsole = true;

#if WITH_EDITORONLY_DATA
  static ConstructorHelpers::FObjectFinder<UMaterial> MarkingNode(TEXT(
      "Material'/Game/Carla/Static/GenericMaterials/LaneMarking/M_MarkingLane_W.M_MarkingLane_W'"));
  static ConstructorHelpers::FObjectFinder<UMaterial> RoadNode(TEXT(
      "Material'/Game/Carla/Static/GenericMaterials/Masters/LowComplexity/M_Road1.M_Road1'"));
  static ConstructorHelpers::FObjectFinder<UMaterial> RoadNodeAux(TEXT(
      "Material'/Game/Carla/Static/GenericMaterials/LaneMarking/M_MarkingLane_Y.M_MarkingLane_Y'"));
  static ConstructorHelpers::FObjectFinder<UMaterial> TerrainNodeMaterial(TEXT(
      "Material'/Game/Carla/Static/GenericMaterials/Grass/M_Grass01.M_Grass01'"));

  MarkingNodeMaterial = (UMaterial *) MarkingNode.Object;
  RoadNodeMaterial = (UMaterial *) RoadNode.Object;
  MarkingNodeMaterialAux = (UMaterial *) RoadNodeAux.Object;
#endif
}
#if WITH_EDITORONLY_DATA

FPackageParams UCookAssetsCommandlet::ParseParams(const FString &InParams) const
{
  TArray<FString> Tokens;
  TArray<FString> Params;
  TMap<FString, FString> ParamVals;

  ParseCommandLine(*InParams, Tokens, Params);

  FPackageParams PackageParams;
  FParse::Value(*InParams, TEXT("PackageName="), PackageParams.Name);
  PackageParams.bUseCarlaMapMaterials = Params.Contains(TEXT("use-carla-map-materials"));

  return PackageParams;
}

void UCookAssetsCommandlet::LoadWorld(FAssetData &AssetData)
{
  FString BaseMap = TEXT("/Game/Carla/Maps/BaseMap");

  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*BaseMap);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    AssetData = AssetDatas.Pop();
  }
}

TArray<AStaticMeshActor *> UCookAssetsCommandlet::AddMeshesToWorld(
    const TArray<FString> &AssetsPaths,
    bool bUseCarlaMaterials)
{
  TArray<AStaticMeshActor *> SpawnedMeshes;

  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);

  // Remove the meshes names from the original path, so LoadAssetDataFromPaths
  // can be used
  TArray<FString> AssetsPathsDirectories;
  for (auto AssetPath : AssetsPaths)
  {
    FString Dir;
    AssetPath.Split(
        TEXT("/"),
        &Dir,
        nullptr,
        ESearchCase::Type::IgnoreCase,
        ESearchDir::Type::FromEnd);
    AssetsPathsDirectories.Add(Dir);
  }

  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPaths(AssetsPathsDirectories);
  AssetsObjectLibrary->LoadAssetsFromAssetData();

  const FTransform zeroTransform = FTransform();
  FVector initialVector = FVector(0, 0, 0);
  FRotator initialRotator = FRotator(0, 180, 0);
  FActorSpawnParameters SpawnInfo;

  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);
  UStaticMesh *MeshAsset;
  AStaticMeshActor *MeshActor;

  for (auto MapAsset : MapContents)
  {
    MeshAsset = CastChecked<UStaticMesh>(MapAsset.GetAsset());
    MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
        initialVector,
        initialRotator);
    MeshActor->GetStaticMeshComponent()->SetStaticMesh(CastChecked<UStaticMesh>(MeshAsset));
    SpawnedMeshes.Add(MeshActor);
    if (bUseCarlaMaterials)
    {
      FString AssetName;
      MapAsset.AssetName.ToString(AssetName);
      if (AssetName.Contains("MarkingNode"))
      {
        MeshActor->GetStaticMeshComponent()->SetMaterial(0, MarkingNodeMaterial);
        MeshActor->GetStaticMeshComponent()->SetMaterial(1, MarkingNodeMaterialAux);
      }
      else if (AssetName.Contains("RoadNode"))
      {
        MeshActor->GetStaticMeshComponent()->SetMaterial(0, RoadNodeMaterial);
      }
      else if (AssetName.Contains("Terrain"))
      {
        MeshActor->GetStaticMeshComponent()->SetMaterial(0, TerrainNodeMaterial);
      }
    }
  }

  World->MarkPackageDirty();
  return SpawnedMeshes;
}

void UCookAssetsCommandlet::DestroyWorldSpawnedActors(TArray<AStaticMeshActor *> &SpawnedActors)
{
  for (auto Actor : SpawnedActors)
  {
    Actor->Destroy();
  }

  World->MarkPackageDirty();
}

bool UCookAssetsCommandlet::SaveWorld(
    FAssetData &AssetData,
    FString &PackageName,
    FString &DestPath,
    FString &WorldName)
{
  UPackage *Package = AssetData.GetPackage();
  Package->SetFolderName(*DestPath);
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(World);

  // Renaming stuff for the map
  World->Rename(*WorldName, World->GetOuter());
  FString PackagePath = DestPath + "/" + WorldName;
  FAssetRegistryModule::AssetRenamed(World, *PackagePath);
  World->MarkPackageDirty();
  World->GetOuter()->MarkPackageDirty();

  // Check if OpenDrive file exists
  FString PathXODR = FPaths::ProjectContentDir() + PackageName + TEXT("/Maps/") + WorldName + TEXT(
      "/OpenDrive/") + WorldName + TEXT(".xodr");

  bool bPackageSaved = false;
  if (FPaths::FileExists(PathXODR))
  {
    // Filling the map stuff (Code only applied for maps)
    AOpenDriveActor *OpenWorldActor =
        CastChecked<AOpenDriveActor>(World->SpawnActor(AOpenDriveActor::StaticClass(),
        new FVector(), NULL));

    OpenWorldActor->BuildRoutes(WorldName);
    OpenWorldActor->AddSpawners();

    SavePackage(PackagePath, Package);

    OpenWorldActor->RemoveRoutes();
    OpenWorldActor->RemoveSpawners();
    OpenWorldActor->Destroy();
  }
  else
  {
    SavePackage(PackagePath, Package);
  }
  return bPackageSaved;
}

FAssetsPaths UCookAssetsCommandlet::GetAssetsPathFromPackage(const FString &PackageName) const
{

  FString PackageConfigPath = FPaths::ProjectContentDir() + PackageName + TEXT("/Config/");
  FString PackageJsonFilePath = PackageConfigPath + PackageName + TEXT(".Package.json");

  FAssetsPaths AssetsPaths;

  // Get All Maps Path
  FString MapsFileJsonContent;
  if (FFileHelper::LoadFileToString(MapsFileJsonContent, *PackageJsonFilePath))
  {
    TSharedPtr<FJsonObject> JsonParsed;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(MapsFileJsonContent);
    if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
    {
      // Add Maps Path
      auto MapsJsonArray = JsonParsed->GetArrayField(TEXT("maps"));

      for (auto &MapJsonValue : MapsJsonArray)
      {
        TSharedPtr<FJsonObject> MapJsonObject = MapJsonValue->AsObject();

        FMapData MapData;
        MapData.Name = MapJsonObject->GetStringField(TEXT("name"));
        MapData.Path = MapJsonObject->GetStringField(TEXT("path"));
        MapData.bUseCarlaMapMaterials = MapJsonObject->GetBoolField(TEXT("use_carla_materials"));

        AssetsPaths.MapsPaths.Add(std::move(MapData));
      }

      // Add Props Path
      auto PropJsonArray = JsonParsed->GetArrayField(TEXT("props"));

      for (auto &PropJsonValue : PropJsonArray)
      {
        TSharedPtr<FJsonObject> PropJsonObject = PropJsonValue->AsObject();

        FString PropAssetPath = PropJsonObject->GetStringField(TEXT("path"));

        AssetsPaths.PropsPaths.Add(PropAssetPath);
      }
    }
  }
  return AssetsPaths;
}

bool SaveStringTextToFile(
    FString SaveDirectory,
    FString FileName,
    FString SaveText,
    bool bAllowOverWriting)
{
  IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

  // CreateDirectoryTree returns true if the destination
  // directory existed prior to call or has been created
  // during the call.
  if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
  {
    // Get absolute file path
    FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

    // Allow overwriting or file doesn't already exist
    if (bAllowOverWriting || !PlatformFile.FileExists(*AbsoluteFilePath))
    {
      FFileHelper::SaveStringToFile(SaveText, *AbsoluteFilePath);
    }
  }
  return true;
}

bool UCookAssetsCommandlet::SavePackage(const FString &PackagePath, UPackage *Package) const
{
  FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath,
      FPackageName::GetMapPackageExtension());

  return UPackage::SavePackage(Package, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
}

int32 UCookAssetsCommandlet::Main(const FString &Params)
{
  FPackageParams PackageParams = ParseParams(Params);

  // Get Props and Maps Path
  FAssetsPaths AssetsPaths = GetAssetsPathFromPackage(PackageParams.Name);

  // Load World
  FAssetData AssetData;
  LoadWorld(AssetData);
  World = CastChecked<UWorld>(AssetData.GetAsset());

  FString MapPathData;
  for (auto Map : AssetsPaths.MapsPaths)
  {
    FString RoadsPath = TEXT("/Game/") + PackageParams.Name + TEXT("/Static/RoadNode/") + Map.Name;
    FString MarkingLinePath = TEXT("/Game/") + PackageParams.Name + TEXT("/Static/MarkingNode/") + Map.Name;
    FString TerrainPath = TEXT("/Game/") + PackageParams.Name + TEXT("/Static/TerrainNode/") + Map.Name;

    TArray<FString> DataPath = {RoadsPath, MarkingLinePath, TerrainPath};

    // Add Map Meshes to World
    TArray<AStaticMeshActor *> SpawnedActors = AddMeshesToWorld(DataPath, Map.bUseCarlaMapMaterials);

    // Save the World in specified path
    SaveWorld(AssetData, PackageParams.Name, Map.Path, Map.Name);

    // Remove spawned actors from world to keep equal as BaseMap
    DestroyWorldSpawnedActors(SpawnedActors);

    MapPathData.Append(Map.Path + TEXT("/") + Map.Name + TEXT("+"));
  }

  FString MapName("PropsMap");
  FString WorldDestPath = TEXT("/Game/") + PackageParams.Name +
      TEXT("/Maps/") + MapName;

  MapPathData.Append(WorldDestPath + TEXT("/") + MapName);

  // Save Map Path File for further use
  FString SaveDirectory = FPaths::ProjectContentDir() + PackageParams.Name + TEXT("/Config");
  FString FileName = FString("MapPaths.txt");
  SaveStringTextToFile(SaveDirectory, FileName, MapPathData, true);

  // Add props in a single Base Map
  AddMeshesToWorld(AssetsPaths.PropsPaths, false);

  SaveWorld(AssetData, PackageParams.Name, WorldDestPath, MapName);

  return 0;
}
#endif
