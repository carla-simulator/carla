// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/LevelStreamingDynamic.h"

#include "Math/DVector.h"

#include "LargeMapTileManager.generated.h"

USTRUCT()
struct FCarlaMapTile
{
  GENERATED_BODY()

#if WITH_EDITOR
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FString Name; // Tile_{TileID_X}_{TileID_Y}
#endif // WITH_EDITOR

  /// Absolute location, does not depend on rebasing
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FVector Location{0.0f};

  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  ULevelStreamingDynamic* StreamingLevel = nullptr;

  /// Assets in tile waiting to be spawned
  // TODO: categorize assets type and prioritize roads
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  TArray<FAssetData> PendingAssetsInTile;

  bool TilesSpawned = false;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API ULargeMapTileManager : public UActorComponent
{
  GENERATED_BODY()

public:

  ULargeMapTileManager();

protected:

  void BeginPlay() override;

public:

  void ChangeWorldOrigin(FIntVector InSrcOrigin, FIntVector InDstOrigin);

  void OnLevelAddedToWorld(ULevel* InLevel);

  void OnLevelRemovedFromWorld(ULevel* InLevel);

  void GenerateMap(const FString& InAssetsPath);

  /// Get num tiles in each axis
  FIntVector GetNumTilesInXY() const;

  bool IsLevelOfTileLoaded(FIntVector InTileID) const;

  /// Get surrounded Tiles for each location.
  /// Each location is translated to a tile and that tile and all its neighbours
  /// are added to the Set.
  void GetTilesToConsider(
    const FDVector& LocationToConsider,
    TSet<uint64>& OutTilesToConsider);

  /// Detects which tiles has to be loaded, unloaded or does not need to change
  void UpdateTilesState(TArray<FDVector>& ActorsLocation);

private:

  /// From a given location it retrieves the TileID that covers that area separated
  /// in X and Y coordinates
  FIntVector GetTileVectorID(FVector TileLocation) const;

  /// From a given location it retrieves the TileID that covers that area separated
  /// in X and Y coordinates
  FIntVector GetTileVectorID(FDVector TileLocation) const;

  /// From a given TileID it retrieves the ID separated in X and Y coordinates
  FIntVector GetTileVectorID(uint64 TileID) const;

  uint64 GetTileID(FVector TileLocation) const;

  uint64 GetTileID(FIntVector TileVectorID) const;

  FCarlaMapTile& GetCarlaMapTile(FVector Location);

  FCarlaMapTile& GetCarlaMapTile(ULevel* InLevel);

  FCarlaMapTile* GetCarlaMapTile(FIntVector TileVectorID);

  ULevelStreamingDynamic* AddNewTile(FString TileName, FVector TileLocation);

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

  TSet<uint64> CurrentTilesLoaded;

  const FString BaseTileMapPath = "/Game/Carla/Maps/LargeMap/EmptyTileBase";

  // Current Origin after rebase
  FIntVector CurrentOriginInt{ 0 };
  FDVector CurrentOriginD;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float LayerStreamingDistance = 3.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float TileSide = 2.0f * 1000.0f * 100.0f; // 2km

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool ShouldTilesBlockOnLoad = false;

#if WITH_EDITOR

public:
  void PrintMapInfo(float MsgTime, int32 FirstMsgIndex, int32 MsgLimit);

private:
  FString GenerateTileName(uint64 TileID);

  void DumpTilesTable() const;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  FString AssetsPath = "";

  FColor PositonMsgColor = FColor::Purple;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintErrors = false;

#endif // WITH_EDITOR

};
