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

  // #endif
}
// #if WITH_EDITORONLY_DATA

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

void UCookAssetsCommandlet::MoveMeshesToMap(const FString &SrcPath, const TArray<FString> &DestPath)
{}

void UCookAssetsCommandlet::LoadWorld(FAssetData &AssetData)
{
  FString BaseMap = TEXT("/Game/Carla/Maps/BaseMap");

  AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  AssetsObjectLibrary->AddToRoot();
  AssetsObjectLibrary->LoadAssetDataFromPath(*BaseMap);
  AssetsObjectLibrary->LoadAssetsFromAssetData();
  AssetsObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    AssetData = AssetDatas.Pop();
  }
}

void UCookAssetsCommandlet::AddMeshesToWorld(
    const TArray<FString> &AssetsPaths,
    bool bUseCarlaMaterials)
{

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
  for (auto MapAsset : MapContents)
  {
    UE_LOG(LogTemp, Log, TEXT("Add Assets"));
    MeshAsset = CastChecked<UStaticMesh>(MapAsset.GetAsset());
    MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
        initialVector,
        initialRotator);
    MeshActor->GetStaticMeshComponent()->SetStaticMesh(CastChecked<UStaticMesh>(MeshAsset));

    if (bUseCarlaMaterials)
    {
      // FString AssetName;
      // MapAsset.AssetName.ToString(AssetName);
      // if (AssetName.Contains("MarkingNode"))
      // {
      //   MeshActor->GetStaticMeshComponent()->SetMaterial(0,
      // MarkingNodeMaterial);
      //   MeshActor->GetStaticMeshComponent()->SetMaterial(1,
      // MarkingNodeMaterialAux);
      // }
      // else if (AssetName.Contains("RoadNode"))
      // {
      //   MeshActor->GetStaticMeshComponent()->SetMaterial(0,
      // RoadNodeMaterial);
      // } else if (AssetName.Contains("Terrain"))
      // {
      //   MeshActor->GetStaticMeshComponent()->SetMaterial(0,
      // TerrainNodeMaterial);
      // }
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
  //   CastChecked<AOpenDriveActor>(World->SpawnActor(AOpenDriveActor::StaticClass(),
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

        FString MapAssetPath = MapJsonObject->GetStringField(TEXT("path"));
        bool bUseCarlaMaterials = MapJsonObject->GetBoolField(TEXT("use_carla_materials"));

        // AssetsPaths.MapsPaths.Add(TPair<FString, bool>(MapAssetPath,
        // bUseCarlaMaterials));
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

  // Load World

  UE_LOG(LogTemp, Log, TEXT("------ ADDING MAPS ------"));

  FString RoadsPath = TEXT("/Game/Carla/Static/Road/") + PackageParams.Name;
  FString MarkingLinePath = TEXT("/Game/Carla/Static/RoadLines/") + PackageParams.Name;
  FString TerrainPath = TEXT("/Game/Carla/Static/Terrain/") + PackageParams.Name;

  TArray<FString> DataPath = {RoadsPath, MarkingLinePath, TerrainPath};
  // DataPath.Add(RoadsPath);
  // DataPath.Add(MarkingLinePath);
  // DataPath.Add(TerrainPath);

  UE_LOG(LogTemp, Log, TEXT("------ ADDING MAPS TO WORLD ------"));
  // for (auto Map : AssetsPaths.MapsPaths) {
  // FAssetData AssetData;
  // LoadWorld(AssetData);
  // World = CastChecked<UWorld>(AssetData.GetAsset());
  // AddMeshesToWorld(Map.Key, Map.Value);
  // }

  UE_LOG(LogTemp, Log, TEXT("------ ADDING PROPS TO WORLD ------"));
  FAssetData AssetData;
  LoadWorld(AssetData);
  World = CastChecked<UWorld>(AssetData.GetAsset());
  // Add props in a single base map
  AddMeshesToWorld(AssetsPaths.PropsPaths, false);

  UE_LOG(LogTemp, Log, TEXT("------ SAVING BASEMAP WORLD ------"));
  FString WorldDestPath = TEXT("/Game/") + PackageParams.Name + "/Maps/MapName";
  FString MapName("MapName");
  SaveWorld(AssetData, WorldDestPath, MapName);

  return 0;
}

// #endif
