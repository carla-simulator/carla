// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapGeneratorWidget.h"

#include "ActorFactories/ActorFactory.h"
#include "AssetRegistryModule.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Components/SplineComponent.h"
#include "Editor/FoliageEdit/Public/FoliageEdMode.h"
#include "EditorLevelLibrary.h"
#include "FileHelpers.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "ProceduralFoliageComponent.h"
#include "ProceduralFoliageVolume.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"

#include "EditorAssetLibrary.h"
#include "EngineUtils.h"
#include "ObjectEditorUtils.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ObjectMacros.h"

#define CUR_CLASS_FUNC (FString(__FUNCTION__))
#define CUR_LINE  (FString::FromInt(__LINE__))
#define CUR_CLASS_FUNC_LINE (CUR_CLASS_FUNC + "::" + CUR_LINE)

#undef CreateDirectory
#undef CopyFile

DEFINE_LOG_CATEGORY(LogCarlaToolsMapGenerator);

void UMapGeneratorWidget::GenerateMapFiles(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Map Generation %s %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.DestinationPath, *MetaInfo.MapName);

  // // 1. Creating tiles terrain
  bool bTilesSuccess = CreateTilesMaps(MetaInfo);
  if(!bTilesSuccess)
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Creating Tile for %s"), 
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  // 2. Create Main Large map
  bool bLargeMapSuccess = CreateMainLargeMap(MetaInfo);
  if(!bLargeMapSuccess)
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error creating Main Large Map for %s"), 
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  // 3. Opendrive auxiliary file
  bool bOpenDriveCopySuccess = CreateOpenDriveFile(MetaInfo);
  if(!bOpenDriveCopySuccess)
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error creating OpenDrive file for %s"), 
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  
}

void UMapGeneratorWidget::CookVegetation(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Cooking Vegetation to Tiles in %s %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.DestinationPath, *MetaInfo.MapName);
  // 3. Add vegetation to tiles
  bool bVegetationSuccess = CookVegetationToTiles(MetaInfo);
  if(!bVegetationSuccess){
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Cooking vegetation for %s"), 
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: SUCCESS Cooking Vegetation to Tiles in %s %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.DestinationPath, *MetaInfo.MapName);
  }
}

void UMapGeneratorWidget::CookVegetationToCurrentTile(const TArray<UProceduralFoliageSpawner*> FoliageSpawners)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, 
      TEXT("%s: Cooking vegetation to current tile. Vegetation type num: %d"), 
      *CUR_CLASS_FUNC_LINE, FoliageSpawners.Num());

  bool result = CookVegetationToWorld(GEditor->GetEditorWorldContext().World(), FoliageSpawners);
  if(!result)
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Cooking vegetation for Current Tile"), 
        *CUR_CLASS_FUNC_LINE);
}

FString UMapGeneratorWidget::SanitizeDirectory(FString InDirectory)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Sanitazing directory: %s"), 
        *CUR_CLASS_FUNC_LINE, *InDirectory);
  // Check that the directory in metaInfo cannot finish in /, if does, delete it
  while(InDirectory.EndsWith("/") || InDirectory.EndsWith("\\"))
  {
    if(InDirectory.EndsWith("/"))
    {
      InDirectory.RemoveFromEnd("/");
    }
    else if(InDirectory.EndsWith("\\"))
    {
      InDirectory.RemoveFromEnd("\\");
    }
  }
  return InDirectory;
}

bool UMapGeneratorWidget::LoadMapInfoFromPath(FString InDirectory, int& OutMapSize, FString& OutFoundMapName)
{
  TArray<FAssetData> DirectoryAssets;
  bool bLoaded = LoadWorlds(DirectoryAssets, InDirectory, false);

  FString MainMapName;
  FString MapNameInTiles;

  if(bLoaded && DirectoryAssets.Num() > 0)
  {
    for(FAssetData AssetData : DirectoryAssets) // Find name in tiles
    {
      const FString AssetName = AssetData.AssetName.ToString();
      FString Name, Coordinates;
      if(AssetName.Split(TEXT("_Tile_"), &Name, &Coordinates))
      {
        MapNameInTiles = Name;
        break;
      }
    }

    if(MapNameInTiles.IsEmpty()) // No tiles found
    {
      return false;
    }

    for(FAssetData AssetData : DirectoryAssets)
    {
      const FString AssetName = AssetData.AssetName.ToString();
      if(!AssetName.Contains("_Tile_") && AssetName == MapNameInTiles) // Find Main Map
      {
        MainMapName = AssetName;
        break;
      }
    }

    if(MainMapName.IsEmpty()) // No main map found
    {
      return false;
    }

    int NumberOfTiles = DirectoryAssets.Num() - 1;
    OutFoundMapName = MainMapName;
    OutMapSize = (int)UKismetMathLibrary::Sqrt((float)NumberOfTiles);
  }
  else
  {
    // No Map Found
    return false;
  }

  return true;
}

AActor* UMapGeneratorWidget::GenerateWater(TSubclassOf<class AActor> RiverClass)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Generating Waterbodies"), 
        *CUR_CLASS_FUNC_LINE);

  UWorld* World = GetWorld();

  ///////********** TODO PROV ***********//////
  ALandscape* Landscape = (ALandscape*) UGameplayStatics::GetActorOfClass(
      World, 
      ALandscape::StaticClass());
  LandscapePostEditEvent(Landscape);

  //////*****************************/////

  float ActorZCoord = GetLandscapeSurfaceHeight(World, 0, 0, false);
  FVector Location(20000, 20000, ActorZCoord); // Auxiliar values for x and y coords
  FRotator Rotation(0,0,0);
  FActorSpawnParameters SpawnInfo;
  
  
  AActor* RiverActor =  World->SpawnActor<AActor>(
      RiverClass, 
      Location, 
      Rotation, 
      SpawnInfo);

  USplineComponent* RiverSpline = RiverActor->FindComponentByClass<USplineComponent>();

  int NumberOfPoints = RiverSpline->GetNumberOfSplinePoints();

  for(int i = 0; i < NumberOfPoints; i++)
  {
    FVector PointPosition = RiverSpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
    PointPosition.Z = GetLandscapeSurfaceHeight(World, PointPosition.X, PointPosition.Y, false);
    RiverSpline->SetLocationAtSplinePoint(i, PointPosition, ESplineCoordinateSpace::World, true);
  }

  return RiverActor;
}

AActor* UMapGeneratorWidget::AddWeatherToExistingMap(TSubclassOf<class AActor> WeatherActorClass, 
    const FMapGeneratorMetaInfo& MetaInfo, const FString SelectedWeather)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Generating Weather to %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  // Check if map is valid
  const FString MapCompletePath = MetaInfo.DestinationPath + "/" + MetaInfo.MapName;
  const FString MapPackageFileName = FPackageName::LongPackageNameToFilename(
      MapCompletePath, 
      FPackageName::GetMapPackageExtension());
    
  if(!FPaths::FileExists(*MapPackageFileName))
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Weather cannot be applied to a non existing map"), 
        *CUR_CLASS_FUNC_LINE);
    return nullptr;
  }

  // Instantiate Weather Actor in main map
  const FString WorldToLoadPath = MapCompletePath + "." + MetaInfo.MapName;
  UWorld* World = LoadObject<UWorld>(nullptr, *WorldToLoadPath);

  AActor* WeatherActor = World->SpawnActor<AActor>(WeatherActorClass);

  return WeatherActor;  

}

void UMapGeneratorWidget::LandscapePostEditEvent(ALandscape* Landscape)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Landscape Post Edit Event Called"), 
      *CUR_CLASS_FUNC_LINE);
  
  // FProperty* PropertyToUpdate = Landscape->GetClass()->FindPropertyByName(TEXT("CollisionMipLevel"));
  // if(!PropertyToUpdate){
  //   UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Could not found the specified property"), 
  //     *CUR_CLASS_FUNC_LINE);
  // }
  // FPropertyChangedEvent LandscapeCustomPropertyChangedEvent(PropertyToUpdate, EPropertyChangeType::Interactive);
  // Landscape->PostEditChangeProperty(LandscapeCustomPropertyChangedEvent);

  // static const FName PropertyName(TEXT("CollisionMipLevel"));
  // FObjectEditorUtils::SetPropertyValue<ALandscape, int32>(Landscape, PropertyName, 1);


  // Post edit event
  // FProperty* PropertyToUpdate = Landscape->GetClass()->FindPropertyByName(TEXT("CollisionMipLevel"));
  // int32* PropertyValue = PropertyToUpdate->ContainerPtrToValuePtr<int32>(Landscape);
  // if(!PropertyToUpdate){
  //   UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Could not found the specified property"), 
  //     *CUR_CLASS_FUNC_LINE);
  //     return;
  // }

  // *PropertyValue += 1; // Increment

  // FPropertyChangedEvent LandscapeCustomPropertyChangedEvent(PropertyToUpdate);
  // Landscape->PostEditChangeProperty(LandscapeCustomPropertyChangedEvent);
  // Landscape->PostEditChange();


  // Recreate Collision Components
  // Landscape->PostDuplicate(false);
  // Landscape->PostDuplicate(EDuplicateMode::Type::World);
  Landscape->RecreateCollisionComponents();


}

bool UMapGeneratorWidget::LoadBaseTileWorld(FAssetData& WorldAssetData)
{
  const FString BaseMapPath = TEXT("/CarlaTools/MapGenerator/BaseMap/Tiles");
  const FString MapName = TEXT("MapGeneratorBaseMap");
  // return LoadWorldByName(WorldAssetData, BaseMapPath, MapName);
  return LoadWorld(WorldAssetData, BaseMapPath);
}

bool UMapGeneratorWidget::LoadBaseLargeMapWorld(FAssetData& WorldAssetData)
{
  const FString BaseMapPath= TEXT("/CarlaTools/MapGenerator/BaseMap/MainLargeMap");
  return LoadWorld(WorldAssetData, BaseMapPath);
}

bool UMapGeneratorWidget::LoadWorld(FAssetData& WorldAssetData, const FString& BaseMapPath)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Loading World from %s"), 
      *CUR_CLASS_FUNC_LINE, *BaseMapPath);

  TArray<FAssetData> AssetsData;
  bool success = LoadWorlds(AssetsData, BaseMapPath);

  if(success && AssetsData.Num() > 0)
  {
    WorldAssetData = AssetsData.Pop();
    return true; 
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error getting worlds from %s"), 
      *CUR_CLASS_FUNC_LINE, *BaseMapPath);
    return false;
  }
}

bool UMapGeneratorWidget::LoadWorldByName(FAssetData& WorldAssetData, const FString& BaseMapPath, const FString& MapName)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Loading World named %s from %s"), 
      *CUR_CLASS_FUNC_LINE, *MapName, *BaseMapPath);

  // Loading all and search for the matching name 
  TArray<FAssetData> AssetsData;
  bool bSuccess = LoadWorlds(AssetsData, BaseMapPath);

  if(bSuccess && AssetsData.Num() > 0)
  {
    for(FAssetData AssetData : AssetsData)
    {
      if(MapName.Equals(AssetData.AssetName.ToString(), ESearchCase::CaseSensitive))
      {
        WorldAssetData = AssetData;
        UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: World is load  %s"), 
          *CUR_CLASS_FUNC_LINE, WorldAssetData.IsAssetLoaded() ? TEXT("YES") : TEXT("NO"));
        return true;
      }
    }
    return false; // Asset not found
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error getting worlds from %s"), 
      *CUR_CLASS_FUNC_LINE, *BaseMapPath);
    return false;
  }
}

bool UMapGeneratorWidget::DUBUG_LandscapeApplyHeightmap(const FMapGeneratorMetaInfo& MetaInfo)
{
  // for(int i = 0; i < MetaInfo.SizeX; i++)
  // {
  //   for(int j = 0; j < MetaInfo.SizeY; j++)
  //   {
  //     const FString MapName = 
  //           MetaInfo.MapName + "_Tile_" + FString::FromInt(i) + "_" + FString::FromInt(j);
  //     const FString WorldSearchPath = MetaInfo.DestinationPath + "/" + MapName + "." + MapName;

  //     UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Heightmap to %s"), 
  //         *CUR_CLASS_FUNC_LINE, *WorldSearchPath);

  //     UWorld* PreWorld = LoadObject<UWorld>(nullptr, *WorldSearchPath);
  //     if(PreWorld == nullptr){
  //       UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Error Finding pre-world in: %s"), 
  //           *CUR_CLASS_FUNC_LINE, *WorldSearchPath);
  //           return false;
  //     }

  //     UPackage* ReloadedPackage = ReloadPackage(PreWorld->GetPackage(), ELoadFlags::LOAD_EditorOnly);
      
  //     UWorld* World = LoadObject<UWorld>(ReloadedPackage, *WorldSearchPath);
  //     if(World == nullptr){
  //       UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Error Finding world in: %s"), 
  //           *CUR_CLASS_FUNC_LINE, *WorldSearchPath);
  //           return false;
  //     }

  //     // Actor count
  //     TArray<AActor*> ActorsInWorld;
  //     UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), ActorsInWorld);

  //     ALandscape* Landscape = (ALandscape*) UGameplayStatics::GetActorOfClass(
  //         World, 
  //         ALandscape::StaticClass());
  //     FMapGeneratorTileMetaInfo MetaTileInfo;
  //     MetaTileInfo.IndexX = i;
  //     MetaTileInfo.IndexY = j;
  //     if (Landscape == nullptr)
  //           UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error. No landscape found in tile %s %d_%d ------- Actors found: %d"), 
  //               *CUR_CLASS_FUNC_LINE, *World->GetName(), MetaTileInfo.IndexX, MetaTileInfo.IndexY, ActorsInWorld.Num());

  //     // Landscape->ComponentSizeQuads = 126;
  //     // Landscape->SubsectionSizeQuads = 63;
  //     // Landscape->NumSubsections = 4;
  //     // UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Landscape info %d %d %d"), 
  //     //       *CUR_CLASS_FUNC_LINE, Landscape->ComponentSizeQuads, Landscape->SubsectionSizeQuads, Landscape->NumSubsections);

  //     AssignLandscapeHeightMap(Landscape, MetaTileInfo);

  //     const FString PackageName = MetaInfo.DestinationPath + "/" + MapName;
  //     const FString PackageFileName = FPackageName::LongPackageNameToFilename(
  //         PackageName, 
  //         FPackageName::GetMapPackageExtension());
  //     UPackage::SavePackage(World->GetPackage(), World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
  //         *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
  //   }
  // }

      const FString MapName = MetaInfo.MapName;
      const FString WorldSearchPath = MetaInfo.DestinationPath + "/" + MapName + "." + MapName;

      UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Heightmap to %s"), 
          *CUR_CLASS_FUNC_LINE, *WorldSearchPath);

      UWorld* PreWorld = LoadObject<UWorld>(nullptr, *WorldSearchPath);
      if(PreWorld == nullptr){
        UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Error Finding pre-world in: %s"), 
            *CUR_CLASS_FUNC_LINE, *WorldSearchPath);
            return false;
      }

      UPackage* ReloadedPackage = ReloadPackage(PreWorld->GetPackage(), ELoadFlags::LOAD_EditorOnly);
      
      UWorld* World = LoadObject<UWorld>(ReloadedPackage, *WorldSearchPath);
      if(World == nullptr){
        UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Error Finding world in: %s"), 
            *CUR_CLASS_FUNC_LINE, *WorldSearchPath);
            return false;
      }

      // Actor count
      TArray<AActor*> ActorsInWorld;
      UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), ActorsInWorld);

      ALandscape* Landscape = (ALandscape*) UGameplayStatics::GetActorOfClass(
          World, 
          ALandscape::StaticClass());
      FMapGeneratorTileMetaInfo MetaTileInfo;
      // MetaTileInfo.IndexX = i;
      // MetaTileInfo.IndexY = j;
      if (Landscape == nullptr)
            UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error. No landscape found in tile %s %d_%d ------- Actors found: %d"), 
                *CUR_CLASS_FUNC_LINE, *World->GetName(), MetaTileInfo.IndexX, MetaTileInfo.IndexY, ActorsInWorld.Num());

      AssignLandscapeHeightMap(Landscape, MetaTileInfo);

      const FString PackageName = MetaInfo.DestinationPath + "/" + MapName;
      const FString PackageFileName = FPackageName::LongPackageNameToFilename(
          PackageName, 
          FPackageName::GetMapPackageExtension());
      UPackage::SavePackage(World->GetPackage(), World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
          *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

  return true;
}


bool UMapGeneratorWidget::LoadWorlds(TArray<FAssetData>& WorldAssetsData, const FString& BaseMapPath, bool bRecursive)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Loading Worlds from %s"), 
      *CUR_CLASS_FUNC_LINE, *BaseMapPath);

  TArray<FAssetData> AssetsData;
  UObjectLibrary *MapObjectLibrary;

  // Loading Map from folder using object library
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
  MapObjectLibrary->bRecursivePaths = bRecursive;
  MapObjectLibrary->AddToRoot();
  MapObjectLibrary->LoadAssetDataFromPath(*BaseMapPath);
  MapObjectLibrary->LoadAssetsFromAssetData();
  MapObjectLibrary->GetAssetDataList(AssetsData);

  if (AssetsData.Num() > 0)
  {
    // Return whole list of world assets found in directory
    WorldAssetsData = AssetsData;
    return true;
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: No Worlds found in %s"), 
      *CUR_CLASS_FUNC_LINE, *BaseMapPath);
    return false;
  }
}

bool UMapGeneratorWidget::SaveWorld(
    FAssetData& WorldToBeSaved, 
    const FString& DestinationPath, 
    const FString& WorldName,
    bool bCheckFileExists)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Saving World to %s as %s"), 
      *CUR_CLASS_FUNC_LINE, *DestinationPath, *WorldName);

  UWorld* World = GetWorldFromAssetData(WorldToBeSaved);

  const FString PackagePath = DestinationPath + "/" + WorldName;

  // Create Package
  UPackage *Package = WorldToBeSaved.GetPackage();
  Package->SetFolderName(*WorldName);
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(World);
  
  // Rename new World
  World->Rename(*WorldName, World->GetOuter());
  
  FAssetRegistryModule::AssetRenamed(World, *PackagePath);
  World->MarkPackageDirty();
  World->GetOuter()->MarkPackageDirty();

  // Saving Package
  const FString PackageFileName = FPackageName::LongPackageNameToFilename(
    PackagePath, 
    FPackageName::GetMapPackageExtension());

  if(bCheckFileExists && FPaths::FileExists(*PackageFileName))
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, 
        TEXT("%s: Could not save %s because it already exists."),
        *PackageFileName); 
    return false;
  }
  return UPackage::SavePackage(
      Package, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
}

bool UMapGeneratorWidget::CreateMainLargeMap(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating %s main large map in %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName, *MetaInfo.DestinationPath);
  FAssetData WorldAssetData;

  const FString BaseWorldSearchPath = TEXT("/CarlaTools/MapGenerator/BaseMap/MainLargeMap/MapGeneratorBaseLargeMap.MapGeneratorBaseLargeMap");
  UWorld* BaseWorld = LoadObject<UWorld>(nullptr, *BaseWorldSearchPath);
  const FString PackageName = MetaInfo.DestinationPath + "/" + MetaInfo.MapName;

  UPackage* BaseMapPackage = CreatePackage(*PackageName);
  FObjectDuplicationParameters Parameters(BaseWorld, BaseMapPackage);
  Parameters.DestName = FName(*MetaInfo.MapName);
  Parameters.DestClass = BaseWorld->GetClass();
  Parameters.DuplicateMode = EDuplicateMode::World;
  Parameters.PortFlags = PPF_Duplicate;

  UWorld* World = CastChecked<UWorld>(StaticDuplicateObjectEx(Parameters));

  const FString PackageFileName = FPackageName::LongPackageNameToFilename(
      PackageName, 
      FPackageName::GetMapPackageExtension());
  UPackage::SavePackage(BaseMapPackage, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);


  bool bLoadedSuccess = FEditorFileUtils::LoadMap(*PackageName, false, true);
    if(!bLoadedSuccess){
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Loading %s"),
          *CUR_CLASS_FUNC_LINE, *PackageName);
      return false;
    }

  AActor* LargeMapManagerActor = UGameplayStatics::GetActorOfClass(World, ALargeMapManager::StaticClass());
  if(LargeMapManagerActor == nullptr)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, 
        TEXT("%s: Could not find LargeMapManager Actor in %s."),
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName); 
    return false;
  }

  ALargeMapManager* LargeMapManager = StaticCast<ALargeMapManager*>(LargeMapManagerActor);
  if(LargeMapManager == nullptr)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, 
        TEXT("%s: Failed to cast Large Map Actor in %s."),
        *MetaInfo.MapName); 
    return false;
  }

  LargeMapManager->LargeMapTilePath = MetaInfo.DestinationPath;
  LargeMapManager->LargeMapName = MetaInfo.MapName;
  // LargeMapManager->AssetsPath = MetaInfo.DestinationPath;
  // LargeMapManager->TileSide = 2.0f * 1000.0f * 100.0f; 
  LargeMapManager->GenerateMap_Editor();

  UPackage::SavePackage(BaseMapPackage, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
      *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

  return true;
}

bool UMapGeneratorWidget::CreateOpenDriveFile(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Warning,
      TEXT("%s: Creating OpenDrive file for %s. This is needed but deprecated for offroad maps."), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);
  
  FString PrunedPath = MetaInfo.DestinationPath.Replace(TEXT("/Game/"), TEXT(""));
  const FString DestinationPath = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectContentDir().Append(PrunedPath + "/OpenDrive/"));
  const FString DestinationPathWithFile = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectContentDir().Append(PrunedPath + "/OpenDrive/" + MetaInfo.MapName + ".xodr"));
  const FString SourcePath = FPaths::ProjectPluginsDir()
      .Append(TEXT("CarlaTools/Content/MapGenerator/Misc/OpenDrive/TemplateOpenDrive.xodr"));
  ;

  IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

  return FileManager.CreateDirectory(*DestinationPath)
      && FileManager.CopyFile(*DestinationPathWithFile,*SourcePath, EPlatformFileRead::None, EPlatformFileWrite::None);
}

bool UMapGeneratorWidget::CreateTilesMaps(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating %s tiles maps in %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName, *MetaInfo.DestinationPath);

   const FString BaseWorldSearchPath = TEXT("/CarlaTools/MapGenerator/BaseMap/TilesEmpty/BaseTileEmpty.BaseTileEmpty");

   UWorld* BaseWorld;
   UWorld* World;


// CREATION OF TILES USING FASSETDATA 
  for(int i = 0; i < MetaInfo.SizeX; i++)
  {
    for(int j = 0; j < MetaInfo.SizeY; j++)
    {
      UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating tile map %s (%d_%d)"), 
          *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName, i, j);

      BaseWorld = LoadObject<UWorld>(nullptr, *BaseWorldSearchPath);

      const FString MapName = 
          MetaInfo.MapName + "_Tile_" + FString::FromInt(i) + "_" + FString::FromInt(j);  
      const FString PackageName = MetaInfo.DestinationPath + "/" + MapName;

      UPackage* TilePackage = CreatePackage(*PackageName);
      FObjectDuplicationParameters Parameters(BaseWorld, TilePackage);
      Parameters.DestName = FName(*MapName);
      Parameters.DestClass = BaseWorld->GetClass();
      Parameters.DuplicateMode = EDuplicateMode::World;
      Parameters.PortFlags = PPF_Duplicate;
      // Parameters.PortFlags = PPF_DuplicateForPIE;

      World = CastChecked<UWorld>(StaticDuplicateObjectEx(Parameters));

      if (World == nullptr)
        UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error. No world found in tile %s %d_%d"), 
            *CUR_CLASS_FUNC_LINE, *World->GetName(), i, j);

      // 4033 x 4033
      ALandscape* Landscape = World->SpawnActor<ALandscape>();
      Landscape->ComponentSizeQuads = 126; // Component Size
      Landscape->SubsectionSizeQuads = 63; // Quads / Section
      Landscape->NumSubsections = 2;       // (1 for 1x1 , 2 for 2x2)
      Landscape->SetLandscapeGuid(FGuid::NewGuid());

      // 2017 x 2017
      // ALandscape* Landscape = World->SpawnActor<ALandscape>();
      // Landscape->ComponentSizeQuads = 63;
      // Landscape->SubsectionSizeQuads = 63;
      // Landscape->NumSubsections = 1;
      // Landscape->SetLandscapeGuid(FGuid::NewGuid());

      // Landscape Material

      // Height Render Target
      UTextureRenderTarget2D* HeightRT = MetaInfo.GlobalHeightmap;
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Heightmap detected with dimensions %dx%d"), 
            *CUR_CLASS_FUNC_LINE, HeightRT->SizeX, HeightRT->SizeY);
      TArray<uint16> HeightData;
      // FTextureRenderTargetResource* RenderTargetResource = HeightRT->GetRenderTargetResource();
      // TODO: UTexture2D and GetMipData
      FTextureRenderTargetResource* RenderTargetResource = HeightRT->GameThread_GetRenderTargetResource();
      FIntRect Rect = FIntRect(0, 0, HeightRT->SizeX, HeightRT->SizeY);
      // FIntRect Rect = FIntRect(0, 0, FMath::Min(1 + MaxX - MinX, HeightRT->SizeX), FMath::Min(1 + MaxY - MinY, HeightRT->SizeY));
      TArray<FLinearColor> HeightmapColor;
      ////
      // TArray<uint16> HeightData;
      // HeightData.Init(0, 2017*2017);
      // HeightmapColor.Reserve(2017*2017);
      ////
      HeightmapColor.Reserve(Rect.Width() * Rect.Height());
      RenderTargetResource->ReadLinearColorPixels(HeightmapColor, FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX), Rect);
      HeightData.Reserve(HeightmapColor.Num());

      // int IterCount = 0;
      // float R = 1.0f;
      // float G = 1.0f;
      for(FLinearColor LinearColor : HeightmapColor)
      {
      //   if(IterCount % HeightRT->SizeX == 0)
      //   {
      //     R -= 0.001f;
      //     // G -= 0.01f;
      //   }
      //   HeightData.Add((uint16)(R * 255 * 255 + G * 255));
      //   IterCount++;
        HeightData.Add((uint16)(LinearColor.R * 255 * 255 + LinearColor.G * 255));
      //   // // HeightData.Add((uint16)(LinearColor.R * 255 + LinearColor.G));
      //   // // HeightData.Add(((uint16)LinearColor.R << 8) | (uint16)LinearColor.G);
      //   // // HeightData.Add((uint16)LinearColor.R);
      //   // // if(IterCount % 10 == 0)
      //   // //  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Color: %d"), 
      //   // //     *CUR_CLASS_FUNC_LINE, HeightData.Last());
      }

      // HeightmapGenerator HMGenerator();
      // HeightData = UHeightmapGenerator::GenerateHeightmap(Rect.Width());
      

      // FVector LandscapeScaleVector(100.0f, 100.0f, 100.0f*255);
      FVector LandscapeScaleVector(100.0f, 100.0f, 100.0f);
      // FVector LandscapeScaleVector(1.0f, 1.0f, 1.0f);
      Landscape->CreateLandscapeInfo();
      // Landscape->SetActorTransform(FTransform(FQuat::Identity, FVector::ZeroVector, FVector::OneVector));
      Landscape->SetActorTransform(FTransform(FQuat::Identity, FVector(), LandscapeScaleVector));

      TMap<FGuid, TArray<uint16>> HeightmapDataPerLayers;
	    TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;

      HeightmapDataPerLayers.Add(FGuid(), HeightData);
      MaterialLayerDataPerLayer.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());
	
      Landscape->Import(Landscape->GetLandscapeGuid(), 0, 0, HeightRT->SizeX-1, HeightRT->SizeY-1, Landscape->NumSubsections, Landscape->SubsectionSizeQuads,
          HeightmapDataPerLayers, TEXT("NONE"), MaterialLayerDataPerLayer, ELandscapeImportAlphamapType::Layered);
      // Landscape->Import(Landscape->GetLandscapeGuid(), 0, 0, 2016, 2016, Landscape->NumSubsections, Landscape->SubsectionSizeQuads,
      //     HeightmapDataPerLayers, TEXT("NONE"), MaterialLayerDataPerLayer, ELandscapeImportAlphamapType::Layered);


      // FVector LandscapeOrigin;
      // FVector LandscapeBounds;
      // Landscape->GetActorBounds(false, LandscapeOrigin, LandscapeBounds);
      // Landscape->SetActorLocation(FVector(0.0f, 0.0f, LandscapeBounds.Z));

      Landscape->ReregisterAllComponents();

      Landscape->CreateLandscapeInfo();
      Landscape->SetActorLabel("Landscape");

      // Apply material
      AssignLandscapeMaterial(Landscape);

      const FString PackageFileName = FPackageName::LongPackageNameToFilename(
          PackageName, 
          FPackageName::GetMapPackageExtension());
      UPackage::SavePackage(TilePackage, World, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
          *PackageFileName, GError, nullptr, true, true, SAVE_NoError);

      // TODO PROV
      FText ErrorUnloadingStr;
      bool bUnload = FEditorFileUtils::AttemptUnloadInactiveWorldPackage(BaseWorld->GetPackage(),ErrorUnloadingStr);
      if(!bUnload){
        UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Error unloading Base map: %s"), 
            *CUR_CLASS_FUNC_LINE, *ErrorUnloadingStr.ToString());
        return false;
      }
      
    }
  }

  return true;
}

bool UMapGeneratorWidget::CookVegetationToTiles(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Cooking vegetation to %s tiles"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  // First check if there is elements in MetaInfo.FoliageSpawners
  if(MetaInfo.FoliageSpawners.Num() == 0)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s Vegetation cooking skipped. No foliage spawners selected."), 
        *MetaInfo.MapName);
        return true;
  }

  // Load all newly generated maps
  TArray<FAssetData> AssetsData;
  const FString TilesPath = MetaInfo.DestinationPath;
  bool success = LoadWorlds(AssetsData, TilesPath);
  if(!success || AssetsData.Num() <= 0)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("No Tiles found in %s. Vegetation cooking Aborted!"), *TilesPath);
    return false;
  }

  // Cook vegetation for each of the maps
  for(FAssetData AssetData : AssetsData)
  {
    UWorld* World = GetWorldFromAssetData(AssetData);

    // Check if it is not a tile
    if(!World->GetMapName().Contains("_Tile_"))
    {
      UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: %s Skipped as it is not a tile"), 
          *CUR_CLASS_FUNC_LINE, *World->GetMapName());
      continue;
    }

    const FString MapNameToLoad = TilesPath + "/" + World->GetMapName() + "." + World->GetMapName();
    
    // Load Map to editor. Required to spawn simulatee procedural foliage
    bool bLoadedSuccess = FEditorFileUtils::LoadMap(*MapNameToLoad, false, true);
    if(!bLoadedSuccess){
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Loading %s"),
          *CUR_CLASS_FUNC_LINE, *MapNameToLoad);
      return false;
    }

    // Cook vegetation to world
    bool bVegetationSuccess = CookVegetationToWorld(World, MetaInfo.FoliageSpawners);
    if(!bVegetationSuccess){
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Cooking Vegetation in %s"),
          *CUR_CLASS_FUNC_LINE, *MapNameToLoad);
      return false;
    }

    // Save world with vegetation spawned
    bool bSaved = SaveWorld(AssetData, MetaInfo.DestinationPath, World->GetMapName());
    if(!bSaved){
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Saving after Cooking Vegetation in %s"),
          *CUR_CLASS_FUNC_LINE, *MapNameToLoad);
      return false;
    }
  }

  return true;
}

bool UMapGeneratorWidget::ApplyHeightMapToLandscape(
    FAssetData& WorldAssetData, 
    FMapGeneratorTileMetaInfo TileMetaInfo)
{
  UWorld* World = GetWorldFromAssetData(WorldAssetData);
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Applying Heigthmap to %s tile (%d_%d)"), 
      *CUR_CLASS_FUNC_LINE, *World->GetMapName(), TileMetaInfo.IndexX, TileMetaInfo.IndexY);

  /****** TODO PROV Loading World  ******/
  // UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("%s: Loading %s tile to Editor (%d_%d)"), 
  //     *CUR_CLASS_FUNC_LINE, *World->GetMapName(), TileMetaInfo.IndexX, TileMetaInfo.IndexY);
  // const FString TilesPath = TileMetaInfo.MapMetaInfo.DestinationPath;
  // const FString MapNameToLoad = TilesPath + "/" + World->GetMapName() + "." + World->GetMapName();
  // bool bLoadedSuccess = FEditorFileUtils::LoadMap(*MapNameToLoad, false, true);
  // if(!bLoadedSuccess){
  //   UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Loading %s"),
  //       *CUR_CLASS_FUNC_LINE, *MapNameToLoad);
  //   return false;
  // }
  /****************************/

  ALandscape* Landscape = (ALandscape*) UGameplayStatics::GetActorOfClass(
      World, 
      ALandscape::StaticClass());
  if (Landscape == nullptr)
        UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error. No landscape found in tile %s %d_%d"), 
            *CUR_CLASS_FUNC_LINE, *World->GetName(), TileMetaInfo.IndexX, TileMetaInfo.IndexY);
  AssignLandscapeHeightMap(Landscape, TileMetaInfo);

  // LandscapePostEditEvent(Landscape);

  // FEditorFileUtils::SaveCurrentLevel();

  return true;
}

bool UMapGeneratorWidget::CookVegetationToWorld(
  UWorld* World, 
  const TArray<UProceduralFoliageSpawner*> FoliageSpawners)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Cooking vegetation to %s"), 
      *CUR_CLASS_FUNC_LINE, *World->GetMapName());

  // TODO PROV : Fix World *********************/
  ALandscape* Landscape = (ALandscape*) UGameplayStatics::GetActorOfClass(
      World, 
      ALandscape::StaticClass());
  LandscapePostEditEvent(Landscape);
  /********************************************/

  // For each spawner create a procedural foliage volume and simulates the vegetation
  for(auto Spawner : FoliageSpawners)
  {
    ULevel* Level = World->GetCurrentLevel();

    VectorRegister	Rotation{ 0,0,0 };
    VectorRegister	Translation{ 200000.0, 200000.0, 0.0 };
    VectorRegister Scale3D{ 2500,2500,900 };
    EObjectFlags InObjectFlags = RF_Transactional;
    FName InName = NAME_None;
    
    FTransform Transform{ Rotation,Translation,Scale3D };
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Creating Volume...");
    UActorFactory* ActorFactory = GEditor->FindActorFactoryForActorClass(AProceduralFoliageVolume::StaticClass());
    AProceduralFoliageVolume* FoliageVolumeActor = (AProceduralFoliageVolume*) ActorFactory->CreateActor(
        AProceduralFoliageVolume::StaticClass(), Level, Transform, InObjectFlags, InName);

    UProceduralFoliageComponent* FoliageComponent = FoliageVolumeActor->ProceduralComponent;
    FoliageComponent->FoliageSpawner = Spawner;

    TArray<FDesiredFoliageInstance> FoliageInstances;
    bool result = FoliageComponent->GenerateProceduralContent(FoliageInstances);

    if(result && FoliageInstances.Num() > 0)
    {
      FoliageComponent->RemoveProceduralContent(false);

      FFoliagePaintingGeometryFilter OverrideGeometryFilter;
      OverrideGeometryFilter.bAllowStaticMesh = FoliageComponent->bAllowStaticMesh;
      OverrideGeometryFilter.bAllowBSP = FoliageComponent->bAllowBSP;
      OverrideGeometryFilter.bAllowLandscape = FoliageComponent->bAllowLandscape;
      OverrideGeometryFilter.bAllowFoliage = FoliageComponent->bAllowFoliage;
      OverrideGeometryFilter.bAllowTranslucent = FoliageComponent->bAllowTranslucent;

      FEdModeFoliage::AddInstances(World, FoliageInstances, OverrideGeometryFilter, true);					
    }
    else
    {
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Procedural content generation failed!"), 
          *CUR_CLASS_FUNC_LINE);
      return false;
    }
  }
  return true;
}

UWorld* UMapGeneratorWidget::GetWorldFromAssetData(FAssetData& WorldAssetData)
{
  UWorld* World;
  UObjectRedirector *BaseMapRedirector = 
    Cast<UObjectRedirector>(WorldAssetData.GetAsset());
  if(BaseMapRedirector != nullptr)
    World = CastChecked<UWorld>(BaseMapRedirector->DestinationObject);
  else
    World = CastChecked<UWorld>(WorldAssetData.GetAsset());

  return World;
}

float UMapGeneratorWidget::GetLandscapeSurfaceHeight(UWorld* World, float x, float y, bool bDrawDebugLines)
{
  if(World)
  {
    FVector RayStartingPoint(x, y, 999999);
    FVector RayEndPoint(x, y, -999999);

    // Raytrace
    FHitResult HitResult;
    World->LineTraceSingleByObjectType(
        OUT HitResult,
        RayStartingPoint,
        RayEndPoint,
        FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic),
        FCollisionQueryParams());

    // Draw debug line.
    if (bDrawDebugLines)
    {
      FColor LineColor;

      if (HitResult.GetActor()) LineColor = FColor::Red;
      else LineColor = FColor::Green;

      DrawDebugLine(
          World,
          RayStartingPoint,
          RayEndPoint,
          LineColor,
          true,
          5.f,
          0.f,
          10.f);
    }

    // Return Z Location.
    if (HitResult.GetActor()) return HitResult.ImpactPoint.Z;
  }
  return 0.0f;
}