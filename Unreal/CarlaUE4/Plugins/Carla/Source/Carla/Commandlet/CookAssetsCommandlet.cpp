// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "CookAssetsCommandlet.h"
#include "GameFramework/WorldSettings.h"
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

void UCookAssetsCommandlet::AddMeshesToWorld(
    const TArray<FString> &AssetsPaths,
    bool bUseCarlaMaterials)
{
  for (auto s : AssetsPaths) {
    UE_LOG(LogTemp, Log, TEXT("MESH: %s"), *s)
  }
  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPaths(AssetsPaths);
  AssetsObjectLibrary->LoadAssetsFromAssetData();

  const FTransform zeroTransform = FTransform();
  FVector initialVector = FVector(0, 0, 0);
  FRotator initialRotator = FRotator(0, 180, 0);
  FActorSpawnParameters SpawnInfo;

  MapContents.Empty();
  AssetsObjectLibrary->GetAssetDataList(MapContents);
  UStaticMesh *MeshAsset;
  AStaticMeshActor *MeshActor;

  if(MapContents.Num() <= 0) {
    UE_LOG(LogTemp, Log, TEXT("EMPTY MAP CONTENTS FOUND"));
  }

  for (auto MapAsset : MapContents)
  {
    UE_LOG(LogTemp, Log, TEXT("ADDING MESHES"));
    MeshAsset = CastChecked<UStaticMesh>(MapAsset.GetAsset());
    MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
        initialVector,
        initialRotator);
    MeshActor->GetStaticMeshComponent()->SetStaticMesh(CastChecked<UStaticMesh>(MeshAsset));

    if (bUseCarlaMaterials)
    {
      FString AssetName;
      MapAsset.AssetName.ToString(AssetName);
      if (AssetName.Contains("MarkingNode"))
      {
        MeshActor->GetStaticMeshComponent()->SetMaterial(0,
            MarkingNodeMaterial);
        MeshActor->GetStaticMeshComponent()->SetMaterial(1,
            MarkingNodeMaterialAux);
      }
      else if (AssetName.Contains("RoadNode"))
      {
        MeshActor->GetStaticMeshComponent()->SetMaterial(0,
            RoadNodeMaterial);
      }
      else if (AssetName.Contains("Terrain"))
      {
        MeshActor->GetStaticMeshComponent()->SetMaterial(0,
            TerrainNodeMaterial);
      }
    }
  }

  World->MarkPackageDirty();
}

bool UCookAssetsCommandlet::SaveWorld(FAssetData &AssetData, FString &DestPath, FString &WorldName)
{
  FString PackageName = DestPath + "/" + WorldName;
  UPackage *Package = AssetData.GetPackage();
  Package->SetFolderName(*DestPath);
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(World);

  // Renaming stuff for the map
  World->Rename(*WorldName, World->GetOuter());
  FAssetRegistryModule::AssetRenamed(World, *PackageName);
  World->MarkPackageDirty();
  World->GetOuter()->MarkPackageDirty();

  // Filling the map stuff (Code only applied for maps)
  // AOpenDriveActor *OpenWorldActor =
  //
  //
  // CastChecked<AOpenDriveActor>(World->SpawnActor(AOpenDriveActor::StaticClass(),
  // new FVector(), NULL));
  // OpenWorldActor->BuildRoutes(WorldName);
  // OpenWorldActor->AddSpawners();

  // Saving the package
  FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName,
      FPackageName::GetMapPackageExtension());
  return UPackage::SavePackage(Package, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
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

int32 UCookAssetsCommandlet::Main(const FString &Params)
{
  FPackageParams PackageParams = ParseParams(Params);
  UE_LOG(LogTemp, Log, TEXT("Call to Cook Assets Commandlet"));
  UE_LOG(LogTemp, Log, TEXT("Package Name: %s"), *PackageParams.Name);

  // Get Props and Maps Path
  UE_LOG(LogTemp, Log, TEXT("------ GET ASSETS PATH ------"));
  FAssetsPaths AssetsPaths = GetAssetsPathFromPackage(PackageParams.Name);

  UE_LOG(LogTemp, Log, TEXT("------ ADDING MAPS TO WORLD ------"));
  for (auto Map : AssetsPaths.MapsPaths)
  {
    UE_LOG(LogTemp, Log, TEXT("MAP NAME: %s"), *Map.Name);
    UE_LOG(LogTemp, Log, TEXT("MAP PATH: %s"), *Map.Path);

    // Load World
    FString RoadsPath = TEXT("/Game/") + PackageParams.Name + TEXT("/Static/RoadNode/") + Map.Name;
    FString MarkingLinePath = TEXT("/Game/") + PackageParams.Name + TEXT("/Static/MarkingNode/") + Map.Name;
    FString TerrainPath = TEXT("/Game/") + PackageParams.Name + TEXT("/Static/TerrainNode/") + Map.Name;

    UE_LOG(LogTemp, Log, TEXT("ROADS PATH: %s"), *RoadsPath);
    UE_LOG(LogTemp, Log, TEXT("MARKING PATH: %s"), *MarkingLinePath);
    UE_LOG(LogTemp, Log, TEXT("TERRAIN PATH: %s"), *TerrainPath);

    TArray<FString> DataPath = {RoadsPath, MarkingLinePath, TerrainPath};

    FAssetData AssetData;
    LoadWorld(AssetData);
    World = CastChecked<UWorld>(AssetData.GetAsset());

    UE_LOG(LogTemp, Log, TEXT("------ ADD MESHES TO MAP ------"));
    // MoveMapMeshes(Map.Path, DataPath);
    // FString Path = TEXT("/Game/") + PackageParams.Name + TEXT("/Maps/") + Map.Name;
    // TArray<FString> Paths;
    // Paths.Add(Path);
    AddMeshesToWorld(DataPath, Map.bUseCarlaMapMaterials);
    UE_LOG(LogTemp, Log, TEXT("-------------------------------"));

    SaveWorld(AssetData, Map.Path, Map.Name);
  }

  // UE_LOG(LogTemp, Log, TEXT("------ ADDING PROPS TO WORLD ------"));
  // FAssetData AssetData;
  // LoadWorld(AssetData);
  // World = CastChecked<UWorld>(AssetData.GetAsset());
  // // Add props in a single base map
  // AddMeshesToWorld(AssetsPaths.PropsPaths, false);

  // UE_LOG(LogTemp, Log, TEXT("------ SAVING BASEMAP WORLD ------"));
  // FString WorldDestPath = TEXT("/Game/") + PackageParams.Name +
  // "/Maps/MapName";
  // FString MapName("MapName");
  // SaveWorld(AssetData, WorldDestPath, MapName);

  return 0;
}

#endif
