// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "PrepareAssetsForCookingCommandlet.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformFile.h"

#include "UObject/MetaData.h"

UPrepareAssetsForCookingCommandlet::UPrepareAssetsForCookingCommandlet()
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

FPackageParams UPrepareAssetsForCookingCommandlet::ParseParams(const FString &InParams) const
{
  TArray<FString> Tokens;
  TArray<FString> Params;
  TMap<FString, FString> ParamVals;

  ParseCommandLine(*InParams, Tokens, Params);

  FPackageParams PackageParams;
  FParse::Value(*InParams, TEXT("PackageName="), PackageParams.Name);
  FParse::Bool(*InParams, TEXT("OnlyPrepareMaps="), PackageParams.bOnlyPrepareMaps);
  return PackageParams;
}

void UPrepareAssetsForCookingCommandlet::LoadWorld(FAssetData &AssetData)
{
  // BaseMap path inside Carla
  FString BaseMap = TEXT("/Game/Carla/Maps/BaseMap");

  // Load Map folder using object library
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*BaseMap);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    // Extract first asset found in folder path (i.e. the BaseMap)
    AssetData = AssetDatas.Pop();
  }
}

TArray<AStaticMeshActor *> UPrepareAssetsForCookingCommandlet::SpawnMeshesToWorld(
    const TArray<FString> &AssetsPaths,
    bool bUseCarlaMaterials,
    bool bIsPropsMap)
{
  TArray<AStaticMeshActor *> SpawnedMeshes;

  // Remove the meshes names from the original path for props, so we can load
  // props inside folder
  TArray<FString> AssetsPathsDirectories = AssetsPaths;
  if (bIsPropsMap)
  {
    for (auto &AssetPath : AssetsPathsDirectories)
    {
      AssetPath.Split(
          TEXT("/"),
          &AssetPath,
          nullptr,
          ESearchCase::Type::IgnoreCase,
          ESearchDir::Type::FromEnd);
    }
  }

  // Load assets specified in AssetsPathsDirectories by using an object library
  // for building map world
  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPaths(AssetsPathsDirectories);
  AssetsObjectLibrary->LoadAssetsFromAssetData();
  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);

  // Create default Transform for all assets to spawn
  const FTransform zeroTransform = FTransform();
  FVector initialVector = FVector(0, 0, 0);
  FRotator initialRotator = FRotator(0, 180, 0);

  UStaticMesh *MeshAsset;
  AStaticMeshActor *MeshActor;

  for (auto MapAsset : MapContents)
  {
    // Spawn Static Mesh
    MeshAsset = CastChecked<UStaticMesh>(MapAsset.GetAsset());
    MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
        initialVector,
        initialRotator);
    MeshActor->GetStaticMeshComponent()->SetStaticMesh(CastChecked<UStaticMesh>(MeshAsset));
    SpawnedMeshes.Add(MeshActor);
    if (bUseCarlaMaterials)
    {
      // Set Carla Materials
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

  // Clear loaded assets in library
  AssetsObjectLibrary->ClearLoaded();

  // Mark package dirty
  World->MarkPackageDirty();

  return SpawnedMeshes;
}

void UPrepareAssetsForCookingCommandlet::DestroySpawnedActorsInWorld(
    TArray<AStaticMeshActor *> &SpawnedActors)
{
  // Destroy all spawned actors
  for (auto Actor : SpawnedActors)
  {
    Actor->Destroy();
  }

  // Mark package dirty
  World->MarkPackageDirty();
}

bool UPrepareAssetsForCookingCommandlet::SaveWorld(
    FAssetData &AssetData,
    FString &PackageName,
    FString &DestPath,
    FString &WorldName)
{
  // Create Package to save
  UPackage *Package = AssetData.GetPackage();
  Package->SetFolderName(*DestPath);
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(World);

  // Renaming map
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
    // We need to spawn OpenDrive assets before saving the map
    AOpenDriveActor *OpenWorldActor =
        CastChecked<AOpenDriveActor>(World->SpawnActor(AOpenDriveActor::StaticClass(),
        new FVector(), NULL));

    OpenWorldActor->BuildRoutes(WorldName);
    OpenWorldActor->AddSpawners();

    SavePackage(PackagePath, Package);

    // We need to destroy OpenDrive assets once saved the map
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

FString UPrepareAssetsForCookingCommandlet::GetFirstPackagePath(const FString &PackageName) const
{
  // Get all Package names
  TArray<FString> PackageList;
  IFileManager::Get().FindFilesRecursive(PackageList, *(FPaths::ProjectContentDir()),
      *(PackageName + TEXT(".Package.json")), true, false, false);

  if (PackageList.Num() == 0)
  {
    UE_LOG(LogTemp, Error, TEXT("Package json file not found."));
    return {};
  }

  return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*PackageList[0]);
}

FAssetsPaths UPrepareAssetsForCookingCommandlet::GetAssetsPathFromPackage(const FString &PackageName) const
{
  FString PackageJsonFilePath = GetFirstPackagePath(PackageName);

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

bool UPrepareAssetsForCookingCommandlet::SavePackage(const FString &PackagePath, UPackage *Package) const
{
  FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath,
      FPackageName::GetMapPackageExtension());

  if (FPaths::FileExists(*PackageFileName))
  {
    // Will not save package if it already exists
    return false;
  }
  return UPackage::SavePackage(Package, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
}

int32 UPrepareAssetsForCookingCommandlet::Main(const FString &Params)
{
  FPackageParams PackageParams = ParseParams(Params);

  // Get Props and Maps Path
  FAssetsPaths AssetsPaths = GetAssetsPathFromPackage(PackageParams.Name);

  // Load World
  FAssetData AssetData;
  LoadWorld(AssetData);
  World = CastChecked<UWorld>(AssetData.GetAsset());

  if (PackageParams.bOnlyPrepareMaps)
  {
    for (auto Map : AssetsPaths.MapsPaths)
    {
      FString RoadsPath = TEXT("/Game/") + PackageParams.Name + TEXT("/Static/RoadNode/") + Map.Name;
      FString MarkingLinePath = TEXT("/Game/") + PackageParams.Name + TEXT("/Static/MarkingNode/") + Map.Name;
      FString TerrainPath = TEXT("/Game/") + PackageParams.Name + TEXT("/Static/TerrainNode/") + Map.Name;

      TArray<FString> DataPath = {RoadsPath, MarkingLinePath, TerrainPath};

      // Add Meshes to inside the loaded World
      TArray<AStaticMeshActor *> SpawnedActors = SpawnMeshesToWorld(DataPath, Map.bUseCarlaMapMaterials);

      // Save the World in specified path
      SaveWorld(AssetData, PackageParams.Name, Map.Path, Map.Name);

      // Remove spawned actors from world to keep equal as BaseMap
      DestroySpawnedActorsInWorld(SpawnedActors);
    }
  }
  else
  {
    FString MapPathData;
    for (auto Map : AssetsPaths.MapsPaths)
    {
      MapPathData.Append(Map.Path + TEXT("/") + Map.Name + TEXT("+"));
    }

    if (AssetsPaths.PropsPaths.Num() > 0)
    {
      FString MapName("PropsMap");
      FString WorldDestPath = TEXT("/Game/") + PackageParams.Name +
          TEXT("/Maps/") + MapName;

      MapPathData.Append(WorldDestPath + TEXT("/") + MapName);

      // Add props in a single Base Map
      TArray<AStaticMeshActor *> SpawnedActors = SpawnMeshesToWorld(AssetsPaths.PropsPaths, false, true);

      SaveWorld(AssetData, PackageParams.Name, WorldDestPath, MapName);

      DestroySpawnedActorsInWorld(SpawnedActors);
      MapObjectLibrary->ClearLoaded();
    }
    else
    {
      if (MapPathData.Len() >= 0)
      {
        MapPathData.RemoveFromEnd(TEXT("+"));
      }
    }

    // Save Map Path File for further use
    FString SaveDirectory = FPaths::ProjectContentDir();
    FString FileName = FString("MapPaths.txt");
    SaveStringTextToFile(SaveDirectory, FileName, MapPathData, true);

    FileName = FString("PackagePath.txt");
    FString PackageJsonFilePath = GetFirstPackagePath(PackageParams.Name);
    SaveStringTextToFile(SaveDirectory, FileName, PackageJsonFilePath, true);
  }

  return 0;
}
#endif
