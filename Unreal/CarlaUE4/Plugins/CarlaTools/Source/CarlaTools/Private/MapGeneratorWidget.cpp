// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// #if WITH_EDITOR

#include "MapGeneratorWidget.h"

#include "ActorFactories/ActorFactory.h"
#include "AssetRegistryModule.h"
#include "Editor/FoliageEdit/Public/FoliageEdMode.h"
#include "EditorLevelLibrary.h"
#include "FileHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "ProceduralFoliageComponent.h"
#include "ProceduralFoliageVolume.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"

#define CUR_CLASS_FUNC (FString(__FUNCTION__))
#define CUR_LINE  (FString::FromInt(__LINE__))
#define CUR_CLASS_FUNC_LINE (CUR_CLASS_FUNC + "::" + CUR_LINE)

DEFINE_LOG_CATEGORY(LogCarlaToolsMapGenerator);

void UMapGeneratorWidget::GenerateMapFiles(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Map Generation %s %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.DestinationPath, *MetaInfo.MapName);

  // TODO: Check if directory exists and if it's empty
  // The tool will need to create a new directory (which cannot share name with
  // an existing one) to don't mess up with redirectors

  // TODO: Check that the directory in metaInfo cannot finish in /, if does, delete it

  // // 1. Creating tiles terrain
  bool bTIlesSuccess = CreateTilesMaps(MetaInfo);
  if(!bTIlesSuccess)
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Cooking vegetation for %s"), 
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  // 2. Create Main Large map
  bool BLargeMapSuccess = CreateMainLargeMap(MetaInfo);
  if(!BLargeMapSuccess)
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error creating Main Large Map for %s"), 
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  
}

void UMapGeneratorWidget::CookVegetation(const FMapGeneratorMetaInfo& MetaInfo)
{
  // 3. Add vegetation to tiles
  bool bVegetationSuccess = CookVegetationToTiles(MetaInfo);
  if(!bVegetationSuccess)
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error Cooking vegetation for %s"), 
        *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);
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

bool UMapGeneratorWidget::LoadBaseTileWorld(FAssetData& WorldAssetData)
{
  const FString BaseMapPath= TEXT("/CarlaTools/MapGenerator/BaseMap/Tiles");
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

bool UMapGeneratorWidget::LoadWorlds(TArray<FAssetData>& WorldAssetsData, const FString& BaseMapPath)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Loading Worlds from %s"), 
      *CUR_CLASS_FUNC_LINE, *BaseMapPath);

  TArray<FAssetData> AssetsData;
  UObjectLibrary *MapObjectLibrary;

  // Loading Map from folder using object library
  MapObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
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

  // Create Package
  // GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Preparing Package");
  UPackage *Package = WorldToBeSaved.GetPackage();
  Package->SetFolderName("MapGeneratorPackage");
  Package->FullyLoad();
  Package->MarkPackageDirty();
  FAssetRegistryModule::AssetCreated(World);
  
  // Rename new World
  World->Rename(*WorldName, World->GetOuter());
  const FString PackagePath = DestinationPath + "/" + WorldName;
  FAssetRegistryModule::AssetRenamed(World, *PackagePath);
  // GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, World->GetMapName());
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
  bool bLoaded = LoadBaseLargeMapWorld(WorldAssetData);
  bool bSaved = SaveWorld(WorldAssetData, MetaInfo.DestinationPath, MetaInfo.MapName, true);

  return true;
}

bool UMapGeneratorWidget::CreateTilesMaps(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Creating %s tiles maps in %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName, *MetaInfo.DestinationPath);
  FAssetData WorldAssetData;

  for(int i = 0; i < MetaInfo.SizeX; i++)
  {
    for(int j = 0; j < MetaInfo.SizeY; j++)
    {
      UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Tile map %s (%d_%d)"), 
          *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName, i, j);
      bool bLoaded = LoadBaseTileWorld(WorldAssetData);

      if(!bLoaded)
        return false;

      FMapGeneratorTileMetaInfo MetaTileInfo;
      MetaTileInfo.IndexX = i;
      MetaTileInfo.IndexY = j;
      
      // 1. Terrain 
      ApplyHeightMapToLandscape(WorldAssetData,MetaTileInfo);

      const FString MapName = 
          MetaInfo.MapName + "_Tile_" + FString::FromInt(i) + "_" + FString::FromInt(j);
      bool bSaved = SaveWorld(WorldAssetData, MetaInfo.DestinationPath, MapName, true);

      if(!bSaved)
        return false;
    }
  }
  return true;
}

bool UMapGeneratorWidget::CookVegetationToTiles(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Cooking vegetation to %s tiles"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.MapName);

  //TODO: First check if there is elements in MetaInfo.FoliageSpawners

  TArray<FAssetData> AssetsData;
  const FString TilesPath = MetaInfo.DestinationPath;
  bool success = LoadWorlds(AssetsData, TilesPath);
  if(!success || AssetsData.Num() <= 0)
  {
    UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("No Tiles found in %s. Vegetation cooking Aborted!"), *TilesPath);
    return false;
  }

  for(FAssetData AssetData : AssetsData)
  {
    UWorld* World = GetWorldFromAssetData(AssetData);

    // Check if it is not a tile
    if(!World->GetMapName().Contains("_Tile_"))
    {
      UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s Skipped as it is not a tile"), *World->GetMapName())
      continue;
    }

    const FString MapNameToLoad = TilesPath + "/" + World->GetMapName() + "." + World->GetMapName();
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Loading to editor %s"), *MapNameToLoad);
    
    // Load Map to editor. Required to spawn simulatee procedural foliage
    bool bLoadedSuccess = FEditorFileUtils::LoadMap(*MapNameToLoad, false, true);
    if(!bLoadedSuccess){
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Error Loading %s"), *MapNameToLoad);
      return false;
    }

    // Cook vegetation to world
    bool bVegetationSuccess = CookVegetationToWorld(World, MetaInfo.FoliageSpawners);
    if(!bVegetationSuccess){
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Error Cooking Vegetation in %s"), *MapNameToLoad);
      return false;
    }

    // Save world with vegetation spawned
    bool bSaved = SaveWorld(AssetData, MetaInfo.DestinationPath, World->GetMapName());
    if(!bSaved){
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Error Saving after Cooking Vegetation in %s"), *MapNameToLoad);
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

  ALandscape* landscape = (ALandscape*) UGameplayStatics::GetActorOfClass(
      World, 
      ALandscape::StaticClass());
  AssignLandscapeHeightMap(landscape, TileMetaInfo);
  return true;
}

bool UMapGeneratorWidget::CookVegetationToWorld(
  UWorld* World, 
  const TArray<UProceduralFoliageSpawner*> FoliageSpawners)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Cooking vegetation to %s"), 
      *CUR_CLASS_FUNC_LINE, *World->GetMapName());
  GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, World->GetMapName());
  for(auto Spawner : FoliageSpawners)
  {
    ULevel* Level = World->GetCurrentLevel();

    VectorRegister	Rotation{ 0,0,0 };
    VectorRegister	Translation{ 0.0,0.0,0.0 };
    VectorRegister Scale3D{ 2500,2500,900 };
    EObjectFlags InObjectFlags = RF_Transactional;
    FName InName = NAME_None;
    
    FTransform Transform{ Rotation,Translation,Scale3D };
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Creating Volume...");
    UActorFactory* ActorFactory = GEditor->FindActorFactoryForActorClass(AProceduralFoliageVolume::StaticClass());
    AProceduralFoliageVolume* FoliageVolumeActor = (AProceduralFoliageVolume*) ActorFactory->CreateActor(AProceduralFoliageVolume::StaticClass(), Level, Transform, InObjectFlags, InName);

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