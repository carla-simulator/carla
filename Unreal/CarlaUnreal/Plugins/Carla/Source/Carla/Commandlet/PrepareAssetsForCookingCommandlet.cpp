// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "PrepareAssetsForCookingCommandlet.h"
#include "Carla/MapGen/LargeMapManager.h"

#include "SSTags.h"

#include <util/ue-header-guard-begin.h>
#if WITH_EDITOR
#include "FileHelpers.h"
#endif
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/PlatformFileManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceConstant.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PhysicsEngine/BodySetup.h"
#include "UObject/SavePackage.h"
#include "UObject/Package.h"
#include <util/ue-header-guard-end.h>

static bool ValidateStaticMesh(UStaticMesh *Mesh)
{
  const FString AssetName = Mesh->GetName();

  if (AssetName.Contains(TEXT("light"), ESearchCase::IgnoreCase) ||
      AssetName.Contains(TEXT("sign"), ESearchCase::IgnoreCase))
  {
    return false;
  }

  for (int i = 0; i < Mesh->GetStaticMaterials().Num(); i++)
  {
    UMaterialInterface *Material = Mesh->GetMaterial(i);
    const FString MaterialName = Material->GetName();

    if (MaterialName.Contains(TEXT("light"), ESearchCase::IgnoreCase) ||
        MaterialName.Contains(TEXT("sign"), ESearchCase::IgnoreCase))
    {
      return false;
    }
  }

  return true;
}

UPrepareAssetsForCookingCommandlet::UPrepareAssetsForCookingCommandlet()
{
  // Set necessary flags to run commandlet
  IsClient = false;
  IsEditor = true;
  IsServer = false;
  LogToConsole = true;

#if WITH_EDITORONLY_DATA
  // Get Carla Default materials, these will be used for maps that need to use
  // Carla materials
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingNodeYellowMaterial(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LargeMaps/M_Road_03_Tiled_V3.M_Road_03_Tiled_V3'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MarkingNodeWhiteMaterial(TEXT(
    "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/M_Road_03_LMW.M_Road_03_LMW'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> RoadNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/RoadPainterMaterials/LargeMaps/M_Road_03_Tiled_V2.M_Road_03_Tiled_V2'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TerrainNodeMaterial(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/00_MastersOpt/Large_Maps/materials/MI_LargeLandscape_Grass.MI_LargeLandscape_Grass'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> CurbNodeMaterial(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/LargeMap_materials/largeM_curb/MI_largeM_curb01.MI_largeM_curb01'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> GutterNodeMaterial(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/LargeMap_materials/largeM_gutter/MI_largeM_gutter01.MI_largeM_gutter01'"));
  static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> SidewalkNode(TEXT(
      "MaterialInstanceConstant'/Game/Carla/Static/GenericMaterials/LargeMap_materials/largeM_sidewalk/tile01/MI_largeM_tile02.MI_largeM_tile02'"));

  GutterNodeMaterialInstance = (UMaterialInstance *) GutterNodeMaterial.Object;
  CurbNodeMaterialInstance = (UMaterialInstance *) CurbNodeMaterial.Object;
  TerrainNodeMaterialInstance = (UMaterialInstance *) TerrainNodeMaterial.Object;
  MarkingNodeYellow = (UMaterialInstance *)MarkingNodeYellowMaterial.Object;
  MarkingNodeWhite = (UMaterialInstance *)MarkingNodeWhiteMaterial.Object;
  RoadNodeMaterial = (UMaterialInstance *) RoadNode.Object;
  SidewalkNodeMaterialInstance = (UMaterialInstance *) SidewalkNode.Object;
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

  // Parse and store Package name
  FParse::Value(*InParams, TEXT("PackageName="), PackageParams.Name);

  // Parse and store flag for only preparing maps
  FParse::Bool(*InParams, TEXT("OnlyPrepareMaps="), PackageParams.bOnlyPrepareMaps);
  return PackageParams;
}

void UPrepareAssetsForCookingCommandlet::LoadWorld(FAssetData &AssetData)
{
  // BaseMap path inside Carla
  const FString BaseMap = TEXT("/Game/Carla/Maps/BaseMap");

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

void UPrepareAssetsForCookingCommandlet::LoadWorldTile(FAssetData &AssetData)
{
  // BaseTile path inside Carla
  const FString BaseTile = TEXT("/Game/Carla/Maps/TestMaps");

  // Load Map folder using object library
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*BaseTile);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    // Extract first asset found in folder path (i.e. the BaseTile)
    AssetData = AssetDatas.Pop();
  }
}

void UPrepareAssetsForCookingCommandlet::LoadLargeMapWorld(FAssetData &AssetData)
{
  // BaseMap path inside Carla
  const FString BaseMap = TEXT("/Game/Carla/Maps/BaseLargeMap");

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
    int i,
    int j)
{
  TArray<AStaticMeshActor *> SpawnedMeshes;

  // Create default Transform for all assets to spawn
  const FTransform ZeroTransform = FTransform();

  // Load assets specified in AssetsPaths by using an object library
  // for building map world
  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPaths(AssetsPaths);
  AssetsObjectLibrary->LoadAssetsFromAssetData();
  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);

  UStaticMesh *MeshAsset;
  AStaticMeshActor *MeshActor;

  // name of current tile to cook
  FString TileName;
  if (i != -1)
  {
    TileName = FString::Printf(TEXT("_Tile_%d_%d"), i, j);
  }

  // try to get the name of the map that precedes all assets name
  FString AssetName;
  for (auto MapAsset : MapContents)
  {
    // Spawn Static Mesh
    MeshAsset = Cast<UStaticMesh>(MapAsset.GetAsset());
    if (MeshAsset && ValidateStaticMesh(MeshAsset))
    {
      // get asset name
      MapAsset.AssetName.ToString(AssetName);

      // check to ignore meshes from other tiles
      if (i == -1 || (i != -1 && (AssetName.EndsWith(TileName) || AssetName.Contains(TileName + "_"))))
      {
        MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), ZeroTransform);
        UStaticMeshComponent *MeshComponent = MeshActor->GetStaticMeshComponent();
        MeshComponent->SetStaticMesh(CastChecked<UStaticMesh>(MeshAsset));
        MeshActor->SetActorLabel(AssetName, true);

        // set complex collision as simple in asset
        UBodySetup *BodySetup = MeshAsset->GetBodySetup();
        if (BodySetup)
        {
          BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
          MeshAsset->MarkPackageDirty();
        }

        SpawnedMeshes.Add(MeshActor);

        if (bUseCarlaMaterials)
        {
          // Set Carla Materials depending on RoadRunner's Semantic Segmentation
          // tag
          if (AssetName.Contains(SSTags::R_MARKING1) || AssetName.Contains(SSTags::R_MARKING2))
          {
            for (int32 materialIndex = 0; materialIndex < MeshActor->GetStaticMeshComponent()->GetStaticMesh()->GetStaticMaterials().Num(); ++materialIndex)
            {
              if (MeshActor->GetStaticMeshComponent()->GetStaticMesh()->GetStaticMaterials()[materialIndex].ImportedMaterialSlotName.ToString().Contains("Yellow"))
              {
                MeshActor->GetStaticMeshComponent()->SetMaterial(materialIndex, MarkingNodeYellow);
              }
              else
              {
                MeshActor->GetStaticMeshComponent()->SetMaterial(materialIndex, MarkingNodeWhite);
              }
            }
          }
          else if (AssetName.Contains(SSTags::R_ROAD1) || AssetName.Contains(SSTags::R_ROAD2))
          {
            MeshActor->GetStaticMeshComponent()->SetMaterial(0, RoadNodeMaterial);
          }
          else if (AssetName.Contains(SSTags::R_TERRAIN))
          {
            MeshActor->GetStaticMeshComponent()->SetMaterial(0, TerrainNodeMaterialInstance);
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
          else if (AssetName.Contains(SSTags::R_SIDEWALK1) || AssetName.Contains(SSTags::R_SIDEWALK2))
          {
            MeshActor->GetStaticMeshComponent()->SetMaterial(0, SidewalkNodeMaterialInstance);
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
          else if (AssetName.Contains(SSTags::R_CURB1) || AssetName.Contains(SSTags::R_CURB2)) {

            MeshActor->GetStaticMeshComponent()->SetMaterial(0, CurbNodeMaterialInstance);
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
          else if (AssetName.Contains(SSTags::R_GUTTER1) || AssetName.Contains(SSTags::R_GUTTER2)) {

            MeshActor->GetStaticMeshComponent()->SetMaterial(0, GutterNodeMaterialInstance);
            MeshActor->GetStaticMeshComponent()->bReceivesDecals = false;
          }
        }
      }
    }
  }

  // Clear loaded assets in library
  AssetsObjectLibrary->ClearLoaded();

  // Mark package dirty
  World->MarkPackageDirty();

  return SpawnedMeshes;
}

bool UPrepareAssetsForCookingCommandlet::IsMapInTiles(const TArray<FString> &AssetsPaths)
{
  // Load assets specified in AssetsPaths by using an object library
  // for building map world
  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPaths(AssetsPaths);
  AssetsObjectLibrary->LoadAssetsFromAssetData();
  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);

  UStaticMesh *MeshAsset;

  FString AssetName;
  bool Found = false;
  for (auto MapAsset : MapContents)
  {
    // Spawn Static Mesh
    MeshAsset = Cast<UStaticMesh>(MapAsset.GetAsset());
    if (MeshAsset && ValidateStaticMesh(MeshAsset))
    {
      // get asset name
      MapAsset.AssetName.ToString(AssetName);

      // check if the asset is a tile
      if (AssetName.Contains("_Tile_"))
      {
        Found = true;
        break;
      }
    }
  }

  // Clear loaded assets in library
  AssetsObjectLibrary->ClearLoaded();

  return Found;
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
    const FString &PackageName,
    const FString &DestPath,
    const FString &WorldName,
    bool bGenerateSpawnPoints)
{
  // Create Package to save
  UPackage *Package = AssetData.GetPackage();
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(World);

  // Renaming map
  World->Rename(*WorldName, World->GetOuter());
  const FString PackagePath = DestPath + "/" + WorldName;
  FAssetRegistryModule::AssetRenamed(World, *PackagePath);
  World->MarkPackageDirty();
  World->GetOuter()->MarkPackageDirty();

  // Check if OpenDrive file exists
  const FString PathXODR = FPaths::ProjectContentDir() + PackageName + TEXT("/Maps/") +
      WorldName + TEXT("/OpenDrive/") + WorldName + TEXT(".xodr");

  bool bPackageSaved = false;
  if (FPaths::FileExists(PathXODR) && bGenerateSpawnPoints)
  {
    // We need to spawn OpenDrive assets before saving the map
    AOpenDriveActor *OpenWorldActor = CastChecked<AOpenDriveActor>(
        World->SpawnActor(AOpenDriveActor::StaticClass(),
        new FVector(),
        NULL));

    OpenWorldActor->BuildRoutes(WorldName);
    OpenWorldActor->AddSpawners();

    bPackageSaved = SavePackage(PackagePath, Package);

    // We need to destroy OpenDrive assets once saved the map
    OpenWorldActor->RemoveRoutes();
    OpenWorldActor->RemoveSpawners();
    OpenWorldActor->Destroy();
  }
  else
  {
    bPackageSaved = SavePackage(PackagePath, Package);
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
  const FString PackageJsonFilePath = GetFirstPackagePath(PackageName);

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

        const FString PropAssetPath = PropJsonObject->GetStringField(TEXT("path"));

        AssetsPaths.PropsPaths.Add(std::move(PropAssetPath));
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
    const FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

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
  const FString PackageFileName = FPackageName::LongPackageNameToFilename(
      PackagePath,
      FPackageName::GetMapPackageExtension());

  if (FPaths::FileExists(*PackageFileName))
  {
    // Will not save package if it already exists
    return false;
  }

  FSavePackageArgs SaveArgs;
  SaveArgs.TopLevelFlags =
    EObjectFlags::RF_Public |
    EObjectFlags::RF_Standalone;
  SaveArgs.Error = GError;
  SaveArgs.bForceByteSwapping = true;
  SaveArgs.bWarnOfLongFilename = true;
  SaveArgs.SaveFlags = SAVE_NoError;

  return UPackage::SavePackage(
      Package,
      World,
      *PackageFileName,
      SaveArgs);
}

void UPrepareAssetsForCookingCommandlet::GenerateMapPathsFile(
    const FAssetsPaths &AssetsPaths,
    const FString &PropsMapPath)
{
  FString MapPathData;
  FString MapPathDataLinux;
  IFileManager &FileManager = IFileManager::Get();
  for (const auto &Map : AssetsPaths.MapsPaths)
  {
    MapPathData.Append(Map.Path + TEXT("/") + Map.Name + TEXT("\n"));
    MapPathDataLinux.Append(Map.Path + TEXT("/") + Map.Name + TEXT("+"));
    TArray<FAssetData> AssetsData;
    UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), true, true);
    ObjectLibrary->LoadAssetDataFromPath(Map.Path);
    ObjectLibrary->GetAssetDataList(AssetsData);
    int NumTiles = 0;
    for (FAssetData &AssetData : AssetsData)
    {
      FString AssetName = AssetData.AssetName.ToString();
      if (AssetName.Contains(Map.Name + "_Tile_"))
      {
        MapPathData.Append(Map.Path + TEXT("/") + AssetName + TEXT("\n"));
        MapPathDataLinux.Append(Map.Path + TEXT("/") + AssetName + TEXT("+"));
        NumTiles++;
      }
    }
    UE_LOG(LogTemp, Warning, TEXT("Found %d tiles"), NumTiles);
  }

  if (!PropsMapPath.IsEmpty())
  {
    MapPathData.Append(PropsMapPath + TEXT("/PropsMap"));
    MapPathDataLinux.Append(PropsMapPath + TEXT("/PropsMap"));
  }
  else
  {
    MapPathDataLinux.RemoveFromEnd(TEXT("+"));
  }

  const FString SaveDirectory = FPaths::ProjectContentDir();
  const FString FileName = FString("MapPaths.txt");
  const FString FileNameLinux = FString("MapPathsLinux.txt");
  SaveStringTextToFile(SaveDirectory, FileName, MapPathData, true);
  SaveStringTextToFile(SaveDirectory, FileNameLinux, MapPathDataLinux, true);
}

void UPrepareAssetsForCookingCommandlet::GeneratePackagePathFile(const FString &PackageName)
{
  FString SaveDirectory = FPaths::ProjectContentDir();
  FString FileName = FString("PackagePath.txt");
  FString PackageJsonFilePath = GetFirstPackagePath(PackageName);
  SaveStringTextToFile(SaveDirectory, FileName, PackageJsonFilePath, true);
}

void UPrepareAssetsForCookingCommandlet::PrepareMapsForCooking(
    const FString &PackageName,
    const TArray<FMapData> &MapsPaths)
{
  FString BasePath = TEXT("/Game/") + PackageName + TEXT("/Static/");

  for (const auto &Map : MapsPaths)
  {
    const FString MapPath = TEXT("/") + Map.Name;

    const FString DefaultPath   = TEXT("/Game/") + PackageName + TEXT("/Maps/") + Map.Name;
    const FString RoadsPath     = BasePath + SSTags::ROAD      + MapPath;
    const FString RoadLinesPath = BasePath + SSTags::ROADLINE  + MapPath;
    const FString TerrainPath   = BasePath + SSTags::TERRAIN   + MapPath;
    const FString SidewalkPath  = BasePath + SSTags::SIDEWALK  + MapPath;

    // Spawn assets located in semantic segmentation folders
    TArray<FString> DataPath = {DefaultPath, RoadsPath, RoadLinesPath, TerrainPath, SidewalkPath};

    // check whether we have a single map or a map in tiles
    if (!IsMapInTiles(DataPath))
    {
      UE_LOG(LogTemp, Log, TEXT("Cooking map"));
      // Load World
      FAssetData AssetData;
      LoadWorld(AssetData);
      UObjectRedirector *BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
      if (BaseMapRedirector != nullptr) {
        World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
      }
      else {
        World = CastChecked<UWorld>(AssetData.GetAsset());
      }
      // try to cook the whole map (no tiles)
      TArray<AStaticMeshActor *> SpawnedActors = SpawnMeshesToWorld(DataPath, Map.bUseCarlaMapMaterials, -1, -1);
      // Save the World in specified path
      SaveWorld(AssetData, PackageName, Map.Path, Map.Name);
      // Remove spawned actors from world to keep equal as BaseMap
      DestroySpawnedActorsInWorld(SpawnedActors);
    }
    else
    {
      TArray<TPair<FString, FIntVector>> MapPathsIds;

      FVector PositionTile0 = FVector();
      float TileSize = 200000.f;
      FString TxtFile;
      FString TilesInfoPath = FPaths::ProjectContentDir() + PackageName + TEXT("/Maps/") + Map.Name + "/TilesInfo.txt";
      UE_LOG(LogTemp, Warning, TEXT("Loading %s ..."), *TilesInfoPath);
      if (FFileHelper::LoadFileToString(TxtFile, *(TilesInfoPath)) == true) {

        TArray<FString> Out;
        TxtFile.ParseIntoArray(Out, TEXT(","), true);
        if (Out.Num() >= 3)
        {
          const float METERSTOCM = 100.f;
          PositionTile0.X = METERSTOCM * FCString::Atof(*Out[0]);
          PositionTile0.Y = METERSTOCM * FCString::Atof(*Out[1]);
          TileSize = METERSTOCM * FCString::Atof(*Out[2]);
        }
        else
        {
          UE_LOG(LogTemp, Warning, TEXT("TilesInfo.txt format is invalid file"));
        }
      }
      else {
        UE_LOG(LogTemp, Warning, TEXT("Could not find TilesInfo.txt file"));
      }

      UE_LOG(LogTemp, Log, TEXT("Cooking tiles:"));
      // Load World
      FAssetData AssetData;
      LoadWorldTile(AssetData);
      UObjectRedirector *BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
      if (BaseMapRedirector != nullptr) {
        World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
      }
      else {
        World = CastChecked<UWorld>(AssetData.GetAsset());
      }
      // try to create each possible tile of the map
      int  i, j;
      bool Res;
      j = 0;
      do
      {
        i = 0;
        do
        {
          // Spawn
          TArray<AStaticMeshActor *> SpawnedActors = SpawnMeshesToWorld(DataPath, Map.bUseCarlaMapMaterials, i, j);
          Res = SpawnedActors.Num() > 0;
          if (Res)
          {
            UE_LOG(LogTemp, Log, TEXT(" Tile %d,%d found"), i, j);
            FString TileName;
            TileName = FString::Printf(TEXT("%s_Tile_%d_%d"), *Map.Name, i, j);
            // Save the World in specified path
            // UE_LOG(LogTemp, Log, TEXT("Saving as %s to %s"), *TileName, *Map.Path);
            SaveWorld(AssetData, PackageName, Map.Path, TileName);
            MapPathsIds.Add(
                TPair<FString, FIntVector>(
                  Map.Path + "/" + TileName, FIntVector(i, j, 0)));
            // Remove spawned actors from world to keep equal as BaseMap
            DestroySpawnedActorsInWorld(SpawnedActors);
            ++i;
          }
        }
        while (Res);
        ++j;
      }
      while (i > 0);

      #if WITH_EDITOR
        UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
      #endif
      // Load base map for tiled maps
      LoadLargeMapWorld(AssetData);
      BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
      if (BaseMapRedirector != nullptr) {
        World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
      }
      else {
        World = CastChecked<UWorld>(AssetData.GetAsset());
      }

      // Generate Large Map Manager
      ALargeMapManager* LargeMapManager = World->SpawnActor<ALargeMapManager>(
          ALargeMapManager::StaticClass(), FTransform());
      LargeMapManager->LargeMapTilePath = Map.Path;
      LargeMapManager->LargeMapName = Map.Name;
      LargeMapManager->SetTile0Offset(PositionTile0);
      LargeMapManager->SetTileSize(TileSize);
      LargeMapManager->GenerateMap(MapPathsIds);

      SaveWorld(AssetData, PackageName, Map.Path, Map.Name, false);

      UE_LOG(LogTemp, Log, TEXT("End cooking tiles"));
    }
  }
}

void UPrepareAssetsForCookingCommandlet::PreparePropsForCooking(
    FString &PackageName,
    const TArray<FString> &PropsPaths,
    FString &MapDestPath)
{
  // Load World
  FAssetData AssetData;
  // Loads the BaseMap
  LoadWorld(AssetData);
  UObjectRedirector *BaseMapRedirector = Cast<UObjectRedirector>(AssetData.GetAsset());
  if (BaseMapRedirector != nullptr) {
    World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
  }
  else {
    World = CastChecked<UWorld>(AssetData.GetAsset());
  }

  // Remove the meshes names from the original path for props, so we can load
  // props inside folder
  TArray<FString> PropPathDirs = PropsPaths;

  for (auto &PropPath : PropPathDirs)
  {
    PropPath.Split(TEXT("/"), &PropPath, nullptr,
        ESearchCase::Type::IgnoreCase, ESearchDir::Type::FromEnd);
  }

  // Add props in a single Base Map
  TArray<AStaticMeshActor *> SpawnedActors = SpawnMeshesToWorld(PropPathDirs, false);

  const FString MapName("PropsMap");
  SaveWorld(AssetData, PackageName, MapDestPath, MapName);

  DestroySpawnedActorsInWorld(SpawnedActors);
  MapObjectLibrary->ClearLoaded();
}

int32 UPrepareAssetsForCookingCommandlet::Main(const FString &Params)
{
  FPackageParams PackageParams = ParseParams(Params);

  // Get Props and Maps Path
  FAssetsPaths AssetsPaths = GetAssetsPathFromPackage(PackageParams.Name);

  if (PackageParams.bOnlyPrepareMaps)
  {
    PrepareMapsForCooking(PackageParams.Name, AssetsPaths.MapsPaths);
  }
  else
  {
    FString PropsMapPath("");

    if (AssetsPaths.PropsPaths.Num() > 0)
    {
      PropsMapPath = TEXT("/Game/") + PackageParams.Name + TEXT("/Maps/PropsMap");
      PreparePropsForCooking(PackageParams.Name, AssetsPaths.PropsPaths, PropsMapPath);
    }

    // Save Map Path File for further use
    GenerateMapPathsFile(AssetsPaths, PropsMapPath);

    // Saves Package path for further use
    GeneratePackagePathFile(PackageParams.Name);
  }

#if WITH_EDITOR
  UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
#endif

  return 0;
}
#endif
