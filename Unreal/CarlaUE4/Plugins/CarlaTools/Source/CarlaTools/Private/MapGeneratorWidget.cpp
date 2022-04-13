// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapGeneratorWidget.h"

#include "ActorFactories/ActorFactory.h"
#include "AssetRegistryModule.h"
#include "Components/SplineComponent.h"
#include "Editor/FoliageEdit/Public/FoliageEdMode.h"
#include "EditorLevelLibrary.h"
#include "FileHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "ProceduralFoliageComponent.h"
#include "ProceduralFoliageVolume.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"

#define CUR_CLASS_FUNC (FString(__FUNCTION__))
#define CUR_LINE  (FString::FromInt(__LINE__))
#define CUR_CLASS_FUNC_LINE (CUR_CLASS_FUNC + "::" + CUR_LINE)

DEFINE_LOG_CATEGORY(LogCarlaToolsMapGenerator);

void UMapGeneratorWidget::GenerateMapFiles(const FMapGeneratorMetaInfo& MetaInfo)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Map Generation %s %s"), 
      *CUR_CLASS_FUNC_LINE, *MetaInfo.DestinationPath, *MetaInfo.MapName);

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

AActor* UMapGeneratorWidget::GenerateWater(TSubclassOf<class AActor> RiverClass)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("%s: Starting Generating Waterbodies"), 
        *CUR_CLASS_FUNC_LINE);

  UWorld* World = GetWorld();

  float ActorZCoord = GetLandscapeSurfaceHeight(World, 0, 0, false);
  FVector Location(0, 0, ActorZCoord);
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
      
      // Loading tile template
      bool bLoaded = LoadBaseTileWorld(WorldAssetData);
      if(!bLoaded){
        UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error loading Base Tile World"), 
            *CUR_CLASS_FUNC_LINE);
        return false;
      }

      FMapGeneratorTileMetaInfo MetaTileInfo;
      MetaTileInfo.IndexX = i;
      MetaTileInfo.IndexY = j;
      
      // Apply heighmap to tile landscape
      ApplyHeightMapToLandscape(WorldAssetData,MetaTileInfo);

      const FString MapName = 
          MetaInfo.MapName + "_Tile_" + FString::FromInt(i) + "_" + FString::FromInt(j);
      
      // Save new tile map
      bool bSaved = SaveWorld(WorldAssetData, MetaInfo.DestinationPath, MapName, true);
      if(!bSaved){
        UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("%s: Error saving %s tile to %s"), 
            *CUR_CLASS_FUNC_LINE, *MapName, *MetaInfo.DestinationPath);
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

  // For each spawner create a procedural foliage volume and simulates the vegetation
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