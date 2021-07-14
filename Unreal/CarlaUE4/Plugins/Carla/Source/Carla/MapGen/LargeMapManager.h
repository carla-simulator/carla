// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Actor/CarlaActor.h"
#include "GameFramework/Actor.h"

#include "Engine/LevelStreamingDynamic.h"

#include "Math/DVector.h"

#include "LargeMapManager.generated.h"


// TODO: Cache CarlaEpisode

USTRUCT()
struct FActiveActor
{
  GENERATED_BODY()

  FActiveActor() {}

  FActiveActor(
    const FCarlaActor* InCarlaActor,
    const FTransform& InTransform)
    : CarlaActor(InCarlaActor),
      WorldLocation(FDVector(InTransform.GetTranslation())),
      Rotation(InTransform.GetRotation()) {}

  const FCarlaActor* CarlaActor;

  FDVector WorldLocation;

  FQuat Rotation;
};

USTRUCT()
struct FCarlaMapTile
{
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FString Name; // Tile_{TileID_X}_{TileID_Y}

  // Absolute location, does not depend on rebasing
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FVector Location{0.0f};
  // TODO: not FVector

  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  ULevelStreamingDynamic* StreamingLevel = nullptr;

  bool TilesSpawned = false;
};

UCLASS()
class CARLA_API ALargeMapManager : public AActor
{
  GENERATED_BODY()

public:

  using TileID = uint64;

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

  void RegisterInitialObjects();

  void OnActorSpawned(const FCarlaActor& CarlaActor);

  UFUNCTION(Category="Large Map Manager")
  void OnActorDestroyed(AActor* DestroyedActor);

  // Called every frame
  void Tick(float DeltaTime) override;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void GenerateMap(FString InAssetsPath);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void GenerateLargeMap();

  void GenerateMap(TArray<TPair<FString, FIntVector>> MapPathsIds);

  UFUNCTION(BlueprintCallable, CallInEditor, Category = "Large Map Manager")
  void ClearWorldAndTiles();

  void AddActorToUnloadedList(const FCarlaActor& CarlaActor, const FTransform& Transform);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  FIntVector GetNumTilesInXY() const;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  int GetNumTiles() const
  {
    return MapTiles.Num();
  }

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  bool IsLevelOfTileLoaded(FIntVector InTileID) const;

  bool IsTileLoaded(TileID TileId) const
  {
    return CurrentTilesLoaded.Contains(TileId);
  }

  bool IsTileLoaded(FVector Location) const
  {
    return IsTileLoaded(GetTileID(Location));
  }

  bool IsTileLoaded(FDVector Location) const
  {
    return IsTileLoaded(GetTileID(Location));
  }

  FTransform GlobalToLocalTransform(const FTransform& InTransform) const;
  FVector GlobalToLocalLocation(const FVector& InLocation) const;

  FTransform LocalToGlobalTransform(const FTransform& InTransform) const;
  FVector LocalToGlobalLocation(const FVector& InLocation) const;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  FString LargeMapTilePath = "/Game/Carla/Maps/testmap";
  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  FString LargeMapName = "testmap";

  void SetTile0Offset(const FVector& Offset);

  void SetTileSize(float Size);

  void SetLayerStreamingDistance(float Distance);

  void SetActorStreamingDistance(float Distance);

  float GetLayerStreamingDistance() const;

  float GetActorStreamingDistance() const;

protected:

  FIntVector GetTileVectorID(FVector TileLocation) const;

  FIntVector GetTileVectorID(FDVector TileLocation) const;

  FIntVector GetTileVectorID(TileID TileID) const;

  FVector GetTileLocation(TileID TileID) const;

  FVector GetTileLocation(FIntVector TileVectorID) const;

  FDVector GetTileLocationD(TileID TileID) const;

  FDVector GetTileLocationD(FIntVector TileVectorID) const;

  /// From a given location it retrieves the TileID that covers that area
  TileID GetTileID(FVector TileLocation) const;

  TileID GetTileID(FDVector TileLocation) const;

  TileID GetTileID(FIntVector TileVectorID) const;

  FCarlaMapTile& GetCarlaMapTile(FVector Location);

  FCarlaMapTile& GetCarlaMapTile(ULevel* InLevel);

  FCarlaMapTile* GetCarlaMapTile(FIntVector TileVectorID);

  FCarlaMapTile& LoadCarlaMapTile(FString TileMapPath, TileID TileId);

  void UpdateTilesState();

  void RemovePendingActorsToRemove();

  // Check if any active actor has to be converted into dormant actor
  // because it went out of range (ActorStreamingDistance)
  // Just stores the array of selected actors
  void CheckActiveActors();

  // Converts active actors that went out of range to dormant actors
  void ConvertActiveToDormantActors();

  // Check if any dormant actor has to be converted into active actor
  // because it enter in range (ActorStreamingDistance)
  // Just stores the array of selected actors
  void CheckDormantActors();

  // Converts active actors that went out of range to dormant actors
  void ConvertDormantToActiveActors();

  void CheckIfRebaseIsNeeded();

  void GetTilesToConsider(
    const AActor* ActorToConsider,
    TSet<TileID>& OutTilesToConsider);

  void GetTilesThatNeedToChangeState(
    const TSet<TileID>& InTilesToConsider,
    TSet<TileID>& OutTilesToBeVisible,
    TSet<TileID>& OutTilesToHidde);

  void UpdateTileState(
    const TSet<TileID>& InTilesToUpdate,
    bool InShouldBlockOnLoad,
    bool InShouldBeLoaded,
    bool InShouldBeVisible);

  void UpdateCurrentTilesLoaded(
    const TSet<TileID>& InTilesToBeVisible,
    const TSet<TileID>& InTilesToHidde);

  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  TMap<uint64, FCarlaMapTile> MapTiles;

  // All actors to be consider for tile loading (all hero vehicles)
  // The first actor in the array is the one selected for rebase
  // TODO: support rebase in more than one hero vehicle
  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  TArray<AActor*> ActorsToConsider;
  //UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  TArray<FCarlaActor::IdType> ActiveActors;
  TArray<FCarlaActor::IdType> DormantActors;

  // Temporal sets to remove actors. Just to avoid removing them in the update loop
  TSet<AActor*> ActorsToRemove;
  TSet<FCarlaActor::IdType> ActivesToRemove;
  TSet<FCarlaActor::IdType> DormantsToRemove;

  // Helpers to move Actors from one array to another.
  TSet<FCarlaActor::IdType> ActiveToDormantActors;
  TSet<FCarlaActor::IdType> DormantToActiveActors;

  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  TSet<uint64> CurrentTilesLoaded;

  // Current Origin after rebase
  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  FIntVector CurrentOriginInt{ 0 };

  FDVector CurrentOriginD;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  FVector Tile0Offset = FVector(0,0,0);

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float TickInterval = 0.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float LayerStreamingDistance = 3.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float ActorStreamingDistance = 2.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float RebaseOriginDistance = 2.0f * 1000.0f * 100.0f;

  float LayerStreamingDistanceSquared = LayerStreamingDistance * LayerStreamingDistance;
  float ActorStreamingDistanceSquared = ActorStreamingDistance * ActorStreamingDistance;
  float RebaseOriginDistanceSquared = RebaseOriginDistance * RebaseOriginDistance;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float TileSide = 2.0f * 1000.0f * 100.0f; // 2km

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool ShouldTilesBlockOnLoad = false;

  UFUNCTION(BlueprintCallable, CallInEditor, Category = "Large Map Manager")
    void GenerateMap_Editor()
  {
    if (!LargeMapTilePath.IsEmpty()) GenerateMap(LargeMapTilePath);
  }

  void RegisterTilesInWorldComposition();

  FString GenerateTileName(TileID TileID);

  FString TileIDToString(TileID TileID);

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

};
