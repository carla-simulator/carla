// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
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
}

// Called when the game starts or when spawned
void ALargeMapManager::BeginPlay()
{
  Super::BeginPlay();

  // Setup delegates
  FCoreDelegates::PreWorldOriginOffset.AddUObject(this, &ALargeMapManager::PreWorldOriginOffset);
  FCoreDelegates::PostWorldOriginOffset.AddUObject(this, &ALargeMapManager::PostWorldOriginOffset);

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
  CurrentOrigin = InDstOrigin;

  GEngine->AddOnScreenDebugMessage(66, 30.0f, FColor::Yellow,
    FString::Printf(TEXT("Src: %s  ->  Dst: %s"), *InSrcOrigin.ToString(), *InDstOrigin.ToString())
    );


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

// Called every frame
void ALargeMapManager::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

#if WITH_EDITOR
  if(bPrintMapInfo) PrintMapInfo();
#endif // WITH_EDITOR

}

void ALargeMapManager::GenerateMap(FString AssetsPath)
{
  /* Retrive all the assets in the path */
  TArray<FAssetData> AssetsData;
  UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), true, GIsEditor);
  ObjectLibrary->LoadAssetDataFromPath(AssetsPath);
  ObjectLibrary->GetAssetDataList(AssetsData);

  /* Generate tiles based on mesh positions */
  UWorld* World = GetWorld();
  for(const FAssetData& AssetData : AssetsData)
  {
    UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.GetAsset());
    FBox BoundingBox = Mesh->GetBoundingBox();
    FVector CenterBB = BoundingBox.GetCenter();
    //GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::White,
    //  FString::Printf(TEXT("  %s - %s "), *CenterBB.ToString(), *BoundingBox.ToString() ));

    // Get map tile: create one if it does not exists
    FCarlaMapTile& MapTile = GetCarlaMapTile(CenterBB);

    // Recalculate asset location based on tile position
    FTransform Transform(
        FRotator(),
        CenterBB - MapTile.Location,
        FVector(1.0f));

    // Create intermediate actor (AUncenteredPivotPointMesh) to paliate not centered pivot point of the mesh
    FActorSpawnParameters SpawnParams;
    SpawnParams.OverrideLevel = MapTile.StreamingLevel->GetLoadedLevel();
    AUncenteredPivotPointMesh* SMActor =
        World->SpawnActor<AUncenteredPivotPointMesh>(
          AUncenteredPivotPointMesh::StaticClass(),
          Transform,
          SpawnParams);

    UStaticMeshComponent* SMComp = SMActor->GetMeshComp();
    SMComp->SetStaticMesh(Mesh);
    SMComp->SetRelativeLocation(CenterBB * -1.0f); // The pivot point does not get affected by Tile location

  }

}

FIntVector ALargeMapManager::GetTileVectorID(FVector TileLocation)
{
  return FIntVector(TileLocation / kTileSide);
}

FIntVector ALargeMapManager::GetTileVectorID(uint32 TileID)
{
  return FIntVector{
    (int32)((TileID >> 16) & 0xFFFF),
    (int32)(TileID & 0xFFFF),
    0
  };
}

uint32 ALargeMapManager::GetTileID(FVector TileLocation)
{
  FIntVector TileID = GetTileVectorID(TileLocation);
  return ( ((TileID.X & 0xFFFF) << 16) | (TileID.Y & 0xFFFF) );
}

FString ALargeMapManager::GenerateTileName(uint32 TileID)
{
  int32 X = (int32)((TileID >> 16) & 0xFFFF);
  int32 Y = (int32)(TileID & 0xFFFF);
  return FString::Printf(TEXT("Tile_%d_%d"), X, Y);
}

FCarlaMapTile& ALargeMapManager::GetCarlaMapTile(FVector Location)
{
  // Asset Location -> TileID
  uint32 TileID = GetTileID(Location);

  UE_LOG(LogCarla, Warning, TEXT("GetCarlaMapTile %s -> %x"), *Location.ToString(), TileID);

  FCarlaMapTile* Tile = MapTiles.Find(TileID);
  if(Tile) return *Tile; // Tile founded

  // Need to generate a new Tile
  FCarlaMapTile NewTile;
  // 1 - Calculate the Tile position
  FIntVector VTileID = GetTileVectorID(TileID);
  NewTile.Location = FVector(VTileID) * kTileSide;
  UE_LOG(LogCarla, Warning, TEXT("                NewTile.Location %s"), *NewTile.Location.ToString());
  // 2 - Generate Tile name
  NewTile.Name = GenerateTileName(TileID);
  // 3 - Generate the StreamLevel
  NewTile.StreamingLevel = AddNewTile(NewTile.Name, NewTile.Location);
  // 4 - Add it to the map
  return MapTiles.Add(TileID, NewTile);
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

#if WITH_EDITOR

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
  Output += FString::Printf(TEXT("Num tiles in world composition: %d\n"), World->WorldComposition->GetTilesList().Num());
  Output += FString::Printf(TEXT("Num world level collections: %d\n"), WorldLevelCollections.Num());
  // Output += FString::Printf(TEXT("Num levels in active collection: %d\n"), LevelCollection->GetLevels().Num());
  Output += FString::Printf(TEXT("Num levels: %d (%d)\n"), Levels.Num(), World->GetNumLevels());
  Output += FString::Printf(TEXT("Num streaming levels: %d\n"), StreamingLevels.Num());
  Output += FString::Printf(TEXT("Current Level: %s\n"), *CurrentLevel->GetName());//, *CurrentLevel->GetFullName());

  /*
  Output += "Streaming Levels List:\n";
  for (ULevelStreaming* Level : StreamingLevels)
  {
    Output += Level->GetName() + " - " + Level->GetFullName() + " - " + Level->GetWorldAssetPackageFName().ToString() + "\n";
  }
  */
  GEngine->AddOnScreenDebugMessage(0, 30.0f, FColor::Cyan, Output);

  Output = "Current tiles - Distance:\n";

  ULocalPlayer* Player = GEngine->GetGamePlayer(World, 0);
  FVector ViewLocation;
  FRotator ViewRotation;
  Player->PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
  int LastMsgIndex = 0;
  for (int i = 0; i < StreamingLevels.Num(); i++)
  {
    ULevelStreaming* Level = StreamingLevels[i];
    FVector LevelLocation = Level->LevelTransform.GetLocation();
    float Distance = FVector::Dist(LevelLocation, ViewLocation);

    FColor MsgColor = (Levels.Contains(Level->GetLoadedLevel())) ? FColor::Green : FColor::Red;

    GEngine->AddOnScreenDebugMessage(++LastMsgIndex, 30.0f, MsgColor,
      FString::Printf(TEXT("%s       %.2f"), *Level->GetName(), Distance));
  }

  FIntVector IntViewLocation(ViewLocation * 100.0f);
  GEngine->AddOnScreenDebugMessage(++LastMsgIndex, 30.0f, PositonMsgColor, IntViewLocation.ToString());

  CurrentPlayerPosition = (CurrentOrigin + FIntVector(ViewLocation));

  FString StrCurrentPlayerPosition = CurrentPlayerPosition.ToString();

  GEngine->AddOnScreenDebugMessage(++LastMsgIndex, 30.0f, PositonMsgColor,
    FString::Printf(TEXT("Origin: %s \nClient Loc: %s"), *CurrentOrigin.ToString(), *StrCurrentPlayerPosition));
}

#endif // WITH_EDITOR

