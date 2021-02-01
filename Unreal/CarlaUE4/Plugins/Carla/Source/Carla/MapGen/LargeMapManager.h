// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
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

  FString Name; // Tile_{TileID_X}_{TileID_Y}

  ULevelStreamingDynamic* StreamingLevel = nullptr;

  FVector Location{0.0f};
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

protected:

  FIntVector GetTileVectorID(FVector TileLocation);

  FIntVector GetTileVectorID(uint32 TileID);

  /// From a given location it retrieves the TileID that covers that area
  uint32 GetTileID(FVector TileLocation);

  FString GenerateTileName(uint32 TileID);

  FCarlaMapTile& GetCarlaMapTile(FVector Location);

  ULevelStreamingDynamic* AddNewTile(FString TileName, FVector TileLocation);


  TMap<uint32, FCarlaMapTile> MapTiles;

  AActor* ActorToConsider = nullptr;

  FIntVector CurrentOrigin{0};

  FDVector CurrentPlayerPosition;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float LayerStreamingDistance = 2.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float RebaseOriginDistance = 2.0f * 1000.0f * 100.0f;

  //UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  const float kTileSide = 2.0f * 1000.0f * 100.0f; // 2km

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool ShouldTilesBlockOnLoad = false;

#if WITH_EDITOR

  void PrintMapInfo();

  FColor PositonMsgColor = FColor::Purple;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintMapInfo = true;

#endif // WITH_EDITOR

};
