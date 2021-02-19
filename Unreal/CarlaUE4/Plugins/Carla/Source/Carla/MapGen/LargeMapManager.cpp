// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.



#include "LargeMapManager.h"

#include "Engine/LocalPlayer.h"
#include "Engine/WorldComposition.h"

#include "UncenteredPivotPointMesh.h"

#include "Walker/WalkerBase.h"

#if WITH_EDITOR
#include "FileHelper.h"
#include "Paths.h"
#endif // WITH_EDITOR

#define LARGEMAP_LOGS 1

#if LARGEMAP_LOGS
#define LM_LOG UE_LOG
#else
#define LM_LOG(...)
#endif

// Sets default values
ALargeMapManager::ALargeMapManager()
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickInterval = TickInterval;
}

ALargeMapManager::~ALargeMapManager()
{
  /* Remove delegates */
  // Origin rebase
  FCoreDelegates::PreWorldOriginOffset.RemoveAll(this);
  FCoreDelegates::PostWorldOriginOffset.RemoveAll(this);
  // Level added/removed from world
  FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);
  FWorldDelegates::LevelAddedToWorld.RemoveAll(this);

}

// Called when the game starts or when spawned
void ALargeMapManager::BeginPlay()
{
	Super::BeginPlay();

  UWorld* World = GetWorld();
  /* Setup delegates */
  // Origin rebase
  FCoreDelegates::PreWorldOriginOffset.AddUObject(this, &ALargeMapManager::PreWorldOriginOffset);
  FCoreDelegates::PostWorldOriginOffset.AddUObject(this, &ALargeMapManager::PostWorldOriginOffset);
  // Level added/removed from world
  FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ALargeMapManager::OnLevelAddedToWorld);
  FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &ALargeMapManager::OnLevelRemovedFromWorld);
  // Actor spawned
  FOnActorSpawned::FDelegate OnActorSpawnedDelegate =
    FOnActorSpawned::FDelegate::CreateUObject(this, &ALargeMapManager::OnActorSpawned);
  World->AddOnActorSpawnedHandler(OnActorSpawnedDelegate);

  UWorldComposition* WorldComposition = World->WorldComposition;
  // Setup Origin rebase settings
  WorldComposition->bRebaseOriginIn3DSpace = true;
  WorldComposition->RebaseOriginDistance = RebaseOriginDistance;

}


void ALargeMapManager::PreWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin)
{
  LM_LOG(LogCarla, Error, TEXT("PreWorldOriginOffset Src: %s  ->  Dst: %s"), *InSrcOrigin.ToString(), *InDstOrigin.ToString());
}

void ALargeMapManager::PostWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin)
{
  CurrentOriginInt = InDstOrigin;
  CurrentOriginD = FDVector(InDstOrigin);

  UWorld* World = GetWorld();
  UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(World);
  CarlaEpisode->SetCurrentMapOrigin(CurrentOriginInt);

#if WITH_EDITOR
  GEngine->AddOnScreenDebugMessage(66, MsgTime, FColor::Yellow,
    FString::Printf(TEXT("Src: %s  ->  Dst: %s"), *InSrcOrigin.ToString(), *InDstOrigin.ToString()));
  LM_LOG(LogCarla, Error, TEXT("PostWorldOriginOffset Src: %s  ->  Dst: %s"), *InSrcOrigin.ToString(), *InDstOrigin.ToString());

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

void ALargeMapManager::OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld)
{
  LM_LOG(LogCarla, Warning, TEXT("OnLevelAddedToWorld"));
  //FDebug::DumpStackTraceToLog(/*ELogVerbosity::Log*/);
  FCarlaMapTile& Tile = GetCarlaMapTile(InLevel);
  SpawnAssetsInTile(Tile);
}

void ALargeMapManager::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
  LM_LOG(LogCarla, Warning, TEXT("OnLevelRemovedFromWorld"));
  //FDebug::DumpStackTraceToLog(/*ELogVerbosity::Log*/);
  FCarlaMapTile& Tile = GetCarlaMapTile(InLevel);
  Tile.TilesSpawned = false;
}

void ALargeMapManager::OnActorSpawned(AActor *Actor)
{
  if (Cast<APawn>(Actor))
  {
    LM_LOG(LogCarla, Warning, TEXT("OnActorSpawned %s"), *Actor->GetName());
    AddActorToConsider(Actor);

    UpdateTilesState();

    // Can I force a level streaming flush here?
    GetWorld()->FlushLevelStreaming();
    // GEngine->BlockTillLevelStreamingCompleted(PlayWorld);
  }
}

void ALargeMapManager::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  UpdateActorsToConsiderPosition();

  UpdateTilesState();

  // TODO: choose ego vehicle
  if(ActorsToConsider.Num() > 0)
  {
    UWorldComposition* WorldComposition = GetWorld()->WorldComposition;
    AActor* ActorToConsider = ActorsToConsider[0].Actor;
    if( IsValid(ActorToConsider) )
    {
      WorldComposition->EvaluateWorldOriginLocation(ActorToConsider->GetActorLocation());
    }
  }

#if WITH_EDITOR
  if (bPrintMapInfo) PrintMapInfo();
#endif // WITH_EDITOR

}

void ALargeMapManager::GenerateMap(FString InAssetsPath)
{
  LM_LOG(LogCarla, Warning, TEXT("Generating Map %s ..."), *InAssetsPath);

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

    //Mesh->ConditionalBeginDestroy();
  }
  ObjectLibrary->ConditionalBeginDestroy();
  GEngine->ForceGarbageCollection(true);

  ActorsToConsider.Reset();

#if WITH_EDITOR
  LM_LOG(LogCarla, Warning, TEXT("GenerateMap num Tiles generated %d"), MapTiles.Num());
  DumpTilesTable();
#endif // WITH_EDITOR
}

void ALargeMapManager::AddActorToConsider(AActor* InActor)
{
  ActorsToConsider.Add({ InActor });
}

void ALargeMapManager::RemoveActorToConsider(AActor* InActor)
{
  int Index = 0;
  for (int i = 0; i < ActorsToConsider.Num(); i++)
  {
    const FActorToConsider& ActorToConsider = ActorsToConsider[i];
    if (ActorToConsider.Actor == InActor)
    {
      ActorsToConsider.Remove(ActorToConsider);
      Index = i;
      break;
    }
  }

}

FIntVector ALargeMapManager::GetNumTilesInXY() const
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
    // LM_LOG(LogCarla, Warning, TEXT("GetNumTilesInXY %s -- X: %d %d -- Y: %d %d"), *TileID.ToString(), MinX, MaxX, MinY, MaxY);
  }
  return { MaxX - MinX + 1, MaxY - MinY + 1, 0 };
}

bool ALargeMapManager::IsLevelOfTileLoaded(FIntVector InTileID) const
{
  uint64 TileID = GetTileID(InTileID);

  const FCarlaMapTile* Tile = MapTiles.Find(TileID);
  if (!Tile)
  {
    if (bPrintErrors)
    {
      LM_LOG(LogCarla, Warning, TEXT("IsLevelOfTileLoaded Tile %s does not exist"), *InTileID.ToString());
    }
    return false;
  }

  const ULevelStreamingDynamic* StreamingLevel = Tile->StreamingLevel;

  return (StreamingLevel && StreamingLevel->GetLoadedLevel());
}

FIntVector ALargeMapManager::GetTileVectorID(FVector TileLocation) const
{
  // LM_LOG(LogCarla, Warning, TEXT("      GetTileVectorID %s --> %s"), *TileLocation.ToString(), *FIntVector(TileLocation / TileSide).ToString());
  return FIntVector(TileLocation / TileSide);
}

FIntVector ALargeMapManager::GetTileVectorID(FDVector TileLocation) const
{
  // LM_LOG(LogCarla, Warning, TEXT("      GetTileVectorID %s --> %s"), *TileLocation.ToString(), *(TileLocation / TileSide).ToFIntVector().ToString());
  return (TileLocation / TileSide).ToFIntVector();
}

FIntVector ALargeMapManager::GetTileVectorID(uint64 TileID) const
{
  // LM_LOG(LogCarla, Warning, TEXT("                GetTileVectorID %ld --> %d %d"), TileID, (TileID >> 32), (TileID & (int32)(~0)) );
  return FIntVector{
    (int32)(TileID >> 32),
    (int32)(TileID & (int32)(~0)),
    0
  };
}

uint64 ALargeMapManager::GetTileID(FIntVector TileVectorID) const
{
  int64 X = ((int64)(TileVectorID.X) << 32);
  int64 Y = (int64)(TileVectorID.Y) & 0x00000000FFFFFFFF;
  return (X | Y);
}

uint64 ALargeMapManager::GetTileID(FVector TileLocation) const
{
  FIntVector TileID = GetTileVectorID(TileLocation);
  return GetTileID(TileID);
}

FCarlaMapTile& ALargeMapManager::GetCarlaMapTile(FVector Location)
{
  // LM_LOG(LogCarla, Error, TEXT("GetCarlaMapTile........"));
  // Asset Location -> TileID
  uint64 TileID = GetTileID(Location);

  // LM_LOG(LogCarla, Warning, TEXT("GetCarlaMapTile %s -> %lx"), *Location.ToString(), TileID);

  FCarlaMapTile* Tile = MapTiles.Find(TileID);
  if (Tile) return *Tile; // Tile founded

  // Need to generate a new Tile
  FCarlaMapTile NewTile;
  // 1 - Calculate the Tile position
  FIntVector VTileID = GetTileVectorID(TileID);
  FVector OriginOffset = FVector(FMath::Sign(VTileID.X), FMath::Sign(VTileID.Y), FMath::Sign(VTileID.Z)) * 0.5f;
  NewTile.Location = (FVector(VTileID) + OriginOffset)* TileSide;
  // LM_LOG(LogCarla, Warning, TEXT("                NewTile.Location %s"), *NewTile.Location.ToString());
#if WITH_EDITOR
  // 2 - Generate Tile name
  NewTile.Name = GenerateTileName(TileID);
#endif // WITH_EDITOR
  // 3 - Generate the StreamLevel
  NewTile.StreamingLevel = AddNewTile(NewTile.Name, NewTile.Location);
  // 4 - Add it to the map
  return MapTiles.Add(TileID, NewTile);
}

FCarlaMapTile& ALargeMapManager::GetCarlaMapTile(ULevel* InLevel)
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

FCarlaMapTile* ALargeMapManager::GetCarlaMapTile(FIntVector TileVectorID)
{
  uint64 TileID = GetTileID(TileVectorID);
  FCarlaMapTile* Tile = MapTiles.Find(TileID);
  return Tile;
}

ULevelStreamingDynamic* ALargeMapManager::AddNewTile(FString TileName, FVector TileLocation)
{
  UWorld* World = GetWorld();
  UWorldComposition* WorldComposition = World->WorldComposition;

  FString FullName = BaseTileMapPath;
  FString PackageFileName = FullName;
  FString LongLevelPackageName = FPackageName::FilenameToLongPackageName(PackageFileName);
  FString UniqueLevelPackageName = LongLevelPackageName + TileName;

  ULevelStreamingDynamic* StreamingLevel = NewObject<ULevelStreamingDynamic>(World, *TileName, RF_Transient);
  check(StreamingLevel);

  // LM_LOG(LogCarla, Error, TEXT("AddNewTile created new streaming level -> %s"), *TileName);

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
    LM_LOG(LogCarla, Error, TEXT("Level does not exist in package with FullName variable -> %s"), *FullName);
  }

  if (!FPackageName::DoesPackageExist(LongLevelPackageName, NULL, &PackageFileName))
  {
    LM_LOG(LogCarla, Error, TEXT("Level does not exist in package with LongLevelPackageName variable -> %s"), *LongLevelPackageName);
  }

  //Actual map package to load
  StreamingLevel->PackageNameToLoad = *LongLevelPackageName;


  World->AddStreamingLevel(StreamingLevel);
  WorldComposition->TilesStreaming.Add(StreamingLevel);


  FWorldTileInfo Info;
  Info.AbsolutePosition = FIntVector(TileLocation);
  FVector MinBound(-20000.0f);
  FVector MaxBound(20000.0f);
  //Info.Bounds = FBox(MinBound, MaxBound);
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

void ALargeMapManager::UpdateActorsToConsiderPosition()
{
  if (!ActorsToConsider.Num())
  {
    LM_LOG(LogCarla, Error, TEXT("No actors to consider"));
    return;
  }

  TArray<FActorToConsider> ActorsToRemove;
  for (FActorToConsider& ActorToConsider : ActorsToConsider)
  {
    AActor* Actor = ActorToConsider.Actor;
    if (IsValid(Actor))
    {
      // Relative location to the current origin
      FDVector ActorLocation(Actor->GetActorLocation());
      // Absolute location of the actor
      ActorToConsider.Location = CurrentOriginD + ActorLocation;
    }
    else
    {
      ActorsToRemove.Add(ActorToConsider);
    }
  }

  for (const FActorToConsider& ActorToRemove : ActorsToRemove)
  {
    ActorsToConsider.Remove(ActorToRemove);
  }
}

void ALargeMapManager::UpdateTilesState()
{
  TSet<uint64> TilesToConsider;
  for (FActorToConsider& ActorToConsider : ActorsToConsider)
  {
    GetTilesToConsider(ActorToConsider, TilesToConsider);
  }

  TSet<uint64> TilesToBeVisible;
  TSet<uint64> TilesToHidde;
  GetTilesThatNeedToChangeState(TilesToConsider, TilesToBeVisible, TilesToHidde);

  UpdateTileState(TilesToBeVisible, true, true, true);

  UpdateTileState(TilesToHidde, false, false, false);

  UpdateCurrentTilesLoaded(TilesToBeVisible, TilesToHidde);

}

void ALargeMapManager::GetTilesToConsider(const FActorToConsider& ActorToConsider,
                                          TSet<uint64>& OutTilesToConsider)
{
  FDVector ActorLocation = ActorToConsider.Location;

  // Calculate Current Tile
  FIntVector CurrentTile = GetTileVectorID(ActorLocation);

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
        // LM_LOG(LogCarla, Error, TEXT("No Tile %s"), *TileToCheck.ToString());
        continue; // Discard
      }

      // Calculate distance between actor and tile
      float Distance = FDVector::Dist(Tile->Location, ActorLocation);

      // Load level if we are in range
      if (Distance < LayerStreamingDistance)
      {
        OutTilesToConsider.Add(TileID);
      }
    }
  }
}

void ALargeMapManager::GetTilesThatNeedToChangeState(
  const TSet<uint64>& InTilesToConsider,
  TSet<uint64>& OutTilesToBeVisible,
  TSet<uint64>& OutTilesToHidde)
{
  OutTilesToBeVisible = InTilesToConsider.Difference(CurrentTilesLoaded);
  OutTilesToHidde = CurrentTilesLoaded.Difference(InTilesToConsider);
}

void ALargeMapManager::UpdateTileState(
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

void ALargeMapManager::UpdateCurrentTilesLoaded(
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

void ALargeMapManager::SpawnAssetsInTile(FCarlaMapTile& Tile)
{
  if (Tile.TilesSpawned) return;

  // LM_LOG(LogCarla, Warning, TEXT("SpawnAssetsInTile %s %d\n"), *Tile.Name, Tile.PendingAssetsInTile.Num());

  FString Output = "";
  Output += FString::Printf(TEXT("SpawnAssetsInTile %s %d\n"), *Tile.Name, Tile.PendingAssetsInTile.Num());

  FVector CurrentWorldOrigin(CurrentOriginInt);
  FVector TileLocation = Tile.Location - CurrentWorldOrigin;
  Output += FString::Printf(TEXT("  Tile.Loc = %s -> %s\n"), *Tile.Location.ToString(), *TileLocation.ToString());

  ULevel* LoadedLevel = Tile.StreamingLevel->GetLoadedLevel();
  UWorld* World = GetWorld();

  for (const FAssetData& AssetData : Tile.PendingAssetsInTile)
  {
    UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.GetAsset());
    if (!Mesh)
    {
      LM_LOG(LogCarla, Error, TEXT("Mesh %s could not be loaded in %s"), *AssetData.ObjectPath.ToString(), *Tile.Name);
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
      Output += FString::Printf(TEXT("    MeshName = %s -> %s\n"), *Mesh->GetName(), *SMActor->GetName());
      Output += FString::Printf(TEXT("      CenterBB = %s\n"), *CenterBB.ToString());
      Output += FString::Printf(TEXT("      ActorLoc = %s\n"), *(CenterBB - TileLocation).ToString());
      Output += FString::Printf(TEXT("      SMRelLoc = %s\n"), *SMComp->GetRelativeLocation().ToString());
    }
  }
  // LM_LOG(LogCarla, Warning, TEXT("%s"), *Output);

  LoadedLevel->ApplyWorldOffset(TileLocation, false);

  Tile.TilesSpawned = true;
}

#if WITH_EDITOR

FString ALargeMapManager::GenerateTileName(uint64 TileID)
{
  int32 X = (int32)(TileID >> 32);
  int32 Y = (int32)(TileID);

  // LM_LOG(LogCarla, Warning, TEXT("                GenerateTileName %d %d"), X, Y);

  return FString::Printf(TEXT("Tile_%d_%d"), X, Y);
}

void ALargeMapManager::DumpTilesTable() const
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

void ALargeMapManager::PrintMapInfo()
{
  UWorld* World = GetWorld();

  FDVector CurrentActorPosition;

  const TArray<FLevelCollection>& WorldLevelCollections = World->GetLevelCollections();
  const FLevelCollection* LevelCollection = World->GetActiveLevelCollection();
  const TArray<ULevel*>& Levels = World->GetLevels();
  const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();
  ULevel* CurrentLevel = World->GetCurrentLevel();

  FString Output = "";
  //World->WorldComposition->GetLevelBounds();
  Output += FString::Printf(TEXT("Num levels in world composition: %d\n"), World->WorldComposition->TilesStreaming.Num());
  Output += FString::Printf(TEXT("Num ue_tiles in world composition: %d\n"), World->WorldComposition->GetTilesList().Num());
  Output += FString::Printf(TEXT("Num world level collections: %d\n"), WorldLevelCollections.Num());
  // Output += FString::Printf(TEXT("Num levels in active collection: %d\n"), LevelCollection->GetLevels().Num());
  Output += FString::Printf(TEXT("Num levels: %d (%d)\n"), Levels.Num(), World->GetNumLevels());
  Output += FString::Printf(TEXT("Num streaming levels: %d\n"), StreamingLevels.Num());
  Output += FString::Printf(TEXT("Num tiles created: %d\n"), MapTiles.Num());
  Output += FString::Printf(TEXT("Current Level: %s\n"), *CurrentLevel->GetName());//, *CurrentLevel->GetFullName());

  /*
  Output += "Streaming Levels List:\n";
  for (ULevelStreaming* Level : StreamingLevels)
  {
    Output += Level->GetName() + " - " + Level->GetFullName() + " - " + Level->GetWorldAssetPackageFName().ToString() + "\n";
  }
  */
  GEngine->AddOnScreenDebugMessage(0, MsgTime, FColor::Cyan, Output);

  int LastMsgIndex = TilesDistMsgIndex;
  GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, FColor::White, TEXT("Closest tiles - Distance:"));

  ULocalPlayer* Player = GEngine->GetGamePlayer(World, 0);
  FVector ViewLocation;
  FRotator ViewRotation;
  Player->PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
  for (int i = 0; i < StreamingLevels.Num(); i++)
  {
    ULevelStreaming* Level = StreamingLevels[i];
    FVector LevelLocation = Level->LevelTransform.GetLocation();
    float Distance = FDVector::Dist(LevelLocation, CurrentActorPosition);

    FColor MsgColor = (Levels.Contains(Level->GetLoadedLevel())) ? FColor::Green : FColor::Red;
    if (Distance < (TileSide * 2.0f))
    {
      GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, MsgColor,
        FString::Printf(TEXT("%s       %.2f"), *Level->GetName(), Distance / (1000.0f * 100.0f)));
    }
    if (LastMsgIndex < MaxTilesDistMsgIndex) break;
  }

  LastMsgIndex = ClientLocMsgIndex;
  GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, FColor::White,
    FString::Printf(TEXT("Origin: %s km"), *(FDVector(CurrentOriginInt) / (1000 * 100)).ToString()) );
  GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, FColor::White,
    FString::Printf(TEXT("Actors To Consider (%d)"), ActorsToConsider.Num()) );
  for (const FActorToConsider& ActorToConsider : ActorsToConsider)
  {
    Output = "";
    float ToKm = 1000.0f * 100.0f;
    FVector TileActorLocation = ActorToConsider.Actor->GetActorLocation();
    FDVector ClientActorLocation = CurrentOriginD + FDVector(TileActorLocation);

    Output += FString::Printf(TEXT("Local Loc: %s meters\n"), *(TileActorLocation / ToKm).ToString());
    Output += FString::Printf(TEXT("Client Loc: %s km\n"), *(ClientActorLocation / ToKm).ToString());
    Output += "---------------";
    GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, PositonMsgColor, Output);

    if (LastMsgIndex > MaxClientLocMsgIndex) break;
  }
}

#endif // WITH_EDITOR
