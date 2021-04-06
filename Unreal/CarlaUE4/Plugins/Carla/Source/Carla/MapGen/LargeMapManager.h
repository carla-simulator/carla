// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Engine/LevelStreamingDynamic.h"

#include "Math/DVector.h"

#include "LargeMapManager.generated.h"


USTRUCT()
struct FCarlaMapTile
{
  GENERATED_BODY()

#if WITH_EDITOR
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FString Name; // Tile_{TileID_X}_{TileID_Y}
#endif // WITH_EDITOR

  // Absolute location, does not depend on rebasing
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FVector Location{0.0f};

  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  ULevelStreamingDynamic* StreamingLevel = nullptr;

  // Assets in tile waiting to be spawned
  // TODO: categorize assets type and prioritize roads
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  TArray<FAssetData> PendingAssetsInTile;

  bool TilesSpawned = false;
};

USTRUCT()
struct FActorToConsider
{
  GENERATED_BODY()

  FActorToConsider() {}

  FActorToConsider(AActor* InActor) : Actor(InActor) {}

  AActor* Actor = nullptr;

  // Absolute location = Current Origin + Relative Tile Location
  FDVector Location;

  bool operator==(const FActorToConsider& Other)
  {
    return Actor == Other.Actor &&
           Location == Other.Location;
  }
};

/*
  Actor that was spawned or queried to be spawn at some point but it was so far away
  from the origin that was removed from the level (or not spawned).
  It is possible that the actor keeps receiving updates, eg, traffic manager.
  FGhostActor is a wrapper of the info and state of the actor in case it needs to be re-spawned.
*/
struct FGhostActor
{
  FGhostActor(const FTransform& InTransform, const FActorView& InActorView) :
    Transform(InTransform),
    ActorView(InActorView) {}

  FTransform Transform;

  FActorView ActorView;
};

UCLASS()
class CARLA_API ALargeMapManager : public AActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ALargeMapManager();

  ~ALargeMapManager();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  void PreWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin);
  void PostWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin);

  void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);
  void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

public:
  void OnActorSpawned(const FActorView& ActorView);

  // Called every frame
  virtual void Tick(float DeltaTime) override;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void GenerateMap(FString InAssetsPath);

  void AddActorToUnloadedList(const FActorView& ActorView, const FTransform& Transform);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void AddActorToConsider(AActor* InActor);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void RemoveActorToConsider(AActor* InActor);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  FIntVector GetNumTilesInXY() const;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  bool IsLevelOfTileLoaded(FIntVector InTileID) const;

  // TODO: IsTileLoaded(FDVector Location)
  // TODO: IsTileLoaded(FVector Location)

protected:

  FIntVector GetTileVectorID(FVector TileLocation) const;

  FIntVector GetTileVectorID(FDVector TileLocation) const;

  FIntVector GetTileVectorID(uint64 TileID) const;

  /// From a given location it retrieves the TileID that covers that area
  uint64 GetTileID(FVector TileLocation) const;

  uint64 GetTileID(FIntVector TileVectorID) const;

  FCarlaMapTile& GetCarlaMapTile(FVector Location);

  FCarlaMapTile& GetCarlaMapTile(ULevel* InLevel);

  FCarlaMapTile* GetCarlaMapTile(FIntVector TileVectorID);

  ULevelStreamingDynamic* AddNewTile(FString TileName, FVector TileLocation);

  void UpdateActorsToConsiderPosition();

  void UpdateTilesState();

  void GetTilesToConsider(
    const FActorToConsider& ActorToConsider,
    TSet<uint64>& OutTilesToConsider);

  void GetTilesThatNeedToChangeState(
    const TSet<uint64>& InTilesToConsider,
    TSet<uint64>& OutTilesToBeVisible,
    TSet<uint64>& OutTilesToHidde);

  void UpdateTileState(
    const TSet<uint64>& InTilesToUpdate,
    bool InShouldBlockOnLoad,
    bool InShouldBeLoaded,
    bool InShouldBeVisible);

  void UpdateCurrentTilesLoaded(
    const TSet<uint64>& InTilesToBeVisible,
    const TSet<uint64>& InTilesToHidde);

  void SpawnAssetsInTile(FCarlaMapTile& Tile);


  TMap<uint64, FCarlaMapTile> MapTiles;

  TMap<uint32, FGhostActor> GhostActors;

  TArray<FActorToConsider> ActorsToConsider;

  TSet<uint64> CurrentTilesLoaded;

  // Current Origin after rebase
  FIntVector CurrentOriginInt{ 0 };
  FDVector CurrentOriginD;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float TickInterval = 0.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float LayerStreamingDistance = 3.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float RebaseOriginDistance = 2.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float TileSide = 2.0f * 1000.0f * 100.0f; // 2km

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool ShouldTilesBlockOnLoad = false;

#if WITH_EDITOR

  UFUNCTION(BlueprintCallable, CallInEditor, Category = "Large Map Manager")
    void GenerateMap_Editor()
  {
    if (!AssetsPath.IsEmpty()) GenerateMap(AssetsPath);
  }

  FString GenerateTileName(uint64 TileID);

  void DumpTilesTable() const;

  void PrintMapInfo();

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  FString AssetsPath = "";

  FString BaseTileMapPath = "/Game/Carla/Maps/LargeMap/EmptyTileBase";

  FColor PositonMsgColor = FColor::Purple;

  const int32 TilesDistMsgIndex = 100;
  const int32 MaxTilesDistMsgIndex = TilesDistMsgIndex + 10;

  const int32 ClientLocMsgIndex = 200;
  const int32 MaxClientLocMsgIndex = ClientLocMsgIndex + 10;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float MsgTime = 1.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintMapInfo = true;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintErrors = false;

#endif // WITH_EDITOR

};