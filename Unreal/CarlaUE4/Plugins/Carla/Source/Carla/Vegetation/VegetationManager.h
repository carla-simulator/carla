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

#include "VegetationManager.generated.h"

USTRUCT()
struct FTileMeshComponent
{
  GENERATED_BODY()
  UInstancedStaticMeshComponent* InstancedStaticMeshComponent {nullptr};
  TArray<int32> IndicesInUse {};
};

USTRUCT()
struct FTileData
{
  GENERATED_BODY()
  AInstancedFoliageActor* InstancedFoliageActor {nullptr};
  AProceduralFoliageVolume* ProceduralFoliageVolume {nullptr};
  TArray<FTileMeshComponent> TileMeshesCache {};
  TArray<UMaterialInstanceDynamic*> MaterialInstanceDynamicCache {};

  bool ContainsMesh(const UInstancedStaticMeshComponent*) const;
  void UpdateTileMeshComponent(UInstancedStaticMeshComponent* NewInstancedStaticMeshComponent);
  void UpdateMaterialCache(const FLinearColor& Value, bool DebugMaterials);
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
  AActor* Actor { nullptr };
  FTransform GlobalTransform {FTransform()};

  void EnableActor();
  void DisableActor();
};

UCLASS()
class CARLA_API AVegetationManager : public AActor
{
  GENERATED_BODY()

  /// @}
  // ===========================================================================
  /// @name Overriden from AActor
  // ===========================================================================
  /// @{
protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  /// @}
  // ===========================================================================
  /// @name Events
  // ===========================================================================
  /// @{
public:
  void AddVehicle(ACarlaWheeledVehicle* Vehicle);
  void RemoveVehicle(ACarlaWheeledVehicle* Vehicle);

private:
  void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);
  void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);  

  /// @}
  // ===========================================================================
  /// @name Tick
  // ===========================================================================
  /// @{
private:
  TArray<FString> GetTilesInUse();
  void UpdateMaterials(TArray<FString>& Tiles);
  TArray<TPair<FFoliageBlueprint, TArray<FTransform>>> GetElementsToSpawn(TArray<FString>& Tiles);
  void SpawnSkeletalFoliages(const TArray<TPair<FFoliageBlueprint, TArray<FTransform>>>& ElementsToSpawn);
  void DestroySkeletalFoliages();

  /// @}
  // ===========================================================================
  /// @name Tiles
  // ===========================================================================
  /// @{
private:
  void CreateOrUpdateTileCache(ULevel* InLevel);
  void SetTileDataInternals(FTileData& TileData);
  void SetInstancedStaticMeshComponentCache(FTileData& TileData);
  void SetMaterialCache(FTileData& TileData);
  void UpdateFoliageBlueprintCache(ULevel* InLevel);
  void FreeTileCache(ULevel* InLevel);

  /// @}
  // ===========================================================================
  /// @name Pools
  // ===========================================================================
  /// @{
private:
  bool EnableActorFromPool(const FTransform& Transform, TArray<FPooledActor>& Pool);
  void CreatePoolForBPClass(const FFoliageBlueprint& BP);
  AActor* CreateFoliage(const FFoliageBlueprint& BP, const FTransform& Transform) const;
  bool IsFoliageTypeEnabled(const FString& Path) const;

public:
  UPROPERTY(Category = "CARLA Vegetation Spwaner", EditDefaultsOnly)
  bool DebugMaterials {false};
  //Filters for debug
  UPROPERTY(Category = "CARLA Vegetation Spwaner", EditDefaultsOnly)
  bool SpawnBushes {true};

  UPROPERTY(Category = "CARLA Vegetation Spwaner", EditDefaultsOnly)
  bool SpawnTrees {true};

  UPROPERTY(Category = "CARLA Vegetation Spwaner", EditDefaultsOnly)
  bool SpawnRocks {true};

  UPROPERTY(Category = "CARLA Vegetation Spwaner", EditDefaultsOnly)
  bool SpawnPlants {true};

  UPROPERTY(Category = "CARLA Vegetation Spwaner", EditDefaultsOnly)
  float SpawnScale {1.0f};

  UPROPERTY(Category = "CARLA Vegetation Spwaner", EditDefaultsOnly)
  int32 InitialPoolSize {5};

private:
  //Actors
  ALargeMapManager* LargeMap {nullptr};
  ACarlaWheeledVehicle* HeroVehicleInLevel {};
  //Caches
  TMap<FString, FFoliageBlueprint> FoliageBlueprintCache {};
  TMap<FString, FTileData> TileCache {};
  FString LastUsedTile {};
  //Pools
  TMap<FString, TArray<FPooledActor>> ActorPool {};
};
