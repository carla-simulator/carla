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
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  TArray<FAssetData> PendingAssetsInTile;
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
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void GenerateMap(FString AssetsPath);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void AddNewClientPosition(AActor* InActor)
  {
    // TODO: This should be able to adapt to a multiactor environment
    // The problem is that the rebase will only be available for one
    // unless we create more instances of the simulation
    ActorToConsider = InActor;
  }

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  FIntVector GetNumTilesInXY() const;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  bool IsLevelOfTileLoaded(FIntVector InTileID) const;

protected:

  FIntVector GetTileVectorID(FVector TileLocation) const;

  FIntVector GetTileVectorID(FDVector TileLocation) const;

  FIntVector GetTileVectorID(uint64 TileID) const;

  /// From a given location it retrieves the TileID that covers that area
  uint64 GetTileID(FVector TileLocation) const;

  uint64 GetTileID(FIntVector TileVectorID) const;

  FCarlaMapTile& GetCarlaMapTile(FVector Location);

  FCarlaMapTile& GetCarlaMapTile(ULevel* InLevel);

  const FCarlaMapTile* GetCarlaMapTile(FIntVector TileVectorID) const;

  ULevelStreamingDynamic* AddNewTile(FString TileName, FVector TileLocation);

  void UpdateActorsToConsiderPosition();

  void UpdateTilesState();

  void SpawnAssetsInTile(FCarlaMapTile& Tile);

  TMap<uint64, FCarlaMapTile> MapTiles;

  AActor* ActorToConsider = nullptr;
  // Current Origin after rebase
  FIntVector CurrentOriginInt{0};
  FDVector CurrentOriginD;
  // Current actor position with rebase in double precision
  FDVector CurrentActorPosition;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float LayerStreamingDistance = 2.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float RebaseOriginDistance = 2.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float TileSide = 2.0f * 1000.0f * 100.0f; // 2km

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool ShouldTilesBlockOnLoad = false;

#if WITH_EDITOR

  FString GenerateTileName(uint64 TileID);

  void PrintMapInfo();

  FColor PositonMsgColor = FColor::Purple;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintMapInfo = true;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintErrors = false;

#endif // WITH_EDITOR

};
