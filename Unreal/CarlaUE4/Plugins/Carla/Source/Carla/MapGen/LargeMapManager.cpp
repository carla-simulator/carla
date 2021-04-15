// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "LargeMapManager.h"

#include "Engine/WorldComposition.h"

#include "UncenteredPivotPointMesh.h"

#include "Walker/WalkerBase.h"

#if WITH_EDITOR
#include "FileHelper.h"
#include "Paths.h"
#define LARGEMAP_LOGS 1
#else
#define LARGEMAP_LOGS 0
#endif // WITH_EDITOR

#if LARGEMAP_LOGS
#define LM_LOG(Level, Msg, ...) UE_LOG(LogCarla, Level, TEXT(Msg), ##__VA_ARGS__)
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
  /// Remove delegates
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
  /// Setup delegates
  // Origin rebase
  FCoreDelegates::PreWorldOriginOffset.AddUObject(this, &ALargeMapManager::PreWorldOriginOffset);
  FCoreDelegates::PostWorldOriginOffset.AddUObject(this, &ALargeMapManager::PostWorldOriginOffset);
  // Level added/removed from world
  FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ALargeMapManager::OnLevelAddedToWorld);
  FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &ALargeMapManager::OnLevelRemovedFromWorld);

  // Setup Origin rebase settings
  UWorldComposition* WorldComposition = World->WorldComposition;
  WorldComposition->bRebaseOriginIn3DSpace = true;
  WorldComposition->RebaseOriginDistance = RebaseOriginDistance;

}

void ALargeMapManager::PreWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin)
{
  LM_LOG(Error, "PreWorldOriginOffset Src: %s  ->  Dst: %s", *InSrcOrigin.ToString(), *InDstOrigin.ToString());
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

void ALargeMapManager::OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld)
{
  LM_LOG(Warning, "OnLevelAddedToWorld");
  //FDebug::DumpStackTraceToLog(ELogVerbosity::Log);
  FCarlaMapTile& Tile = GetCarlaMapTile(InLevel);
  SpawnAssetsInTile(Tile);
}

void ALargeMapManager::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
  LM_LOG(Warning, "OnLevelRemovedFromWorld");
  //FDebug::DumpStackTraceToLog(ELogVerbosity::Log);
  FCarlaMapTile& Tile = GetCarlaMapTile(InLevel);
  Tile.TilesSpawned = false;
}

void ALargeMapManager::OnActorSpawned(
    const FActorView& ActorView,
    const FTransform& Transform)
{
  UWorld* World = GetWorld();
  const FActorInfo* ActorInfo = ActorView.GetActorInfo();
  AActor* Actor = const_cast<AActor*>(ActorView.GetActor());
  bool IsHeroVehicle = false;

  LM_LOG(Warning, "ALargeMapManager::OnActorSpawned func %s", *Actor->GetName());

  if (IsValid(Actor) && ActorView.GetActorType() == FActorView::ActorType::Vehicle)
  { // Check if is hero vehicle

    assert(ActorInfo);

    const FActorDescription& Description = ActorInfo->Description;
    const FActorAttribute* Attribute = Description.Variations.Find("role_name");
    // If is the hero vehicle
    if(Attribute && Attribute->Value.Contains("hero"))
    {
      LM_LOG(Error, "HERO VEHICLE DETECTED");

      AddActorToConsider(Actor);

      CheckIfRebaseIsNeeded();

      UpdateTilesState();

      // Wait until the pending levels changes are finished to avoid spawning
      // the car without ground underneath
      World->FlushLevelStreaming();

      IsHeroVehicle = true;
    }
  }

  // Any other actor that its role is not "hero"
  if(!IsHeroVehicle)
  {
    UCarlaEpisode* CurrentEpisode = UCarlaStatics::GetCurrentEpisode(World);
    const FActorRegistry& ActorRegistry = CurrentEpisode->GetActorRegistry();

    // Any actor that is not the hero vehicle could possible be destroyed at some point
    // we need to store the ActorView information to be able to spawn it again if needed


    if(IsValid(Actor))
    { // Actor was spwaned succesfully
      // TODO: not dormant but not hero => GhostActor
      //       LM: Map<AActor* FGhostActor>  maybe per tile and in a tile sublevel?

      GhostActors.Add(Actor);

    }
    else
    { // Actor was spawned as dormant
      // TODO: dormant => no actor so Actorview stored per tile
      //       LM: Map<ActorId, TileID> , Tile: Map<ActorID, FDormantActor>
      //       In case of update: update Tile Map, update LM Map
    }

    //GhostActors.Add(ActorView.GetActorId(),
    //                FGhostActor(
    //                  ActorView,
    //                  FDVector(Transform.GetTranslation()),
    //                  Transform.GetRotation() ));
  }

  if (IsValid(Actor)) {
    Actor->OnDestroyed.AddDynamic(this, &ALargeMapManager::OnActorDestroyed);
  }

}

void ALargeMapManager::OnActorDestroyed(AActor* DestroyedActor)
{
  LM_LOG(Warning, "ALargeMapManager::OnActorDestroyed %s", *DestroyedActor->GetName());
}

void ALargeMapManager::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  // First we check if ghost actors are still in range or have to be converted to dormant
  CheckGhostActors();

  // Check if dormant actors have been moved to the load range
  CheckDormantActors();

  // Update map tiles, load/unload based on actors to consider (heros) position
  // Also, to avoid looping over the heros again, it checks if any actor to consider has been removed
  UpdateTilesState();

  // Remove the hero actors that doesn't exits any more from the ActorsToConsider vector
  RemovePendingActorsToRemove();

  ConvertGhostToDormantActors();

  CheckIfRebaseIsNeeded();

#if WITH_EDITOR
  if (bPrintMapInfo) PrintMapInfo();
#endif // WITH_EDITOR

}

void ALargeMapManager::GenerateMap(FString InAssetsPath)
{
  LM_LOG(Warning, "Generating Map %s ...", *InAssetsPath);

#if WITH_EDITOR
  AssetsPath = InAssetsPath;
#endif // WITH_EDITOR

  /// Retrive all the assets in the path
  TArray<FAssetData> AssetsData;
  UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), true, true);
  ObjectLibrary->LoadAssetDataFromPath(InAssetsPath);
  ObjectLibrary->GetAssetDataList(AssetsData);

  /// Generate tiles based on mesh positions
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
  LM_LOG(Warning, "GenerateMap num Tiles generated %d", MapTiles.Num());
  DumpTilesTable();
#endif // WITH_EDITOR
}

void ALargeMapManager::AddActorToUnloadedList(const FActorView& ActorView, const FTransform& Transform)
{
  // GhostActors.Add(ActorView.GetActorId(), {Transform, ActorView});
}

void ALargeMapManager::AddActorToConsider(AActor* InActor)
{
  ActorsToConsider.Add(InActor);
}

void ALargeMapManager::RemoveActorToConsider(AActor* InActor)
{
  int Index = 0;
  for (int i = 0; i < ActorsToConsider.Num(); i++)
  {
    AActor* Actor = ActorsToConsider[i];
    if (Actor == InActor)
    {
      ActorsToConsider.Remove(Actor);
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
      LM_LOG(Warning, "IsLevelOfTileLoaded Tile %s does not exist", *InTileID.ToString());
    }
    return false;
  }

  const ULevelStreamingDynamic* StreamingLevel = Tile->StreamingLevel;

  return (StreamingLevel && StreamingLevel->GetLoadedLevel());
}

FIntVector ALargeMapManager::GetTileVectorID(FVector TileLocation) const
{
  return FIntVector(TileLocation / TileSide);
}

FIntVector ALargeMapManager::GetTileVectorID(FDVector TileLocation) const
{
  return (TileLocation / TileSide).ToFIntVector();
}

FIntVector ALargeMapManager::GetTileVectorID(uint64 TileID) const
{
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

void ALargeMapManager::UpdateTilesState()
{
  TSet<uint64> TilesToConsider;

  // Loop over ActorsToConsider to update the state of the map tiles
  // if the actor is not valid will be removed
  for (AActor* Actor : ActorsToConsider)
  {
    if (IsValid(Actor))
    {
      GetTilesToConsider(Actor, TilesToConsider);
    }
    else
    {
      ActorsToRemove.Add(Actor);
    }
  }

  TSet<uint64> TilesToBeVisible;
  TSet<uint64> TilesToHidde;
  GetTilesThatNeedToChangeState(TilesToConsider, TilesToBeVisible, TilesToHidde);

  UpdateTileState(TilesToBeVisible, true, true, true);

  UpdateTileState(TilesToHidde, false, false, false);

  UpdateCurrentTilesLoaded(TilesToBeVisible, TilesToHidde);

}

void ALargeMapManager::RemovePendingActorsToRemove()
{
  // Discard removed actors
  for (AActor* ActorToRemove : ActorsToRemove)
  {
    ActorsToConsider.Remove(ActorToRemove);
  }
  ActorsToRemove.Reset();
}

void ALargeMapManager::CheckGhostActors()
{
  // Check if they have to be destroyed
  for(AActor* Actor : GhostActors)
  {
    check(Actor);

    FVector ActorLocation = Actor->GetActorLocation();

    if( ActorLocation.SizeSquared() > ActorStreamingDistanceSquared)
    {
      // Save to temporal container. Later will be converted to dormant
      GhostToDormantActors.Add(Actor);
    }
  }
}

void ALargeMapManager::ConvertGhostToDormantActors()
{
  UWorld* World = GetWorld();
  UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(World);

  // These actors are on dormant state so remove them from ghost actors
  // But save them on the dormant array first
  for(AActor* Actor : GhostToDormantActors)
  {
    check(IsValid(Actor));

    // To dormant state
    FActorView ActorView = CarlaEpisode->FindActor(Actor);
    ActorView.SetActorState(carla::rpc::ActorState::Dormant);

    // Save current location and rotation
    FActorInfo* ActorInfo = const_cast<FActorInfo*>(ActorView.GetActorInfo());
    ActorInfo->Location = CurrentOriginD + Actor->GetActorLocation();
    ActorInfo->Rotation = Actor->GetActorQuat();

    CarlaEpisode->DestroyActor(Actor);

    // Need the ID of the dormant actor and save it
    DormantActors.Add(ActorView.GetActorId());

    GhostActors.Remove(Actor);
  }

  GhostToDormantActors.Reset();
}

void ALargeMapManager::CheckDormantActors()
{
  UWorld* World = GetWorld();
  UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(World);

  for(FActorView::IdType Id : DormantActors)
  {
    FActorView ActorView = CarlaEpisode->FindActor(Id);

    check(ActorView.IsDormant());// if is not Dormant something is very wrong
    const FActorInfo* ActorInfo = ActorView.GetActorInfo();

    FDVector ActorLocation = ActorInfo->Location;

    if(ActorLocation.SizeSquared() < ActorStreamingDistanceSquared)
    {
      LM_LOG(Warning, "Need to spawn a dormant actor");

      // TODO: spawn the actor again
      FTransform Transform(ActorInfo->Rotation, ActorLocation.ToFVector());
      TPair<EActorSpawnResultStatus, FActorView> Result =
        CarlaEpisode->SpawnActorWithInfo(Transform, ActorInfo->Description, Id);
      if(EActorSpawnResultStatus::Success == Result.Key)
      {
        // Add the actor to GhostActors
        GhostActors.Add(ActorView.GetActor());
        DormantToGhostActors.Add(Id);
      }
    }
  }
}

void ALargeMapManager::ConvertDormantToGhostActors()
{
  for(FActorView::IdType Id : DormantToGhostActors)
  {
    DormantActors.Remove(Id);
  }
  DormantToGhostActors.Reset();
}

void ALargeMapManager::CheckIfRebaseIsNeeded()
{
  if(ActorsToConsider.Num() > 0)
  {
    UWorld* World = GetWorld();
    UWorldComposition* WorldComposition = World->WorldComposition;
    // TODO: consider multiple hero vehicles for rebasing
    AActor* ActorToConsider = ActorsToConsider[0];
    if( IsValid(ActorToConsider) )
    {
      FVector ActorLocation = ActorToConsider->GetActorLocation();
      FIntVector ILocation = FIntVector(ActorLocation.X, ActorLocation.Y, ActorLocation.Z);
      //WorldComposition->EvaluateWorldOriginLocation(ActorToConsider->GetActorLocation());
      if (ActorLocation.SizeSquared() > FMath::Square(RebaseOriginDistance) )
      {
        LM_LOG(Error, "Rebasing from %s to %s", *CurrentOriginInt.ToString(), *(ILocation + CurrentOriginInt).ToString());
        World->SetNewWorldOrigin(ILocation + CurrentOriginInt);
      }
    }
  }
}

void ALargeMapManager::GetTilesToConsider(const AActor* ActorToConsider,
                                          TSet<uint64>& OutTilesToConsider)
{
  check(ActorToConsider);
  // World location
  FDVector ActorLocation = CurrentOriginD + ActorToConsider->GetActorLocation();

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
        continue; // Tile does not exist, discard
      }

      // Calculate distance between actor and tile
      float Distance2 = FDVector::DistSquared(Tile->Location, ActorLocation);

      // Load level if we are in range
      if (Distance2 < LayerStreamingDistanceSquared)
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
FString ALargeMapManager::GenerateTileName(uint64 TileID)
{
  int32 X = (int32)(TileID >> 32);
  int32 Y = (int32)(TileID);
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
  Output += FString::Printf(TEXT("Num levels in world composition: %d\n"), World->WorldComposition->TilesStreaming.Num());
  Output += FString::Printf(TEXT("Num levels loaded: %d\n"), Levels.Num(), World->GetNumLevels());
  GEngine->AddOnScreenDebugMessage(0, MsgTime, FColor::Cyan, Output);

  int LastMsgIndex = TilesDistMsgIndex;
  GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, FColor::White, TEXT("Closest tiles - Distance:"));

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
    FString::Printf(TEXT("Origin: %s km"), *(FDVector(CurrentOriginInt) / (1000.0 * 100.0)).ToString()) );
  GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, FColor::White,
     FString::Printf(TEXT("Num ghost actors (%d)"), GhostActors.Num()) );
  GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, FColor::White,
     FString::Printf(TEXT("Num dormant actors (%d)"), DormantActors.Num()));
  GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, FColor::White,
    FString::Printf(TEXT("Actors To Consider (%d)"), ActorsToConsider.Num()));
  for (const AActor* Actor : ActorsToConsider)
  {
    if (IsValid(Actor))
    {
      Output = "";
      float ToKm = 1000.0f * 100.0f;
      FVector TileActorLocation = Actor->GetActorLocation();
      FDVector ClientActorLocation = CurrentOriginD + FDVector(TileActorLocation);

      Output += FString::Printf(TEXT("Local Loc: %s meters\n"), *(TileActorLocation / ToKm).ToString());
      Output += FString::Printf(TEXT("Client Loc: %s km\n"), *(ClientActorLocation / ToKm).ToString());
      Output += "---------------";
      GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, PositonMsgColor, Output);

      if (LastMsgIndex > MaxClientLocMsgIndex) break;
    }
  }
}

#endif // WITH_EDITOR

