// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include "LargeMapManager.h"

#include "Engine/LocalPlayer.h"
#include "Engine/WorldComposition.h"

#include "UncenteredPivotPointMesh.h"


// Sets default values
ALargeMapManager::ALargeMapManager()
{
  PrimaryActorTick.bCanEverTick = true;

}

ALargeMapManager::~ALargeMapManager()
{
  // Remove delegates
  FCoreDelegates::PreWorldOriginOffset.RemoveAll(this);
  FCoreDelegates::PostWorldOriginOffset.RemoveAll(this);

  FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);
  FWorldDelegates::LevelAddedToWorld.RemoveAll(this);
}

// Called when the game starts or when spawned
void ALargeMapManager::BeginPlay()
{
  Super::BeginPlay();

  // Setup delegates
  FCoreDelegates::PreWorldOriginOffset.AddUObject(this, &ALargeMapManager::PreWorldOriginOffset);
  FCoreDelegates::PostWorldOriginOffset.AddUObject(this, &ALargeMapManager::PostWorldOriginOffset);

  FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ALargeMapManager::OnLevelAddedToWorld);
  FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &ALargeMapManager::OnLevelRemovedFromWorld);

  UWorldComposition* WorldComposition = GetWorld()->WorldComposition;
  // Setup Origin rebase settings
  WorldComposition->bRebaseOriginIn3DSpace = true;
  WorldComposition->RebaseOriginDistance = RebaseOriginDistance;
}

void ALargeMapManager::PreWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin)
{
}

void ALargeMapManager::PostWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin)
{
  CurrentOriginInt = InDstOrigin;
  CurrentOriginD = FDVector(InDstOrigin);

  GEngine->AddOnScreenDebugMessage(66, 30.0f, FColor::Yellow,
    FString::Printf(TEXT("Src: %s  ->  Dst: %s"), *InSrcOrigin.ToString(), *InDstOrigin.ToString()) );
  UE_LOG(LogCarla, Warning, TEXT("PostWorldOriginOffset Src: %s  ->  Dst: %s"), *InSrcOrigin.ToString(), *InDstOrigin.ToString());

  // This is just to update the color of the msg with the same as the closest map
  UWorld* World = GetWorld();
  const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();
  FColor LevelColor = FColor::White;
  float MinDistance = 1000000.0f;
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
}

void ALargeMapManager::OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld)
{
  UE_LOG(LogCarla, Warning, TEXT("OnLevelAddedToWorld"));
  FCarlaMapTile& Tile = GetCarlaMapTile(InLevel);
  SpawnAssetsInTile(Tile);
}

void ALargeMapManager::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
  UE_LOG(LogCarla, Warning, TEXT("OnLevelRemovedFromWorld"));
}

// Called every frame
void ALargeMapManager::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  UpdateActorsToConsiderPosition();

#if WITH_EDITOR
  if(bPrintMapInfo) PrintMapInfo();
#endif // WITH_EDITOR

}

void ALargeMapManager::GenerateMap(FString AssetsPath)
{
  /* Retrive all the assets in the path */
  TArray<FAssetData> AssetsData;
  UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), true, true);
  ObjectLibrary->LoadAssetDataFromPath(AssetsPath);
  ObjectLibrary->GetAssetDataList(AssetsData);

  /* Generate tiles based on mesh positions */
  UWorld* World = GetWorld();
  for(const FAssetData& AssetData : AssetsData)
  {
    UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.FastGetAsset());
    FBox BoundingBox = Mesh->GetBoundingBox();
    FVector CenterBB = BoundingBox.GetCenter();

    // Get map tile: create one if it does not exists
    FCarlaMapTile& MapTile = GetCarlaMapTile(CenterBB);

    // Update tile assets list
    MapTile.PendingAssetsInTile.Add(AssetData);
  }

}

FIntVector ALargeMapManager::GetTileVectorID(FVector TileLocation)
{
  UE_LOG(LogCarla, Warning, TEXT("      GetTileVectorID %s --> %s"), *TileLocation.ToString(), *FIntVector(TileLocation / TileSide).ToString());
  return FIntVector(TileLocation / TileSide);
}

FIntVector ALargeMapManager::GetTileVectorID(uint64 TileID)
{
  UE_LOG(LogCarla, Warning, TEXT("                GetTileVectorID %ld --> %d %d"), TileID, (TileID >> 32), (TileID & (int32)(~0)) );
  return FIntVector{
    (int32)(TileID >> 32),
    (int32)(TileID & (int32)(~0)),
    0
  };
}

uint64 ALargeMapManager::GetTileID(FVector TileLocation)
{
  FIntVector TileID = GetTileVectorID(TileLocation);
  int64 X = ((int64)(TileID.X) << 32);
  int64 Y = (int64)(TileID.Y) & 0x00000000FFFFFFFF;
  UE_LOG(LogCarla, Warning, TEXT("                GetTileID %s --> %x %x --> %lx"), *TileID.ToString(), (int64)(TileID.X), TileID.Y, ( X | Y ) );
  return ( X | Y );
}

FCarlaMapTile& ALargeMapManager::GetCarlaMapTile(FVector Location)
{
  UE_LOG(LogCarla, Error, TEXT("GetCarlaMapTile........"));
  // Asset Location -> TileID
  uint64 TileID = GetTileID(Location);

  UE_LOG(LogCarla, Warning, TEXT("GetCarlaMapTile %s -> %lx"), *Location.ToString(), TileID);

  FCarlaMapTile* Tile = MapTiles.Find(TileID);
  if(Tile) return *Tile; // Tile founded

  // Need to generate a new Tile
  FCarlaMapTile NewTile;
  // 1 - Calculate the Tile position
  FIntVector VTileID = GetTileVectorID(TileID);
  NewTile.Location = FVector(VTileID) * TileSide * 1.5f;
  UE_LOG(LogCarla, Warning, TEXT("                NewTile.Location %s"), *NewTile.Location.ToString());
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
  for(auto& It : MapTiles)
  {
    ULevelStreamingDynamic* StreamingLevel = It.Value.StreamingLevel;
    ULevel* Level = StreamingLevel->GetLoadedLevel();
    if(Level == InLevel)
    {
      Tile = &(It.Value);
      break;
    }
  }
  check(Tile);
  return *Tile;
}

ULevelStreamingDynamic* ALargeMapManager::AddNewTile(FString TileName, FVector TileLocation)
{
  UWorld* World = GetWorld();
  UWorldComposition* WorldComposition = World->WorldComposition;

  FString FullName = "/Game/Carla/Maps/LargeMap/EmptyTileBase";
  FString PackageFileName = FullName;
  FString LongLevelPackageName = FPackageName::FilenameToLongPackageName(PackageFileName);
  FString UniqueLevelPackageName = LongLevelPackageName + TileName;

  ULevelStreamingDynamic* StreamingLevel = NewObject<ULevelStreamingDynamic>(World, *TileName, RF_Transient);
  check(StreamingLevel);

  UE_LOG(LogCarla, Error, TEXT("AddNewTile created new streaming level -> %s"), *TileName);

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

  StreamingLevel->SetShouldBeLoaded(true);
  StreamingLevel->SetShouldBeVisible(true);
  StreamingLevel->bShouldBlockOnLoad = ShouldTilesBlockOnLoad;
  StreamingLevel->bInitiallyLoaded = true;
  StreamingLevel->bInitiallyVisible = true;
  StreamingLevel->LevelTransform = FTransform(TileLocation);
  StreamingLevel->PackageNameToLoad = *FullName;

  if (!FPackageName::DoesPackageExist(FullName, NULL, &PackageFileName))
  {
    UE_LOG(LogCarla, Error, TEXT("Level does not exist in package with FullName variable -> %s"), *FullName);
  }

  if (!FPackageName::DoesPackageExist(LongLevelPackageName, NULL, &PackageFileName))
  {
    UE_LOG(LogCarla, Error, TEXT("Level does not exist in package with LongLevelPackageName variable -> %s"), *LongLevelPackageName);
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
  WorldTileLayer.DistanceStreamingEnabled = true;
  Info.Layer = WorldTileLayer;

  FWorldCompositionTile NewTile;
  NewTile.PackageName = *FullName;
  NewTile.Info = Info;
  WorldComposition->GetTilesList().Add(NewTile);

  return StreamingLevel;
}

void ALargeMapManager::UpdateActorsToConsiderPosition()
{
  // Relative location to the current origin
  FDVector ActorLocation(ActorToConsider->GetActorLocation());
  // Absolute location of the actor
  CurrentActorPosition = CurrentOriginD + ActorLocation;
}

void ALargeMapManager::SpawnAssetsInTile(FCarlaMapTile& Tile)
{
  FString Output = "";
  Output += FString::Printf(TEXT("SpawnAssetsInTile %s %d\n"), *Tile.Name, Tile.PendingAssetsInTile.Num());
  Output += FString::Printf(TEXT("  Tile.Loc = %s\n"), *Tile.Location.ToString());


  FVector TileLocation = Tile.Location;
  ULevel* LoadedLevel = Tile.StreamingLevel->GetLoadedLevel();
  UWorld* World = GetWorld();

  for(const FAssetData& AssetData : Tile.PendingAssetsInTile)
  {
    UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.GetAsset());
    FBox BoundingBox = Mesh->GetBoundingBox();
    FVector CenterBB = BoundingBox.GetCenter();

    // Recalculate asset location based on tile position
    FTransform Transform(FRotator(0.0f), CenterBB - TileLocation, FVector(1.0f));

    // Create intermediate actor (AUncenteredPivotPointMesh) to paliate not centered pivot point of the mesh
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = *FString::Printf(TEXT("UPPM_%s_%s"), *Mesh->GetName(),*Tile.Name);
    SpawnParams.OverrideLevel = LoadedLevel;
    AUncenteredPivotPointMesh* SMActor =
        World->SpawnActor<AUncenteredPivotPointMesh>(
          AUncenteredPivotPointMesh::StaticClass(),
          Transform,
          SpawnParams);

    UStaticMeshComponent* SMComp = SMActor->GetMeshComp();
    SMComp->SetStaticMesh(Mesh);
    SMComp->SetRelativeLocation(CenterBB * -1.0f); // The pivot point does not get affected by Tile location

    Output += FString::Printf(TEXT("    MeshName = %s -> %s\n"), *Mesh->GetName(), *SMActor->GetName());
    Output += FString::Printf(TEXT("      CenterBB = %s\n"), *CenterBB.ToString());
    Output += FString::Printf(TEXT("      ActorLoc = %s\n"), *(CenterBB - TileLocation).ToString());
    Output += FString::Printf(TEXT("      SMRelLoc = %s\n"), *SMComp->GetRelativeLocation().ToString());
  }
  UE_LOG(LogCarla, Warning, TEXT("%s"), *Output);
  // Tile.PendingAssetsInTile.Reset();

  LoadedLevel->ApplyWorldOffset(TileLocation, false);
}

#if WITH_EDITOR

FString ALargeMapManager::GenerateTileName(uint64 TileID)
{
  int32 X = (int32)(TileID >> 32);
  int32 Y = (int32)(TileID);

  UE_LOG(LogCarla, Warning, TEXT("                GenerateTileName %d %d"), X, Y);

  return FString::Printf(TEXT("Tile_%d_%d"), X, Y);
}

void ALargeMapManager::PrintMapInfo()
{
  UWorld* World = GetWorld();

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
  GEngine->AddOnScreenDebugMessage(0, 30.0f, FColor::Cyan, Output);

  int LastMsgIndex = 0;
  GEngine->AddOnScreenDebugMessage(++LastMsgIndex, 30.0f, FColor::White, TEXT("Current tiles - Distance:"));

  ULocalPlayer* Player = GEngine->GetGamePlayer(World, 0);
  FVector ViewLocation;
  FRotator ViewRotation;
  Player->PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
  for (int i = 0; i < StreamingLevels.Num(); i++)
  {
    ULevelStreaming* Level = StreamingLevels[i];
    FVector LevelLocation = Level->LevelTransform.GetLocation();
    float Distance = FVector::Dist(LevelLocation, ViewLocation) * 100.0f * 100.0f;

    FColor MsgColor = (Levels.Contains(Level->GetLoadedLevel())) ? FColor::Green : FColor::Red;

    GEngine->AddOnScreenDebugMessage(++LastMsgIndex, 30.0f, MsgColor,
      FString::Printf(TEXT("%s       %.2f"), *Level->GetName(), Distance));
  }


  FIntVector IntViewLocation(ViewLocation * 100.0f);
  GEngine->AddOnScreenDebugMessage(++LastMsgIndex, 30.0f, PositonMsgColor, IntViewLocation.ToString());

  CurrentActorPosition = (CurrentOriginInt + FIntVector(ViewLocation));
  FString StrCurrentActorPosition = CurrentActorPosition.ToString();
  GEngine->AddOnScreenDebugMessage(++LastMsgIndex, 30.0f, PositonMsgColor,
    FString::Printf(TEXT("Origin: %s \nClient Loc: %s"), *CurrentOriginInt.ToString(), *StrCurrentActorPosition));
}

#endif // WITH_EDITOR

