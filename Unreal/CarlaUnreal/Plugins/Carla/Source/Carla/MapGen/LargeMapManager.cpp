// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "LargeMapManager.h"

#include "Game/CarlaStatics.h"
#include "Actor/ActorRegistry.h"
#include "Game/CarlaEpisode.h"

#include "UncenteredPivotPointMesh.h"

#include "Walker/WalkerBase.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Vehicle/CustomTerrainPhysicsComponent.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/WorldComposition.h"
#include "Engine/ObjectLibrary.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/EngineTypes.h"
#include "Components/PrimitiveComponent.h"
#include "Landscape.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "LandscapeComponent.h"
#include <util/ue-header-guard-end.h>

#define LARGEMAP_LOGS 1

#if LARGEMAP_LOGS
#define LM_LOG(Level, Msg, ...) UE_LOG(LogCarla, Level, TEXT(Msg), ##__VA_ARGS__)
#else
#define LM_LOG(...)
#endif

// Sets default values
ALargeMapManager::ALargeMapManager()
{
  PrimaryActorTick.bCanEverTick = true;
  // PrimaryActorTick.TickInterval = TickInterval;
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
  RegisterTilesInWorldComposition();

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

  LayerStreamingDistanceSquared = LayerStreamingDistance * LayerStreamingDistance;
  ActorStreamingDistanceSquared = ActorStreamingDistance * ActorStreamingDistance;
  RebaseOriginDistanceSquared = RebaseOriginDistance * RebaseOriginDistance;

  // Look for terramechanics actor
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
  for(auto CurrentActor : FoundActors)
  {
    if( CurrentActor->FindComponentByClass( UCustomTerrainPhysicsComponent::StaticClass() ) != nullptr )
    {
      bHasTerramechanics = true;
      break;
    }
  }

  // Get spectator
  APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
  if(PlayerController)
  {
    Spectator = PlayerController->GetPawnOrSpectator();
  }
}

void ALargeMapManager::PreWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin)
{
  LM_LOG(Log, "PreWorldOriginOffset Src: %s  ->  Dst: %s", *InSrcOrigin.ToString(), *InDstOrigin.ToString());
}

void ALargeMapManager::PostWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::PostWorldOriginOffset);
  CurrentOriginInt = InDstOrigin;
  CurrentOriginD = FDVector(InDstOrigin);

  UWorld* World = GetWorld();
  UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(World);
  CarlaEpisode->SetCurrentMapOrigin(CurrentOriginInt);

#if WITH_EDITOR
  GEngine->AddOnScreenDebugMessage(66, MsgTime, FColor::Yellow,
    FString::Printf(TEXT("Src: %s  ->  Dst: %s"), *InSrcOrigin.ToString(), *InDstOrigin.ToString()));
  LM_LOG(Log, "PostWorldOriginOffset Src: %s  ->  Dst: %s", *InSrcOrigin.ToString(), *InDstOrigin.ToString());

  // This is just to update the color of the msg with the same as the closest map
  const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();
  FColor LevelColor = FColor::White;
  float MinDistance = 10000000.0f;
  for (const auto& TilePair : MapTiles)
  {
    const FCarlaMapTile& Tile = TilePair.Value;
    const ULevelStreaming* Level = Tile.StreamingLevel;
    FVector LevelLocation = Tile.Location;
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
  ATagger::TagActorsInLevel(*InLevel, true);


  //FDebug::DumpStackTraceToLog(ELogVerbosity::Log);
}

void ALargeMapManager::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
  LM_LOG(Warning, "OnLevelRemovedFromWorld");
  //FDebug::DumpStackTraceToLog(ELogVerbosity::Log);
  FCarlaMapTile& Tile = GetCarlaMapTile(InLevel);
  Tile.TilesSpawned = false;
}

void ALargeMapManager::RegisterInitialObjects()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::RegisterInitialObjects);
  UWorld* World = GetWorld();
  UCarlaEpisode* CurrentEpisode = UCarlaStatics::GetCurrentEpisode(World);
  const FActorRegistry& ActorRegistry = CurrentEpisode->GetActorRegistry();
  for (const auto& CarlaActorPair : ActorRegistry)
  {
    if (CarlaActorPair.Value->GetActorInfo()->Description.Id == "spectator")
    {
      continue;
    }
    OnActorSpawned(*CarlaActorPair.Value.Get());
  }
}

void ALargeMapManager::OnActorSpawned(
    const FCarlaActor& CarlaActor)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::OnActorSpawned);
  UWorld* World = GetWorld();
  const FActorInfo* ActorInfo = CarlaActor.GetActorInfo();
  AActor* Actor = const_cast<AActor*>(CarlaActor.GetActor());
  bool IsHeroVehicle = false;

  // LM_LOG(Warning, "ALargeMapManager::OnActorSpawned func %s %s", *Actor->GetName(), *Actor->GetTranslation().ToString());

  if (Actor)
  { // Check if is hero vehicle

    assert(ActorInfo);

    const FActorDescription& Description = ActorInfo->Description;
    const FActorAttribute* Attribute = Description.Variations.Find("role_name");
    // If is the hero vehicle
    if(Attribute && (Attribute->Value.Contains("hero") || Attribute->Value.Contains("ego_vehicle")))
    {
      LM_LOG(Log, "HERO VEHICLE DETECTED");

      if (ActorsToConsider.Num() == 1 && ActorsToConsider.Contains(Spectator))
      {
        ActorsToConsider.Reset();
      }
      ActorsToConsider.Add(Actor);

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
    // we need to store the CarlaActor information to be able to spawn it again if needed

    if(IsValid(Actor))
    { // Actor was spwaned succesfully
      // TODO: not dormant but not hero => ActiveActor
      //       LM: Map<AActor* FActiveActor>  maybe per tile and in a tile sublevel?

      LM_LOG(Log, "ACTIVE VEHICLE DETECTED");
      ActiveActors.Add(CarlaActor.GetActorId());
    }
    else
    { // Actor was spawned as dormant
      // TODO: dormant => no actor so Actorview stored per tile
      //       LM: Map<ActorId, TileID> , Tile: Map<ActorID, FDormantActor>
      //       In case of update: update Tile Map, update LM Map
      LM_LOG(Log, "DORMANT VEHICLE DETECTED");
      DormantActors.Add(CarlaActor.GetActorId());
    }
  }

  if (IsValid(Actor)) {
    Actor->OnDestroyed.AddDynamic(this, &ALargeMapManager::OnActorDestroyed);
    FVector GlobalPosition = LocalToGlobalLocation(Actor->GetActorLocation());
    LM_LOG(Warning, "Actor Spawned at %s", *GlobalPosition.ToString());
  }

}

ACarlaWheeledVehicle* ALargeMapManager::GetHeroVehicle()
{
  if (ActorsToConsider.Num() > 0)
  {
    ACarlaWheeledVehicle* Hero = Cast<ACarlaWheeledVehicle>(ActorsToConsider[0]);
    if (IsValid(Hero))
      return Hero;
  }
  return nullptr;
}

void ALargeMapManager::OnActorDestroyed(AActor* DestroyedActor)
{
  LM_LOG(Warning, "ALargeMapManager::OnActorDestroyed %s", *DestroyedActor->GetName());

  UWorld* World = GetWorld();
  UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(World);
  FCarlaActor* CarlaActor = CarlaEpisode->FindCarlaActor(DestroyedActor);
  if (CarlaActor)
    const FActorInfo* ActorInfo = CarlaActor->GetActorInfo();

  // Hero has been removed?
  //

}

void ALargeMapManager::SetTile0Offset(const FVector& Offset)
{
  Tile0Offset = Offset;
}

void ALargeMapManager::SetTileSize(float Size)
{
  TileSide = Size;
}

float ALargeMapManager::GetTileSize()
{
  return TileSide;
}

FVector ALargeMapManager::GetTile0Offset()
{
  return Tile0Offset;
}

void ALargeMapManager::SetLayerStreamingDistance(float Distance)
{
  LayerStreamingDistance = Distance;
  LayerStreamingDistanceSquared =
      LayerStreamingDistance*LayerStreamingDistance;
}

void ALargeMapManager::SetActorStreamingDistance(float Distance)
{
  ActorStreamingDistance = Distance;
  ActorStreamingDistanceSquared =
      ActorStreamingDistance*ActorStreamingDistance;
}

float ALargeMapManager::GetLayerStreamingDistance() const
{
  return LayerStreamingDistance;
}

float ALargeMapManager::GetActorStreamingDistance() const
{
  return ActorStreamingDistance;
}

FTransform ALargeMapManager::GlobalToLocalTransform(const FTransform& InTransform) const
{
  return FTransform(
        InTransform.GetRotation(),
        InTransform.GetLocation() - CurrentOriginD.ToFVector(),
        InTransform.GetScale3D());
}

FVector ALargeMapManager::GlobalToLocalLocation(const FVector& InLocation) const
{
  return InLocation - CurrentOriginD.ToFVector();
}

FTransform ALargeMapManager::LocalToGlobalTransform(const FTransform& InTransform) const
{
  return FTransform(
        InTransform.GetRotation(),
        CurrentOriginD.ToFVector() + InTransform.GetLocation(),
        InTransform.GetScale3D());
}

FVector ALargeMapManager::LocalToGlobalLocation(const FVector& InLocation) const
{
  return CurrentOriginD.ToFVector() + InLocation;
}

float tick_execution_time = 0;
uint64_t num_ticks = 0;
void ALargeMapManager::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  // Update map tiles, load/unload based on actors to consider (heros) position
  // Also, to avoid looping over the heros again, it checks if any actor to consider has been removed
  UpdateTilesState();

  // Check if active actors are still in range and inside a loaded tile or have to be converted to dormant
  CheckActiveActors();

  // Check if dormant actors have been moved to the load range
  CheckDormantActors();

  // Remove the hero actors that doesn't exits any more from the ActorsToConsider vector
  RemovePendingActorsToRemove();

  ConvertActiveToDormantActors();

  ConvertDormantToActiveActors();

  CheckIfRebaseIsNeeded();

#if WITH_EDITOR
  if (bPrintMapInfo) PrintMapInfo();
#endif // WITH_EDITOR

}

void ALargeMapManager::GenerateLargeMap() {
  GenerateMap(LargeMapTilePath);
}

void ALargeMapManager::GenerateMap(FString InAssetsPath)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::GenerateMap);
  LM_LOG(Warning, "Generating Map %s ...", *InAssetsPath);
  ClearWorldAndTiles();

  AssetsPath = InAssetsPath;

  /// Retrive all the assets in the path
  TArray<FAssetData> AssetsData;
  UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), true, true);
  ObjectLibrary->LoadAssetDataFromPath(InAssetsPath);
  ObjectLibrary->GetAssetDataList(AssetsData);

  /// Generate tiles based on mesh positions
  UWorld* World = GetWorld();
  MapTiles.Reset();
  for (const FAssetData& AssetData : AssetsData)
  {
    #if WITH_EDITOR
      // LM_LOG(Warning, "Loading asset name: %s", *(AssetData.AssetName.ToString()));
      // LM_LOG(Warning, "Asset class: %s", *(AssetData.AssetClass.ToString()));
    #endif
    FString TileName = AssetData.AssetName.ToString();
    if (!TileName.Contains("_Tile_"))
    {
      continue;
    }
    FString TileName_X = "";
    FString TileName_Y = "";
    size_t i = TileName.Len()-1;
    for (; i > 0; i--) {
      TCHAR character = TileName[i];
      if (character == '_') {
        break;
      }
      TileName_Y = FString::Chr(character) + TileName_Y;
    }
    i--;
    for (; i > 0; i--) {
      TCHAR character = TileName[i];
      if (character == '_') {
        break;
      }
      TileName_X = FString::Chr(character) + TileName_X;
    }
    FIntVector TileVectorID = FIntVector(FCString::Atoi(*TileName_X), FCString::Atoi(*TileName_Y), 0);
    #if WITH_EDITOR
      // LM_LOG(Warning, "Tile: %d, %d", TileVectorID.X, TileVectorID.Y);
    #endif
    TileID TileId = GetTileID(TileVectorID);
    LoadCarlaMapTile(InAssetsPath + "/" + AssetData.AssetName.ToString(), TileId);
  }
  ObjectLibrary->ConditionalBeginDestroy();
#if WITH_EDITOR
  GEngine->ForceGarbageCollection(true);
#endif
  ActorsToConsider.Reset();
  if (SpectatorAsEgo && Spectator)
  {
    ActorsToConsider.Add(Spectator);
  }

#if WITH_EDITOR
  LM_LOG(Warning, "GenerateMap num Tiles generated %d", MapTiles.Num());
  DumpTilesTable();
#endif // WITH_EDITOR
}

void ALargeMapManager::GenerateMap(TArray<TPair<FString, FIntVector>> MapPathsIds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::GenerateMap);
  LM_LOG(Warning, "Generating Large Map");
  ClearWorldAndTiles();

  for (TPair<FString, FIntVector>& PathId : MapPathsIds)
  {
    FIntVector& TileVectorID = PathId.Value;
    FString& Path = PathId.Key;
    TileID TileId = GetTileID(TileVectorID);
    LoadCarlaMapTile(Path, TileId);
  }

  #if WITH_EDITOR
  LM_LOG(Warning, "GenerateMap num Tiles generated %d", MapTiles.Num());
  #endif // WITH_EDITOR
}

void ALargeMapManager::ClearWorldAndTiles()
{
  MapTiles.Empty();
}

void ALargeMapManager::RegisterTilesInWorldComposition()
{
  UWorld* World = GetWorld();
  UWorldComposition* WorldComposition = World->WorldComposition;
  World->ClearStreamingLevels();
  WorldComposition->TilesStreaming.Empty();
  WorldComposition->GetTilesList().Empty();

  for (auto& It : MapTiles)
  {
    ULevelStreamingDynamic* StreamingLevel = It.Value.StreamingLevel;
    World->AddStreamingLevel(StreamingLevel);
    WorldComposition->TilesStreaming.Add(StreamingLevel);
  }
}

// TODO: maybe remove this, I think I will not need it any more
void ALargeMapManager::AddActorToUnloadedList(const FCarlaActor& CarlaActor, const FTransform& Transform)
{
  // ActiveActors.Add(CarlaActor.GetActorId(), {Transform, CarlaActor});
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
  TileID TileID = GetTileID(InTileID);

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
  FIntVector VectorId = FIntVector(
      (TileLocation -
      (Tile0Offset - FVector(0.5f*TileSide,-0.5f*TileSide, 0) + LocalTileOffset))
      / TileSide);
  VectorId.Y *= -1;
  return VectorId;
}

FIntVector ALargeMapManager::GetTileVectorID(FDVector TileLocation) const
{
  FIntVector VectorId = (
      (TileLocation -
      (Tile0Offset - FVector(0.5f*TileSide,-0.5f*TileSide, 0) + LocalTileOffset))
      / TileSide).ToFIntVector();
  VectorId.Y *= -1;
  return VectorId;
}

FIntVector ALargeMapManager::GetTileVectorID(TileID TileID) const
{
  return FIntVector{
    (int32)(TileID >> 32),
    (int32)(TileID & (int32)(~0)),
    0
  };
}

FVector ALargeMapManager::GetTileLocation(TileID TileID) const
{
  FIntVector VTileId = GetTileVectorID(TileID);
  return GetTileLocation(VTileId);
}

FVector ALargeMapManager::GetTileLocation(FIntVector TileVectorID) const
{
  TileVectorID.Y *= -1;
  return FVector(TileVectorID)* TileSide + Tile0Offset;
}

FDVector ALargeMapManager::GetTileLocationD(TileID TileID) const
{
  FIntVector VTileId = GetTileVectorID(TileID);
  return GetTileLocationD(VTileId);
}

FDVector ALargeMapManager::GetTileLocationD(FIntVector TileVectorID) const
{
  TileVectorID.Y *= -1;
  return FDVector(TileVectorID) * TileSide + Tile0Offset;
}

ALargeMapManager::TileID ALargeMapManager::GetTileID(FIntVector TileVectorID) const
{
  int64 X = ((int64)(TileVectorID.X) << 32);
  int64 Y = (int64)(TileVectorID.Y) & 0x00000000FFFFFFFF;
  return (X | Y);
}

ALargeMapManager::TileID ALargeMapManager::GetTileID(FVector TileLocation) const
{
  FIntVector TileID = GetTileVectorID(TileLocation);
  return GetTileID(TileID);
}

ALargeMapManager::TileID ALargeMapManager::GetTileID(FDVector TileLocation) const
{
  FIntVector TileID = GetTileVectorID(TileLocation);
  return GetTileID(TileID);
}

FCarlaMapTile* ALargeMapManager::GetCarlaMapTile(FVector Location)
{
  TileID TileID = GetTileID(Location);
  return GetCarlaMapTile(TileID);
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

FCarlaMapTile& ALargeMapManager::GetCarlaMapTile(FIntVector TileVectorID)
{
  TileID TileID = GetTileID(TileVectorID);
  FCarlaMapTile* Tile = MapTiles.Find(TileID);
  return *Tile;
}

FCarlaMapTile* ALargeMapManager::GetCarlaMapTile(TileID TileID)
{
  FCarlaMapTile* Tile = MapTiles.Find(TileID);
  return Tile;
}

FCarlaMapTile& ALargeMapManager::LoadCarlaMapTile(FString TileMapPath, TileID TileId) {
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::LoadCarlaMapTile);
  // Need to generate a new Tile
  FCarlaMapTile NewTile;
  // 1 - Calculate the Tile position
  FIntVector VTileID = GetTileVectorID(TileId);
  NewTile.Location = GetTileLocation(TileId);
  NewTile.Name = TileMapPath;

  // 3 - Generate the StreamLevel
  FVector TileLocation = NewTile.Location;
  FString TileName = NewTile.Name;
  UWorld* World = GetWorld();
  UWorldComposition* WorldComposition = World->WorldComposition;

  FString FullName = TileMapPath;
  FString PackageFileName = FullName;
  FString LongLevelPackageName = FPackageName::FilenameToLongPackageName(PackageFileName);
  FString UniqueLevelPackageName = LongLevelPackageName;

  ULevelStreamingDynamic* StreamingLevel = NewObject<ULevelStreamingDynamic>(World, *TileName);
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

  if (!FPackageName::DoesPackageExist(FullName, &PackageFileName))
  {
    LM_LOG(Error, "Level does not exist in package with FullName variable -> %s", *FullName);
  }

  if (!FPackageName::DoesPackageExist(LongLevelPackageName, &PackageFileName))
  {
    LM_LOG(Error, "Level does not exist in package with LongLevelPackageName variable -> %s", *LongLevelPackageName);
  }

  //Actual map package to load
  StreamingLevel->PackageNameToLoad = *LongLevelPackageName;

  NewTile.StreamingLevel = StreamingLevel;

  // 4 - Add it to the map
  return MapTiles.Add(TileId, NewTile);
}

void ALargeMapManager::UpdateTilesState()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::UpdateTilesState);
  TSet<TileID> TilesToConsider;

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

  TSet<TileID> TilesToBeVisible;
  TSet<TileID> TilesToHidde;
  GetTilesThatNeedToChangeState(TilesToConsider, TilesToBeVisible, TilesToHidde);

  UpdateTileState(TilesToBeVisible, true, true, true);

  UpdateTileState(TilesToHidde, false, false, false);

  UpdateCurrentTilesLoaded(TilesToBeVisible, TilesToHidde);

}

void ALargeMapManager::RemovePendingActorsToRemove()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::RemovePendingActorsToRemove);
  if(ActorsToRemove.Num() > 0 || ActivesToRemove.Num() > 0)
  {
    LM_LOG(Log, "ActorsToRemove %d ActivesToRemove %d", ActorsToRemove.Num(), ActivesToRemove.Num());
  }

  for (AActor* ActorToRemove : ActorsToRemove)
  {
    ActorsToConsider.Remove(ActorToRemove);
  }
  if(ActorsToConsider.Num() == 0 && SpectatorAsEgo && Spectator)
  {
    ActorsToConsider.Add(Spectator);
  }
  ActorsToRemove.Reset();


  for (FCarlaActor::IdType ActorToRemove : ActivesToRemove)
  {
    ActiveActors.Remove(ActorToRemove);
  }
  ActivesToRemove.Reset();

  for(FCarlaActor::IdType Id : DormantsToRemove)
  {
    DormantActors.Remove(Id);
  }
  DormantsToRemove.Reset();
}

void ALargeMapManager::CheckActiveActors()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::CheckActiveActors);
  UWorld* World = GetWorld();
  UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(World);
  // Check if they have to be destroyed
  for(FCarlaActor::IdType Id : ActiveActors)
  {
    FCarlaActor* View = CarlaEpisode->FindCarlaActor(Id);
    if (View)
    {
      AActor * Actor = View->GetActor();
      FVector RelativeLocation = Actor->GetActorLocation();
      FDVector WorldLocation = CurrentOriginD + RelativeLocation;

      if(!IsTileLoaded(WorldLocation))
      {
        // Save to temporal container. Later will be converted to dormant
        ActiveToDormantActors.Add(Id);
        ActivesToRemove.Add(Id);
        continue;
      }

      for(AActor* HeroActor : ActorsToConsider)
      {
        FVector HeroLocation = HeroActor->GetActorLocation();

        float DistanceSquared = (RelativeLocation - HeroLocation).SizeSquared();

        if (DistanceSquared > ActorStreamingDistanceSquared && View->GetActorType() != FCarlaActor::ActorType::Sensor)
        {
          // Save to temporal container. Later will be converted to dormant
          ActiveToDormantActors.Add(Id);
          ActivesToRemove.Add(Id);
        }
      }
    }
    else
    {
      LM_LOG(Warning, "CheckActiveActors Actor does not exist -> Remove actor");
      ActivesToRemove.Add(Id);
    }
  }
}

void ALargeMapManager::ConvertActiveToDormantActors()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::ConvertActiveToDormantActors);
  UWorld* World = GetWorld();
  UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(World);

  // These actors are on dormant state so remove them from active actors
  // But save them on the dormant array first
  for(FCarlaActor::IdType Id : ActiveToDormantActors)
  {
    // To dormant state
    CarlaEpisode->PutActorToSleep(Id);

    LM_LOG(Warning, "Converting Active To Dormant... %d", Id);

    // Need the ID of the dormant actor and save it
    DormantActors.Add(Id);
  }

  ActiveToDormantActors.Reset();
}

void ALargeMapManager::CheckDormantActors()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::CheckDormantActors);
  UWorld* World = GetWorld();
  UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(World);


  for(FCarlaActor::IdType Id : DormantActors)
  {
    FCarlaActor* CarlaActor = CarlaEpisode->FindCarlaActor(Id);

    // If the Ids don't match, the actor has been removed
    if(!CarlaActor)
    {
      LM_LOG(Log, "CheckDormantActors Carla Actor %d not found", Id);
      DormantsToRemove.Add(Id);
      continue;
    }
    if(CarlaActor->GetActorId() != Id)
    {
      LM_LOG(Warning, "CheckDormantActors IDs doesn't match!! Wanted = %d Received = %d", Id, CarlaActor->GetActorId());
      DormantsToRemove.Add(Id);
      continue;
    }
    if (!CarlaActor->IsDormant())
    {
      LM_LOG(Warning, "CheckDormantActors Carla Actor %d is not dormant", Id);
      DormantsToRemove.Add(Id);
      continue;
    }

    const FActorData* ActorData = CarlaActor->GetActorData();

    for(AActor* Actor : ActorsToConsider)
    {
      FVector HeroLocation = Actor->GetActorLocation();

      FDVector WorldLocation = ActorData->Location;
      FDVector RelativeLocation = WorldLocation - CurrentOriginD;

      float DistanceSquared = (RelativeLocation - HeroLocation).SizeSquared();

      if(DistanceSquared < ActorStreamingDistanceSquared && IsTileLoaded(WorldLocation))
      {
        DormantToActiveActors.Add(Id);
        DormantsToRemove.Add(Id);
        break;
      }
    }
  }
}

void ALargeMapManager::ConvertDormantToActiveActors()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::ConvertDormantToActiveActors);
  UWorld* World = GetWorld();
  UCarlaEpisode* CarlaEpisode = UCarlaStatics::GetCurrentEpisode(World);

  for(FCarlaActor::IdType Id : DormantToActiveActors)
  {
    LM_LOG(Warning, "Converting %d Dormant To Active", Id);

    CarlaEpisode->WakeActorUp(Id);

    FCarlaActor* View = CarlaEpisode->FindCarlaActor(Id);

    if (View->IsActive()){
      LM_LOG(Warning, "Spawning dormant at %s\n\tOrigin: %s\n\tRel. location: %s", \
        *((CurrentOriginD + View->GetActor()->GetActorLocation()).ToString()), \
        *(CurrentOriginD.ToString()), \
        *((View->GetActor()->GetActorLocation()).ToString()) \
      );
      ActiveActors.Add(Id);
    }
    else
    {
      LM_LOG(Warning, "Actor %d could not be woken up, keeping sleep state", Id);
      DormantActors.Add(Id);
    }
  }
  DormantToActiveActors.Reset();
}

void ALargeMapManager::CheckIfRebaseIsNeeded()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::CheckIfRebaseIsNeeded);
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
      if (ActorLocation.SizeSquared() > FMath::Square(RebaseOriginDistance) )
      {
        TileID TileId = GetTileID(CurrentOriginD + ActorLocation);
        FVector NewOrigin = GetTileLocation(TileId);
        World->SetNewWorldOrigin(FIntVector(NewOrigin));
      }
    }
  }
}

void ALargeMapManager::GetTilesToConsider(const AActor* ActorToConsider,
                                          TSet<TileID>& OutTilesToConsider)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::GetTilesToConsider);
  check(ActorToConsider);
  // World location
  FDVector ActorLocation = CurrentOriginD + ActorToConsider->GetActorLocation();

  // Calculate Current Tile
  FIntVector CurrentTile = GetTileVectorID(ActorLocation);

  // Calculate tile bounds
  FDVector UpperPos = ActorLocation + FDVector(LayerStreamingDistance,LayerStreamingDistance,0);
  FDVector LowerPos = ActorLocation + FDVector(-LayerStreamingDistance,-LayerStreamingDistance,0);
  FIntVector UpperTileId = GetTileVectorID(UpperPos);
  FIntVector LowerTileId = GetTileVectorID(LowerPos);
  for (int Y = UpperTileId.Y; Y <= LowerTileId.Y; Y++)
  {
    for (int X = LowerTileId.X; X <= UpperTileId.X; X++)
    {
      // I don't check the bounds of the Tile map, if the Tile does not exist
      // I just simply discard it
      FIntVector TileToCheck = FIntVector(X, Y, 0);

      TileID TileID = GetTileID(TileToCheck);
      FCarlaMapTile* Tile = MapTiles.Find(TileID);
      if (!Tile)
      {
        // LM_LOG(Warning, "Requested tile %d, %d  but tile was not found", TileToCheck.X, TileToCheck.Y);
        continue; // Tile does not exist, discard
      }

        OutTilesToConsider.Add(TileID);
    }
  }
}

void ALargeMapManager::GetTilesThatNeedToChangeState(
  const TSet<TileID>& InTilesToConsider,
  TSet<TileID>& OutTilesToBeVisible,
  TSet<TileID>& OutTilesToHidde)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::GetTilesThatNeedToChangeState);
  OutTilesToBeVisible = InTilesToConsider.Difference(CurrentTilesLoaded);
  OutTilesToHidde = CurrentTilesLoaded.Difference(InTilesToConsider);
}

void ALargeMapManager::UpdateTileState(
  const TSet<TileID>& InTilesToUpdate,
  bool InShouldBlockOnLoad,
  bool InShouldBeLoaded,
  bool InShouldBeVisible)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::UpdateTileState);
  UWorld* World = GetWorld();
  UWorldComposition* WorldComposition = World->WorldComposition;

  // Gather all the locations of the levels to load
  for (const TileID TileID : InTilesToUpdate)
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
  const TSet<TileID>& InTilesToBeVisible,
  const TSet<TileID>& InTilesToHidde)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ALargeMapManager::UpdateCurrentTilesLoaded);
  for (const TileID TileID : InTilesToHidde)
  {
    CurrentTilesLoaded.Remove(TileID);
  }

  for (const TileID TileID : InTilesToBeVisible)
  {
    CurrentTilesLoaded.Add(TileID);
  }
}

FString ALargeMapManager::GenerateTileName(TileID TileID)
{
  int32 X = (int32)(TileID >> 32);
  int32 Y = (int32)(TileID);
  return FString::Printf(TEXT("Tile_%d_%d"), X, Y);
}

FString ALargeMapManager::TileIDToString(TileID TileID)
{
  int32 X = (int32)(TileID >> 32);
  int32 Y = (int32)(TileID);
  return FString::Printf(TEXT("%d_%d"), X, Y);
}

void ALargeMapManager::DumpTilesTable() const
{
  FString FileContent = "";
  FileContent += FString::Printf(TEXT("LargeMapManager state\n"));

  FileContent += FString::Printf(TEXT("Tile:\n"));
  FileContent += FString::Printf(TEXT("ID\tName\tLocation\n"));
  for (auto& It : MapTiles)
  {
    const FCarlaMapTile& Tile = It.Value;
    FileContent += FString::Printf(TEXT("  %ld\t%s\t%s\n"), It.Key, *Tile.Name, *Tile.Location.ToString());
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
#if WITH_EDITOR
  UWorld* World = GetWorld();

  FDVector CurrentActorPosition;
  if (ActorsToConsider.Num() > 0)
    CurrentActorPosition = CurrentOriginD + ActorsToConsider[0]->GetActorLocation();

  const TArray<FLevelCollection>& WorldLevelCollections = World->GetLevelCollections();
  const FLevelCollection* LevelCollection = World->GetActiveLevelCollection();
  const TArray<ULevel*>& Levels = World->GetLevels();
  const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();
  ULevel* CurrentLevel = World->GetCurrentLevel();

  FString Output = "";
  Output += FString::Printf(TEXT("Num levels in world composition: %d\n"), World->WorldComposition->TilesStreaming.Num());
  Output += FString::Printf(TEXT("Num levels loaded: %d\n"), Levels.Num() );
  Output += FString::Printf(TEXT("Num tiles loaded: %d\n"), CurrentTilesLoaded.Num() );
  Output += FString::Printf(TEXT("Tiles loaded: [ "));
  for(TileID& TileId : CurrentTilesLoaded)
  {
    Output += FString::Printf(TEXT("%s, "), *TileIDToString(TileId));
  }
  Output += FString::Printf(TEXT("]\n"));
  GEngine->AddOnScreenDebugMessage(0, MsgTime, FColor::Cyan, Output);

  int LastMsgIndex = TilesDistMsgIndex;
  GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, FColor::White,
    FString::Printf(TEXT("\nActor Global Position: %s km"), *(FDVector(CurrentActorPosition) / (1000.0 * 100.0)).ToString()) );

  FIntVector CurrentTile = GetTileVectorID(CurrentActorPosition);
  GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, FColor::White,
    FString::Printf(TEXT("\nActor Current Tile: %d_%d"), CurrentTile.X, CurrentTile.Y ));

  LastMsgIndex = ClientLocMsgIndex;
  GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, FColor::White,
    FString::Printf(TEXT("\nOrigin: %s km"), *(FDVector(CurrentOriginInt) / (1000.0 * 100.0)).ToString()) );
  GEngine->AddOnScreenDebugMessage(LastMsgIndex++, MsgTime, FColor::White,
     FString::Printf(TEXT("Num active actors (%d)"), ActiveActors.Num()) );
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
#endif
}

void ALargeMapManager::ConsiderSpectatorAsEgo(bool _SpectatorAsEgo)
{
  SpectatorAsEgo = _SpectatorAsEgo;
  if(SpectatorAsEgo && ActorsToConsider.Num() == 0 && Spectator)
  {
    // Activating the spectator in an empty world
    ActorsToConsider.Add(Spectator);
  }
  if (!SpectatorAsEgo && ActorsToConsider.Num() == 1 && ActorsToConsider.Contains(Spectator))
  {
    // Deactivating the spectator in a world with no other egos
    ActorsToConsider.Reset();
  }
}
