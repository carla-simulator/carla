// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "LargeMapTileManager.h"
#include "LargeMapUtils.h"


ULargeMapTileManager::ULargeMapTileManager()
{
  PrimaryComponentTick.bCanEverTick = false;
}

void ULargeMapTileManager::BeginPlay()
{
  Super::BeginPlay();

}

void ULargeMapTileManager::ChangeWorldOrigin(FIntVector InSrcOrigin, FIntVector InDstOrigin)
{
  CurrentOriginInt = InDstOrigin;
  CurrentOriginD = FDVector(InDstOrigin);

  UWorld* World = GetWorld();
  UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(World);
  CarlaEpisode->SetCurrentMapOrigin(CurrentOriginInt);

#if WITH_EDITOR
  GEngine->AddOnScreenDebugMessage(66, 1.0f, FColor::Yellow,
    FString::Printf(TEXT("Src: %s  ->  Dst: %s"), *InSrcOrigin.ToString(), *InDstOrigin.ToString()));
  LM_LOG(Error, "PostWorldOriginOffset Src: %s  ->  Dst: %s", *InSrcOrigin.ToString(), *InDstOrigin.ToString());

  // This is just to update the color of the msg with the same as the closest map
  const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();
  FColor LevelColor = FColor::White;
  float MinDistance = 10000000.0f;
  for (int i = 0; i < StreamingLevels.Num(); i++)
  {
    ULevelStreaming* Level = StreamingLevels[i];
    FVector LevelLocation = Level->LevelTransform.GetLocation();
    float Distance = FVector::Dist(LevelLocation, FVector(InDstOrigin));
    if (Distance < MinDistance)
    {
      MinDistance = Distance;
      PositonMsgColor = Level->LevelColor.ToFColor(false);
    }
  }
#endif // WITH_EDITOR
}

void ULargeMapTileManager::OnLevelAddedToWorld(ULevel* InLevel)
{
  LM_LOG(Warning, "ULargeMapTileManager::OnLevelAddedToWorld");
  //FDebug::DumpStackTraceToLog(/*ELogVerbosity::Log*/);
  FCarlaMapTile& Tile = GetCarlaMapTile(InLevel);
  SpawnAssetsInTile(Tile);
}

void ULargeMapTileManager::OnLevelRemovedFromWorld(ULevel* InLevel)
{
  LM_LOG(Warning, "ULargeMapTileManager::OnLevelRemovedFromWorld");
  //FDebug::DumpStackTraceToLog(/*ELogVerbosity::Log*/);
  FCarlaMapTile& Tile = GetCarlaMapTile(InLevel);
  Tile.TilesSpawned = false;
}

void ULargeMapTileManager::GenerateMap(const FString& InAssetsPath)
{
  LM_LOG(Warning, "Generating Map %s ...", *InAssetsPath);

#if WITH_EDITOR
  AssetsPath = InAssetsPath;
#endif // WITH_EDITOR

  /* Retrive all the assets in the path */
  TArray<FAssetData> AssetsData;
  UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), true, true);
  ObjectLibrary->LoadAssetDataFromPath(InAssetsPath);
  ObjectLibrary->GetAssetDataList(AssetsData);

  /* Generate tiles based on mesh positions */
  UWorld* World = GetWorld();
  MapTiles.Reset();
  for (const FAssetData& AssetData : AssetsData)
  {
    UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.GetAsset());
    FBox BoundingBox = Mesh->GetBoundingBox();
    FVector CenterBB = BoundingBox.GetCenter();

    // Get map tile: create one if it does not exists
    FCarlaMapTile& MapTile = GetCarlaMapTile(CenterBB);

    // Update tile assets list
    MapTile.PendingAssetsInTile.Add(AssetData);
  }
  ObjectLibrary->ConditionalBeginDestroy();
  GEngine->ForceGarbageCollection(true);

#if WITH_EDITOR
  LM_LOG(Warning, "GenerateMap num Tiles generated %d", MapTiles.Num());
  DumpTilesTable();
#endif // WITH_EDITOR
}

FIntVector ULargeMapTileManager::GetNumTilesInXY() const
{
  int32 MinX = 0;
  int32 MaxX = 0;
  int32 MinY = 0;
  int32 MaxY = 0;
  for (auto& It : MapTiles)
  {
    FIntVector TileID = GetTileVectorID(It.Key);
    MinX = (TileID.X < MinX) ? TileID.X : MinX;
    MaxX = (TileID.X > MaxX) ? TileID.X : MaxX;

    MinY = (TileID.Y < MinY) ? TileID.Y : MinY;
    MaxY = (TileID.Y > MaxY) ? TileID.Y : MaxY;
  }
  return { MaxX - MinX + 1, MaxY - MinY + 1, 0 };
}

bool ULargeMapTileManager::IsLevelOfTileLoaded(FIntVector InTileID) const
{
  uint64 TileID = GetTileID(InTileID);

  const FCarlaMapTile* Tile = MapTiles.Find(TileID);
  if (!Tile)
  {
    if (bPrintErrors)
    {
      LM_LOG(Warning, "IsLevelOfTileLoaded Tile %s does not exist", *InTileID.ToString());
    }
    return false;
  }

  const ULevelStreamingDynamic* StreamingLevel = Tile->StreamingLevel;

  return (StreamingLevel && StreamingLevel->GetLoadedLevel());
}

void ULargeMapTileManager::GetTilesToConsider(
  const FDVector& LocationToConsider,
  TSet<uint64>& OutTilesToConsider)
{
  // Calculate Current Tile
  FIntVector CurrentTile = GetTileVectorID(LocationToConsider);

  // Calculate the number of tiles in range based on LayerStreamingDistance
  int32 TilesToConsider = (int32)(LayerStreamingDistance / TileSide) + 1;
  for (int Y = -TilesToConsider; Y < TilesToConsider; Y++)
  {
    for (int X = -TilesToConsider; X < TilesToConsider; X++)
    {
      // I don't check the bounds of the Tile map, if the Tile does not exist
      // I just simply discard it
      FIntVector TileToCheck = CurrentTile + FIntVector(X, Y, 0);

      uint64 TileID = GetTileID(TileToCheck);
      FCarlaMapTile* Tile = MapTiles.Find(TileID);
      if (!Tile)
      {
        continue; // Tile does not exist, discard
      }

      // Calculate distance between actor and tile
      float Distance = FDVector::Dist(Tile->Location, LocationToConsider);

      // Load level if we are in range
      if (Distance < LayerStreamingDistance)
      {
        OutTilesToConsider.Add(TileID);
      }
    }
  }
}

void ULargeMapTileManager::UpdateTilesState(TArray<FDVector>& ActorsLocation)
{

  TSet<uint64> TilesToConsider;
  for (FDVector& Location : ActorsLocation)
  {
    GetTilesToConsider(Location, TilesToConsider);
  }

  TSet<uint64> TilesToBeVisible;
  TSet<uint64> TilesToHidde;
  GetTilesThatNeedToChangeState(TilesToConsider, TilesToBeVisible, TilesToHidde);

  UpdateTileState(TilesToBeVisible, true, true, true);
  UpdateTileState(TilesToHidde, false, false, false);

  UpdateCurrentTilesLoaded(TilesToBeVisible, TilesToHidde);
}

FIntVector ULargeMapTileManager::GetTileVectorID(FVector TileLocation) const
{
  return FIntVector(TileLocation / TileSide);
}

FIntVector ULargeMapTileManager::GetTileVectorID(FDVector TileLocation) const
{
  return (TileLocation / TileSide).ToFIntVector();
}

FIntVector ULargeMapTileManager::GetTileVectorID(uint64 TileID) const
{
  return FIntVector{
    (int32)(TileID >> 32),
    (int32)(TileID & (int32)(~0)),
    0
  };
}

uint64 ULargeMapTileManager::GetTileID(FIntVector TileVectorID) const
{
  int64 X = ((int64)(TileVectorID.X) << 32);
  int64 Y = (int64)(TileVectorID.Y) & 0x00000000FFFFFFFF;
  return (X | Y);
}

uint64 ULargeMapTileManager::GetTileID(FVector TileLocation) const
{
  FIntVector TileID = GetTileVectorID(TileLocation);
  return GetTileID(TileID);
}

FCarlaMapTile& ULargeMapTileManager::GetCarlaMapTile(FVector Location)
{
  // Asset Location -> TileID
  uint64 TileID = GetTileID(Location);

  FCarlaMapTile* Tile = MapTiles.Find(TileID);
  if (Tile) return *Tile; // Tile founded

  // Need to generate a new Tile
  FCarlaMapTile NewTile;
  // 1 - Calculate the Tile position
  FIntVector VTileID = GetTileVectorID(TileID);
  FVector OriginOffset = FVector(FMath::Sign(VTileID.X), FMath::Sign(VTileID.Y), FMath::Sign(VTileID.Z)) * 0.5f;
  NewTile.Location = (FVector(VTileID) + OriginOffset)* TileSide;
#if WITH_EDITOR
  // 2 - Generate Tile name
  NewTile.Name = GenerateTileName(TileID);
#endif // WITH_EDITOR
  // 3 - Generate the StreamLevel
  NewTile.StreamingLevel = AddNewTile(NewTile.Name, NewTile.Location);
  // 4 - Add it to the map
  return MapTiles.Add(TileID, NewTile);
}

FCarlaMapTile& ULargeMapTileManager::GetCarlaMapTile(ULevel* InLevel)
{
  FCarlaMapTile* Tile = nullptr;
  for (auto& It : MapTiles)
  {
    ULevelStreamingDynamic* StreamingLevel = It.Value.StreamingLevel;
    ULevel* Level = StreamingLevel->GetLoadedLevel();
    if (Level == InLevel)
    {
      Tile = &(It.Value);
      break;
    }
  }
  check(Tile);
  return *Tile;
}

FCarlaMapTile* ULargeMapTileManager::GetCarlaMapTile(FIntVector TileVectorID)
{
  uint64 TileID = GetTileID(TileVectorID);
  FCarlaMapTile* Tile = MapTiles.Find(TileID);
  return Tile;
}

ULevelStreamingDynamic* ULargeMapTileManager::AddNewTile(FString TileName, FVector TileLocation)
{
  UWorld* World = GetWorld();
  UWorldComposition* WorldComposition = World->WorldComposition;

  FString FullName = BaseTileMapPath;
  FString PackageFileName = FullName;
  FString LongLevelPackageName = FPackageName::FilenameToLongPackageName(PackageFileName);
  FString UniqueLevelPackageName = LongLevelPackageName + TileName;

  ULevelStreamingDynamic* StreamingLevel = NewObject<ULevelStreamingDynamic>(World, *TileName, RF_Transient);
  check(StreamingLevel);

  StreamingLevel->SetWorldAssetByPackageName(*UniqueLevelPackageName);

#if WITH_EDITOR
  if (World->IsPlayInEditor())
  {
    FWorldContext WorldContext = GEngine->GetWorldContextFromWorldChecked(World);
    StreamingLevel->RenameForPIE(WorldContext.PIEInstance);
  }
  StreamingLevel->SetShouldBeVisibleInEditor(true);
  StreamingLevel->LevelColor = FColor::MakeRandomColor();
#endif // WITH_EDITOR

  StreamingLevel->SetShouldBeLoaded(false);
  StreamingLevel->SetShouldBeVisible(false);
  StreamingLevel->bShouldBlockOnLoad = ShouldTilesBlockOnLoad;
  StreamingLevel->bInitiallyLoaded = false;
  StreamingLevel->bInitiallyVisible = false;
  StreamingLevel->LevelTransform = FTransform(TileLocation);
  StreamingLevel->PackageNameToLoad = *FullName;

  if (!FPackageName::DoesPackageExist(FullName, NULL, &PackageFileName))
  {
    LM_LOG(Error, "Level does not exist in package with FullName variable -> %s", *FullName);
  }

  if (!FPackageName::DoesPackageExist(LongLevelPackageName, NULL, &PackageFileName))
  {
    LM_LOG(Error, "Level does not exist in package with LongLevelPackageName variable -> %s", *LongLevelPackageName);
  }

  //Actual map package to load
  StreamingLevel->PackageNameToLoad = *LongLevelPackageName;


  World->AddStreamingLevel(StreamingLevel);
  WorldComposition->TilesStreaming.Add(StreamingLevel);


  FWorldTileInfo Info;
  Info.AbsolutePosition = FIntVector(TileLocation);
  FWorldTileLayer WorldTileLayer;
  WorldTileLayer.Name = "CarlaLayer";
  WorldTileLayer.StreamingDistance = LayerStreamingDistance;
  WorldTileLayer.DistanceStreamingEnabled = false; // we will handle this, not unreal
  Info.Layer = WorldTileLayer;

  FWorldCompositionTile NewTile;
  NewTile.PackageName = *FullName;
  NewTile.Info = Info;
  WorldComposition->GetTilesList().Add(NewTile);

  return StreamingLevel;
}

void ULargeMapTileManager::GetTilesThatNeedToChangeState(
  const TSet<uint64>& InTilesToConsider,
  TSet<uint64>& OutTilesToBeVisible,
  TSet<uint64>& OutTilesToHidde)
{
  OutTilesToBeVisible = InTilesToConsider.Difference(CurrentTilesLoaded);
  OutTilesToHidde = CurrentTilesLoaded.Difference(InTilesToConsider);
}

void ULargeMapTileManager::UpdateTileState(
  const TSet<uint64>& InTilesToUpdate,
  bool InShouldBlockOnLoad,
  bool InShouldBeLoaded,
  bool InShouldBeVisible)
{
  UWorld* World = GetWorld();
  UWorldComposition* WorldComposition = World->WorldComposition;

  // Gather all the locations of the levels to load
  for (const uint64 TileID : InTilesToUpdate)
  {
      FCarlaMapTile* CarlaTile = MapTiles.Find(TileID);
      check(CarlaTile); // If an invalid ID reach here, we did something very wrong
      ULevelStreamingDynamic* StreamingLevel = CarlaTile->StreamingLevel;
      StreamingLevel->bShouldBlockOnLoad = InShouldBlockOnLoad;
      StreamingLevel->SetShouldBeLoaded(InShouldBeLoaded);
      StreamingLevel->SetShouldBeVisible(InShouldBeVisible);
  }
}

void ULargeMapTileManager::UpdateCurrentTilesLoaded(
  const TSet<uint64>& InTilesToBeVisible,
  const TSet<uint64>& InTilesToHidde)
{
  for (const uint64 TileID : InTilesToHidde)
  {
    CurrentTilesLoaded.Remove(TileID);
  }

  for (const uint64 TileID : InTilesToBeVisible)
  {
    CurrentTilesLoaded.Add(TileID);
  }
}

void ULargeMapTileManager::SpawnAssetsInTile(FCarlaMapTile& Tile)
{
  if (Tile.TilesSpawned) return;

  FVector CurrentWorldOrigin(CurrentOriginInt);
  FVector TileLocation = Tile.Location - CurrentWorldOrigin;

  ULevel* LoadedLevel = Tile.StreamingLevel->GetLoadedLevel();
  UWorld* World = GetWorld();

#if WITH_EDITOR
  FString Output = "";
  Output += FString::Printf(TEXT("SpawnAssetsInTile %s %d\n"), *Tile.Name, Tile.PendingAssetsInTile.Num());
  Output += FString::Printf(TEXT("  Tile.Loc = %s -> %s\n"), *Tile.Location.ToString(), *TileLocation.ToString());
#endif // WITH_EDITOR

  for (const FAssetData& AssetData : Tile.PendingAssetsInTile)
  {
    UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.GetAsset());
    if (!Mesh)
    {
      LM_LOG(Error, "Mesh %s could not be loaded in %s", *AssetData.ObjectPath.ToString(), *Tile.Name);
      continue;
    }
    FBox BoundingBox = Mesh->GetBoundingBox();
    FVector CenterBB = BoundingBox.GetCenter();

    // Recalculate asset location based on tile position
    FTransform Transform(FRotator(0.0f), CenterBB - TileLocation - CurrentWorldOrigin, FVector(1.0f));

    // Create intermediate actor (AUncenteredPivotPointMesh) to paliate not centered pivot point of the mesh
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = *FString::Printf(TEXT("UPPM_%s_%s"), *Mesh->GetName(), *Tile.Name);
    SpawnParams.OverrideLevel = LoadedLevel;

    AUncenteredPivotPointMesh* SMActor =
      World->SpawnActor<AUncenteredPivotPointMesh>(
        AUncenteredPivotPointMesh::StaticClass(),
        Transform,
        SpawnParams);

    if (SMActor)
    {
      UStaticMeshComponent* SMComp = SMActor->GetMeshComp();
      if (SMComp)
      {
        SMComp->SetStaticMesh(Mesh);
        SMComp->SetRelativeLocation(CenterBB * -1.0f); // The pivot point does not get affected by Tile location
      }
#if WITH_EDITOR
      Output += FString::Printf(TEXT("    MeshName = %s -> %s\n"), *Mesh->GetName(), *SMActor->GetName());
      Output += FString::Printf(TEXT("      CenterBB = %s\n"), *CenterBB.ToString());
      Output += FString::Printf(TEXT("      ActorLoc = %s\n"), *(CenterBB - TileLocation).ToString());
      Output += FString::Printf(TEXT("      SMRelLoc = %s\n"), *SMComp->GetRelativeLocation().ToString());
#endif // WITH_EDITOR
    }
  }
  LM_LOG(Warning, "%s", *Output);

  LoadedLevel->ApplyWorldOffset(TileLocation, false);

  Tile.TilesSpawned = true;
}

#if WITH_EDITOR

void ULargeMapTileManager::PrintMapInfo(float MsgTime, int32 FirstMsgIndex, int32 MsgLimit)
{
  UWorld* World = GetWorld();

  FDVector CurrentActorPosition;

  const TArray<ULevel*>& Levels = World->GetLevels();
  const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();

  FString Output = "";
  Output += FString::Printf(TEXT("Num levels in world composition: %d\n"), World->WorldComposition->TilesStreaming.Num());
  Output += FString::Printf(TEXT("Num levels loaded: %d\n"), Levels.Num(), World->GetNumLevels());
  GEngine->AddOnScreenDebugMessage(0, MsgTime, FColor::Cyan, Output);

  int CurrentMsgIndex = FirstMsgIndex;
  int LastMsgIndex = FirstMsgIndex + MsgLimit;

  GEngine->AddOnScreenDebugMessage(CurrentMsgIndex++, MsgTime, FColor::White, TEXT("Closest tiles - Distance:"));

  for (int i = 0; i < StreamingLevels.Num(); i++)
  {
    ULevelStreaming* Level = StreamingLevels[i];
    FVector LevelLocation = Level->LevelTransform.GetLocation();
    float Distance = FDVector::Dist(LevelLocation, CurrentActorPosition);

    FColor MsgColor = (Levels.Contains(Level->GetLoadedLevel())) ? FColor::Green : FColor::Red;
    if (Distance < (TileSide * 2.0f))
    {
      GEngine->AddOnScreenDebugMessage(CurrentMsgIndex++, MsgTime, MsgColor,
        FString::Printf(TEXT("%s       %.2f"), *Level->GetName(), Distance / (1000.0f * 100.0f)));
    }
    if (CurrentMsgIndex < LastMsgIndex) break;
  }

}

FString ULargeMapTileManager::GenerateTileName(uint64 TileID)
{
  int32 X = (int32)(TileID >> 32);
  int32 Y = (int32)(TileID);
  return FString::Printf(TEXT("Tile_%d_%d"), X, Y);
}

void ULargeMapTileManager::DumpTilesTable() const
{
  FString FileContent = "";
  FileContent += FString::Printf(TEXT("LargeMapManager state\n"));

  FileContent += FString::Printf(TEXT("Tile:\n"));
  FileContent += FString::Printf(TEXT("ID\tName\tLocation\tAssetsInTile\n"));
  for (auto& It : MapTiles)
  {
    const FCarlaMapTile& Tile = It.Value;
    FileContent += FString::Printf(TEXT("  %ld\t%s\t%s\t%d\n"), It.Key, *Tile.Name, *Tile.Location.ToString(), Tile.PendingAssetsInTile.Num());
  }
  FileContent += FString::Printf(TEXT("\nNum generated tiles: %d\n"), MapTiles.Num());

  // Generate the map name with the assets folder name
  TArray<FString> StringArray;
  AssetsPath.ParseIntoArray(StringArray, TEXT("/"), false);

  FString FilePath = FPaths::ProjectSavedDir() + StringArray[StringArray.Num() - 1] + ".txt";
  FFileHelper::SaveStringToFile(
    FileContent,
    *FilePath,
    FFileHelper::EEncodingOptions::AutoDetect,
    &IFileManager::Get(),
    EFileWrite::FILEWRITE_Silent);
}

#endif // WITH_EDITOR
