// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "LargeMapManager.h"

#include "Engine/LocalPlayer.h"
#include "Engine/WorldComposition.h"


// Sets default values
ALargeMapManager::ALargeMapManager()
{
   // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
  FString Output = "";

  /* Retrive all the assets in the path */
  FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
  TArray<FAssetData> AssetsData;
  const UClass* Class = UStaticMesh::StaticClass();
  AssetRegistryModule.Get().GetAssetsByPath(*AssetsPath, AssetsData, true);

  Output += FString::Printf(TEXT("AssetsData (%d): \n"), AssetsData.Num());
  for(const FAssetData& AssetData : AssetsData)
  {
    Output += AssetData.ObjectPath.ToString() + "\n";
  }

  GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::White, Output);

}

ULevelStreamingDynamic* ALargeMapManager::AddNewTile(FString TileName, FVector TileLocation)
{
  ULevelStreamingDynamic* CreatedLevel = AddNewTileInternal(*TileName, TileLocation);
  TilesCreated.Add(TileName, CreatedLevel);
  return CreatedLevel;
}

ULevelStreamingDynamic* ALargeMapManager::AddNewTileInternal(FString TileName, FVector Location)
{
  UWorld* World = GetWorld();
  UWorldComposition* WorldComposition = World->WorldComposition;

  FString FullName = "/Game/Carla/Maps/Sublevels/Town03_Opt/T03_Layout";
  FName LevelFName = FName(*FullName);
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
  StreamingLevel->LevelTransform = FTransform(Location);
  StreamingLevel->PackageNameToLoad = *FullName;

  if (!FPackageName::DoesPackageExist(FullName, NULL, &PackageFileName))
  {
    GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("Level does not exist in package with FullName variable -> %s"), *FullName));
  }

  if (!FPackageName::DoesPackageExist(LongLevelPackageName, NULL, &PackageFileName))
  {
    GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, FString::Printf(TEXT("Level does not exist in package with LongLevelPackageName variable -> %s"), *LongLevelPackageName));
  }

  //Actual map package to load
  StreamingLevel->PackageNameToLoad = *LongLevelPackageName;


  World->AddStreamingLevel(StreamingLevel);
  WorldComposition->TilesStreaming.Add(StreamingLevel);


  FWorldTileInfo Info;
  Info.AbsolutePosition = FIntVector(Location);
  FVector MinBound(-20000.0f);
  FVector MaxBound(20000.0f);
  //Info.Bounds = FBox(MinBound, MaxBound);
  FWorldTileLayer WorldTileLayer;
  WorldTileLayer.Name = "CarlaLayer";
  WorldTileLayer.StreamingDistance = LayerStreamingDistance;
  WorldTileLayer.DistanceStreamingEnabled = true;
  Info.Layer = WorldTileLayer;

  FWorldCompositionTile NewTile;
  NewTile.PackageName = LevelFName;
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

