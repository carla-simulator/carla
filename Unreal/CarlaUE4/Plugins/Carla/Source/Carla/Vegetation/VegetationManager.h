// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "ProceduralFoliageVolume.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Containers/Array.h"
#include "Containers/Map.h"

#include "Carla/MapGen/LargeMapManager.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include <memory>

#include "VegetationManager.generated.h"

USTRUCT()
struct FTileMeshComponent
{
  GENERATED_BODY()
  UInstancedStaticMeshComponent* InstancedStaticMeshComponent {nullptr};
  TArray<int32> IndicesInUse {};
  bool bIsAlive = false;
};

USTRUCT()
struct FTileData
{
  GENERATED_BODY()
  AInstancedFoliageActor* InstancedFoliageActor {nullptr};
  AProceduralFoliageVolume* ProceduralFoliageVolume {nullptr};
  TArray<std::shared_ptr<FTileMeshComponent>> TileMeshesCache {};
  TArray<UMaterialInstanceDynamic*> MaterialInstanceDynamicCache {};

  bool ContainsMesh(const UInstancedStaticMeshComponent*) const;
  void UpdateTileMeshComponent(UInstancedStaticMeshComponent* NewInstancedStaticMeshComponent);
  void UpdateMaterialCache(const FLinearColor& Value, bool DebugMaterials);
  ~FTileData();
};

USTRUCT()
struct FFoliageBlueprint
{
  GENERATED_BODY()
  FString BPFullClassName {};
  TSubclassOf<AActor> SpawnedClass { nullptr };

  bool IsValid() const;
  bool SetBPClassName(const FString& Path);
  bool SetSpawnedClass();
};

USTRUCT()
struct FPooledActor
{
  GENERATED_BODY()
  bool InUse { false };
  bool IsActive { false };
  AActor* Actor { nullptr };
  FTransform GlobalTransform {FTransform()};
  int32 Index {-1};
  std::shared_ptr<FTileMeshComponent> TileMeshComponent {nullptr};

  void EnableActor(
      const FTransform& Transform,
      int32 NewIndex,
      std::shared_ptr<FTileMeshComponent>& NewTileMeshComponent);
  void ActiveActor();
  void DisableActor();
};

USTRUCT()
struct FElementsToSpawn
{
  GENERATED_BODY()
  std::shared_ptr<FTileMeshComponent> TileMeshComponent;
  FFoliageBlueprint BP;
  TArray<TPair<FTransform, int32>> TransformIndex;
};

UCLASS()
class CARLA_API AVegetationManager : public AActor
{
  GENERATED_BODY()

public:
  void AddVehicle(ACarlaWheeledVehicle* Vehicle);
  void RemoveVehicle(ACarlaWheeledVehicle* Vehicle);

  UFUNCTION(BlueprintCallable)
  void UpdatePoolBasePosition();

public:
  UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
  bool DebugMaterials {false};

  UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
  float HideMaterialDistance {500.0f};

  UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
  float ActiveActorDistance {500.0f};

  //Filters for debug
  UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
  bool SpawnBushes {true};

  UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
  bool SpawnTrees {true};

  UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
  bool SpawnRocks {true};

  UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
  bool SpawnPlants {true};

  UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
  float SpawnScale {1.0f};

  UPROPERTY(Category = "CARLA Vegetation Spawner", EditDefaultsOnly)
  int32 InitialPoolSize {10};

  /// @}
  // ===========================================================================
  /// @name Overriden from AActor
  // ===========================================================================
  /// @{
protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

private:
  bool IsFoliageTypeEnabled(const FString& Path) const;
  bool CheckForNewTiles() const;

  TArray<FString> GetTilesInUse();

  void UpdateMaterials(FTileData* Tile);
  TArray<FElementsToSpawn> GetElementsToSpawn(FTileData* Tile);
  void SpawnSkeletalFoliages(TArray<FElementsToSpawn>& ElementsToSpawn);
  void DestroySkeletalFoliages();
  void ActivePooledActors();
  bool EnableActorFromPool(
      const FTransform& Transform,
      int32 Index,
      std::shared_ptr<FTileMeshComponent>& TileMeshComponent,
      TArray<FPooledActor>& Pool);

  void CreateOrUpdateTileCache(ULevel* InLevel);
  void UpdateFoliageBlueprintCache(ULevel* InLevel);
  void SetTileDataInternals(FTileData& TileData);
  void SetInstancedStaticMeshComponentCache(FTileData& TileData);
  void SetMaterialCache(FTileData& TileData);

  void FreeTileCache(ULevel* InLevel);

  void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);
  void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);
  void PostWorldOriginOffset(UWorld*, FIntVector, FIntVector InDstOrigin);

  void CreatePoolForBPClass(const FFoliageBlueprint& BP);
  AActor* CreateFoliage(const FFoliageBlueprint& BP, const FTransform& Transform) const;

  void GetSketalTemplates();

private:
  float PoolTranslationTimer {30.0f};
  FTransform InactivePoolTransform { FQuat(1.0f, 1.0f, 1.0f, 1.0f), FVector(1.0f, 1.0f, 1.0f), FVector(1.0f, 1.0f, 1.0f)};
  //Actors
  ALargeMapManager* LargeMap {nullptr};
  ACarlaWheeledVehicle* HeroVehicle {nullptr};
  //Caches
  TMap<FString, FFoliageBlueprint> FoliageBlueprintCache {};
  TMap<FString, FTileData> TileCache {};
  //Pools
  TMap<FString, TArray<FPooledActor>> ActorPool {};

  FTimerHandle UpdatePoolInactiveTransformTimer;
};
