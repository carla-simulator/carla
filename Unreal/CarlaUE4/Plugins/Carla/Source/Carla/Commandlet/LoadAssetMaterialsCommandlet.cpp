// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "LoadAssetMaterialsCommandlet.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#endif
#include "Misc/FileHelper.h"
#include "JsonObject.h"
#include "JsonSerializer.h"
#include "UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine/StreamableManager.h"
#include "Engine/StaticMeshActor.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include "Engine/DecalActor.h"
#include "Components/DecalComponent.h"

ULoadAssetMaterialsCommandlet::ULoadAssetMaterialsCommandlet()
{
  // Set necessary flags to run commandlet
  IsClient = false;
  IsEditor = true;
  IsServer = false;
  LogToConsole = true;
  FilledData = false;
  ReadConfigFile = true;

#if WITH_EDITORONLY_DATA

  static ConstructorHelpers::FObjectFinder<UBlueprint> RoadPainterBlueprint(TEXT(
    "Blueprint'/Game/Carla/Blueprints/LevelDesign/RoadPainterPreset.RoadPainterPreset'"));

  RoadPainterSubclass = (UClass*)RoadPainterBlueprint.Object->GeneratedClass;

  // Dirt
  DecalNamesMap.Add("dirt1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_01.DI_RoadDirt_01'");
  DecalNamesMap.Add("dirt2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_02.DI_RoadDirt_02'");
  DecalNamesMap.Add("dirt3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_03.DI_RoadDirt_03'");
  DecalNamesMap.Add("dirt4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_04.DI_RoadDirt_04'");
  DecalNamesMap.Add("dirt5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDirt_05.DI_RoadDirt_05'");

  // Drip
  DecalNamesMap.Add("drip1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDrip_01.DI_RoadDrip_01'");
  DecalNamesMap.Add("drip2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDrip_02.DI_RoadDrip_02'");
  DecalNamesMap.Add("drip3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadDrip_03.DI_RoadDrip_03'");

  // Road lines
  DecalNamesMap.Add("roadline1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_01.DI_RoadLine_01'");
  DecalNamesMap.Add("roadline2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_02.DI_RoadLine_02'");
  DecalNamesMap.Add("roadline3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_03.DI_RoadLine_03'");
  DecalNamesMap.Add("roadline4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_04.DI_RoadLine_04'");
  DecalNamesMap.Add("roadline5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_RoadLine_05.DI_RoadLine_05'");

  // Tiremarks
  DecalNamesMap.Add("tiremark1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_TireMark_01.DI_TireMark_01'");
  DecalNamesMap.Add("tiremark2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_TireMark_02.DI_TireMark_02'");
  DecalNamesMap.Add("tiremark3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/RoadDirt/DI_TireMark_03.DI_TireMark_03'");

  // Tarsnakes
  DecalNamesMap.Add("tarsnake1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake1.DI_tarsnake1'");
  DecalNamesMap.Add("tarsnake2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake2.DI_tarsnake2'");
  DecalNamesMap.Add("tarsnake3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake3.DI_tarsnake3'");
  DecalNamesMap.Add("tarsnake4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake4.DI_tarsnake4'");
  DecalNamesMap.Add("tarsnake5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake5.DI_tarsnake5'");
  DecalNamesMap.Add("tarsnake6", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake6.DI_tarsnake6'");
  DecalNamesMap.Add("tarsnake7", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake7.DI_tarsnake7'");
  DecalNamesMap.Add("tarsnake8", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake8.DI_tarsnake8'");
  DecalNamesMap.Add("tarsnake9", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake9.DI_tarsnake9'");
  DecalNamesMap.Add("tarsnake10", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake10.DI_tarsnake10'");
  DecalNamesMap.Add("tarsnake11", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/TarSnakes/DI_tarsnake11.DI_tarsnake11'");

  // Cracks big
  DecalNamesMap.Add("cracksbig1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig1.DI_cracksBig1'");
  DecalNamesMap.Add("cracksbig2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig2.DI_cracksBig2'");
  DecalNamesMap.Add("cracksbig3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig3.DI_cracksBig3'");
  DecalNamesMap.Add("cracksbig4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig4.DI_cracksBig4'");
  DecalNamesMap.Add("cracksbig5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig5.DI_cracksBig5'");
  DecalNamesMap.Add("cracksbig6", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig6.DI_cracksBig6'");
  DecalNamesMap.Add("cracksbig7", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig7.DI_cracksBig7'");
  DecalNamesMap.Add("cracksbig8", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_cracksBig8.DI_cracksBig8'");

  // Cracks
  DecalNamesMap.Add("crack1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack01.DI_RoadCrack01'");
  DecalNamesMap.Add("crack2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack02.DI_RoadCrack02'");
  DecalNamesMap.Add("crack3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack05.DI_RoadCrack05'");
  DecalNamesMap.Add("crack4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack09.DI_RoadCrack09'");
  DecalNamesMap.Add("crack5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack11.DI_RoadCrack11'");
  DecalNamesMap.Add("crack6", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack13.DI_RoadCrack13'");
  DecalNamesMap.Add("crack7", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack15.DI_RoadCrack15'");
  DecalNamesMap.Add("crack8", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Cracks/DI_RoadCrack16.DI_RoadCrack16'");

  // Manholes
  DecalNamesMap.Add("manhole1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Manhole/DI_Manhole01.DI_Manhole01'");
  DecalNamesMap.Add("manhole2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Manhole/DI_Manhole02.DI_Manhole02'");
  DecalNamesMap.Add("manhole3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Manhole/DI_Manhole03.DI_Manhole03'");

  // Mud
  DecalNamesMap.Add("mud1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/MudDecal/DI_Mud_1.DI_Mud_1'");
  DecalNamesMap.Add("mud2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/MudDecal/DI_Mud_2.DI_Mud_2'");
  DecalNamesMap.Add("mud3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/MudDecal/DI_Mud_3.DI_Mud_3'");
  DecalNamesMap.Add("mud4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/MudDecal/DI_Mud_4.DI_Mud_4'");
  DecalNamesMap.Add("mud5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/MudDecal/DI_Mud_5.DI_Mud_5'");

  // Oil splats
  DecalNamesMap.Add("oilsplat1", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_OilSplat1.DI_OilSplat1'");
  DecalNamesMap.Add("oilsplat2", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_OilSplat2.DI_OilSplat2'");
  DecalNamesMap.Add("oilsplat3", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_OilSplat3.DI_OilSplat3'");
  DecalNamesMap.Add("oilsplat4", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_OilSplat4.DI_OilSplat4'");
  DecalNamesMap.Add("oilsplat5", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_OilSplat5.DI_OilSplat5'");

  // Misc
  DecalNamesMap.Add("gum", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/OilSplats/DI_Gum.DI_Gum'");
  DecalNamesMap.Add("grate", "MaterialInstanceConstant'/Game/Carla/Static/Decals/Road/Manhole/DI_Grate_01_v2.DI_Grate_01_v2'");

#endif
}

#if WITH_EDITORONLY_DATA

void ULoadAssetMaterialsCommandlet::ApplyRoadPainterMaterials(const FString &LoadedMapName, const FString &PackageName, bool IsInTiles)
{
  if (IsInTiles == true) {

    if (FilledData == false) {

      // As the OpenDrive file has the same name as level, build the path to the
      // xodr file using the label name and the game content directory.
      FString MapName = LoadedMapName;
      if (IsInTiles)
      {
        int32 idx = MapName.Find("_Tile_");
        if(idx > -1)
        {
          MapName = MapName.Mid(0, idx);
        }
      }
      const FString XodrContent = UOpenDrive::LoadXODR(MapName);
      XODRMap = carla::opendrive::OpenDriveParser::Load(carla::rpc::FromLongFString(XodrContent));

      // Acquire the TilesInfo.txt file for storing the tile data (offset and size)
      TArray<FString> FileList;
      IFileManager::Get().FindFilesRecursive(FileList, *(FPaths::ProjectContentDir() + "/" + PackageName + "/Maps/" + MapName), *(FString("TilesInfo.txt")), true, false, false);

      FString TxtFile;
      if (FFileHelper::LoadFileToString(TxtFile, *FileList[0]) == true) {

        TArray<FString> Out;
        TxtFile.ParseIntoArray(Out, TEXT(","), true);

        TileData.FirstTileCenterX = (float) FCString::Atoi(*Out[0]);
        TileData.FirstTileCenterY = (float) FCString::Atoi(*Out[1]);
        TileData.Size = (float) FCString::Atoi(*Out[2]);
      }
      else {
        UE_LOG(LogTemp, Warning, TEXT("Could not read TilesInfo.txt file"));
        return;
      }

      FilledData = true;
    }

    TArray<FString> StringArray = {};
    LoadedMapName.ParseIntoArray(StringArray, TEXT("_"), false);
    // From the loaded map name (in style mymap_Tile_200_400) get the X and the Y (Tile_200_400 -> X = 200, Y = 400)
    int32 XIndex = FCString::Atoi(*StringArray[StringArray.Num() - 2]);
    int32 YIndex = FCString::Atoi(*StringArray[StringArray.Num() - 1]);
    FVector TilePosition;
    // This means it's the initial tile (mymap_Tile_0_0)
    // This is in RELATIVE coords
    TilePosition.X = TileData.FirstTileCenterX + (TileData.Size * (float)XIndex);
    TilePosition.Y = TileData.FirstTileCenterY - (TileData.Size * (float)YIndex);
    TilePosition.Z = 0.0f;

    float HalfSize = TileData.Size / 2.0f;

    float MinXSize = (TilePosition.X - HalfSize);
    float MaxXSize = (TilePosition.X + HalfSize);

    float MinYSize = (TilePosition.Y - HalfSize);
    float MaxYSize = (TilePosition.Y + HalfSize);

    if (ReadConfigFile == true) {

      DecalsProperties = ReadDecalsConfigurationFile(PackageName);
      ReadConfigFile = false;
    }

    carla::geom::Location DecalLocation;

    //Store results for avoiding excessive calculations
    float MinXSizeCm = MinXSize * 100.0f;
    float MaxXSizeCm = MaxXSize * 100.0f;
    float MinYSizeCm = MinYSize * 100.0f;
    float MaxYSizeCm = MaxYSize * 100.0f;

    float TileSizeCm = TileData.Size * 100.0f;
    float FirstTileWorldLocationX = TileData.FirstTileCenterX * 100.0f;
    float FirstTileWorldLocationY = TileData.FirstTileCenterY * 100.0f;

    float CenterOfTileX = FirstTileWorldLocationX + (XIndex * TileSizeCm);
    float CenterOfTileY = FirstTileWorldLocationY - (YIndex * TileSizeCm);

    for (int32 i = 0; i < DecalsProperties.DecalMaterials.Num(); ++i) {

      for (int32 j = 0; j < DecalsProperties.DecalNumToSpawn[i]; ++j) {

        DecalLocation.x = FMath::RandRange(MinXSize, MaxXSize);
        DecalLocation.y = FMath::RandRange(MinYSize, MaxYSize);
        DecalLocation.z = 0.0f;
  
        // Get the closest road waypoint from the random location calculated
        auto Wp = XODRMap->GetClosestWaypointOnRoad(DecalLocation);
        carla::geom::Location RoadLocation = XODRMap->ComputeTransform(Wp.get()).location;
        FVector FinalLocation(RoadLocation);

        // Check we don't exceed the map boundaries
        if (FinalLocation.X > MinXSizeCm && FinalLocation.X < MaxXSizeCm) {

          if (FinalLocation.Y > MinYSizeCm && FinalLocation.Y < MaxYSizeCm) {

            FRotator FinalRotation(XODRMap->ComputeTransform(Wp.get()).rotation);

            // Transform the location from world coords to tile coordinates.
            // The location we get is the location of the XODR waypoint, which is in WORLD coordinates
            // The Y coordinates are reversed! -Y = Y and Y = -Y

            FinalLocation.X -= CenterOfTileX;
            FinalLocation.Y -= CenterOfTileY;

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            SpawnParams.bNoFail = true;
            ADecalActor* Decal = World->SpawnActor<ADecalActor>(FinalLocation, FRotator(), SpawnParams);
            Decal->SetDecalMaterial(DecalsProperties.DecalMaterials[i]);

            // Calculate random scale for decal
            float RandScale = FMath::RandRange(DecalsProperties.DecalMinScale, DecalsProperties.DecalMaxScale);
            FVector FinalDecalScale;
            FinalDecalScale.X = DecalsProperties.DecalScale.X + RandScale;
            FinalDecalScale.Y = DecalsProperties.DecalScale.Y + RandScale;
            FinalDecalScale.Z = 1.0f;
            Decal->SetActorScale3D(FinalDecalScale);

            // Calculate random yaw for decal
            float RandomYaw = FMath::RandRange(0.0f, DecalsProperties.DecalRandomYaw);
            FinalRotation.Yaw += RandomYaw;
            FinalRotation.Pitch = -90.0f;
            Decal->SetActorRotation(FinalRotation);
            Decal->SetActorLabel("RoadDecal", true);
          }
        }
      }
    }

#if WITH_EDITOR
  UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
#endif
  }
}

FDecalsProperties ULoadAssetMaterialsCommandlet::ReadDecalsConfigurationFile(const FString &PackageName) {

  // Get road painter configuration file
  FString JsonConfigFile;

  TArray<FString> FileList;
  IFileManager::Get().FindFilesRecursive(FileList, *(FPaths::ProjectContentDir() + "/" + PackageName),
    *(FString("roadpainter_decals.json")), true, false, false);

  FDecalsProperties DecalConfiguration;
  if (FFileHelper::LoadFileToString(JsonConfigFile, *(IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FileList[0]))))
  {
    TSharedPtr<FJsonObject> JsonParsed;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonConfigFile);
    if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
    {
      // Get decals object array
      auto DecalJsonArray = JsonParsed->GetArrayField(TEXT("decals"));
      for (auto &DecalJsonValue : DecalJsonArray)
      {
        const auto DecalJsonObject = DecalJsonValue->AsObject();

        // With the decal name array we created earlier, we traverse it 
        // and look up it's name in the .json file
        for (const TPair<FString, FString>& Pair : DecalNamesMap) {
          if (DecalJsonObject->HasField(Pair.Key) == true) {
            DecalConfiguration.DecalMaterials.Add(LoadObject<UMaterialInstanceConstant>(nullptr, *Pair.Value));
            DecalConfiguration.DecalNumToSpawn.Add(DecalJsonObject->GetIntegerField(Pair.Key));
          }
        }

        TSharedPtr<FJsonObject> VectorObject = DecalJsonObject->GetObjectField(TEXT("decal_scale"));
        FVector DecalScale;
        DecalScale.X = (float)VectorObject->GetNumberField(TEXT("x_axis"));
        DecalScale.Y = (float)VectorObject->GetNumberField(TEXT("y_axis"));
        DecalScale.Z = (float)VectorObject->GetNumberField(TEXT("z_axis"));
        DecalConfiguration.DecalScale = DecalScale;
        DecalConfiguration.FixedDecalOffset = DecalScale;
        DecalConfiguration.DecalMinScale = (float)DecalJsonObject->GetNumberField(TEXT("decal_min_scale"));
        DecalConfiguration.DecalMaxScale = (float)DecalJsonObject->GetNumberField(TEXT("decal_max_scale"));
        DecalConfiguration.DecalRandomYaw = (float)DecalJsonObject->GetNumberField(TEXT("decal_random_yaw"));
        DecalConfiguration.RandomOffset = (float)DecalJsonObject->GetNumberField(TEXT("random_offset"));
      }
    }
  }
  return DecalConfiguration;
}

FString ULoadAssetMaterialsCommandlet::GetFirstPackagePath(const FString &PackageName) const
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

void ULoadAssetMaterialsCommandlet::LoadAssetsMaterials(const FString &PackageName, const TArray<FMapData> &MapsPaths)
{
  // Load World
  FAssetData AssetData;
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  const FString DefaultPath = TEXT("/Game/") + PackageName + TEXT("/Maps/");
  MapObjectLibrary->AddToRoot();
  for (auto &&data : MapsPaths)
  {
    MapObjectLibrary->LoadAssetDataFromPath(*data.Path);
  }
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetDatas);

  if (AssetDatas.Num() > 0)
  {
    int32 NumAssets = AssetDatas.Num();
    //If the map is tiled, there will be several umaps in the same folder
    for (int32 i = 0; i < NumAssets; ++i) {

      // Extract first asset found in folder path (i.e. the imported map)
      AssetData = AssetDatas.Pop();
      World = Cast<UWorld>(AssetData.GetAsset());
      if (World != nullptr) {

        World->InitWorld();

        // Check if there's any road meshes in the world for spawning the decals.
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), FoundActors);
        bool HasRoadMesh = false;
        for (int32 j = 0; j < FoundActors.Num() && HasRoadMesh == false; ++j) {

          AStaticMeshActor *MeshActor = Cast<AStaticMeshActor>(FoundActors[j]);
          if (MeshActor->GetName().Contains("Roads") || MeshActor->GetName().Contains("Road")){

            HasRoadMesh = true;
          }
        }

        if (HasRoadMesh == true) {

          bool IsTiledMap = World->GetName().Contains("_Tile_", ESearchCase::Type::CaseSensitive);
          ApplyRoadPainterMaterials(World->GetName(), PackageName, IsTiledMap);
        }

#if WITH_EDITOR
        UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
#endif
      }
    }
  }
}

FPackageParams ULoadAssetMaterialsCommandlet::ParseParams(const FString &InParams) const
{
  TArray<FString> Tokens;
  TArray<FString> Params;
  TMap<FString, FString> ParamVals;

  ParseCommandLine(*InParams, Tokens, Params);

  FPackageParams PackageParams;

  // Parse and store Package name
  FParse::Value(*InParams, TEXT("PackageName="), PackageParams.Name);

  return PackageParams;
}

FAssetsPaths ULoadAssetMaterialsCommandlet::GetAssetsPathFromPackage(const FString &PackageName) const
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

int32 ULoadAssetMaterialsCommandlet::Main(const FString &Params)
{
  FPackageParams PackageParams = ParseParams(Params);

  // Get Props and Maps Path
  FAssetsPaths AssetsPaths = GetAssetsPathFromPackage(PackageParams.Name);

  LoadAssetsMaterials(PackageParams.Name, AssetsPaths.MapsPaths);

#if WITH_EDITOR
  UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
#endif

  return 0;
}

#endif
