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
  FString StaticMeshPath {};
  UInstancedStaticMeshComponent* InstancedStaticMeshComponent {nullptr};
  TArray<int32> IndicesInUse {};
};

USTRUCT()
struct FTileData
{
  GENERATED_BODY()
  FTransform TileGlobalTransform {FTransform()};
  AInstancedFoliageActor* InstancedFoliageActor {nullptr};
  AProceduralFoliageVolume* ProceduralFoliageVolume {nullptr};
  TArray<FTileMeshComponent> TileMeshesCache {};
  TArray<UMaterialInstanceDynamic*> MaterialInstanceDynamicCache {};

  void UpdateMaterialCache(const FLinearColor& Value);
};

USTRUCT()
struct FFoliageBlueprint
{
  GENERATED_BODY()
  FString BPClassName {};
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

public:
  void AddVehicle(ACarlaWheeledVehicle* Vehicle);
  void RemoveVehicle(ACarlaWheeledVehicle* Vehicle);

public:
//Cuando se carge un level engancharme al broadcast del delate del collision
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
  bool CheckIfAnyVehicleInLevel() const;
  bool CheckForNewTiles() const;

  TArray<FString> GetTilesInUse();

  void UpdateVehiclesDetectionBoxes();
  void UpdateMaterials(TArray<FString>& Tiles);
  TArray<TPair<FFoliageBlueprint, TArray<FTransform>>> GetElementsToSpawn(const TArray<FString>& Tiles);
  void SpawnSkeletalFoliages(TArray<TPair<FFoliageBlueprint, TArray<FTransform>>>& ElementsToSpawn);
  void DestroySkeletalFoliages();
  bool EnableActorFromPool(const FTransform& Transform, TArray<FPooledActor>& Pool);

  void UpdateTileDataCache();
  void UpdateFoliageBlueprintCache();
  void GenerateTileDataInternals();
  void InitializeInstancedStaticMeshComponentCache(FTileData& TileData);
  void InitializeMaterialCache(FTileData& TileData);

  void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);
  void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

  void CreatePoolForBPClass(const FFoliageBlueprint& BP);
  AActor* CreateFoliage(const FFoliageBlueprint& BP, const FTransform& Transform) const;

  void GetSketalTemplates();

private:
  //Actors
  ALargeMapManager* LargeMap {nullptr};
  TArray<ACarlaWheeledVehicle*> VehiclesInLevel {};
  //Caches
  TMap<FString, FFoliageBlueprint> FoliageBlueprintCache {};
  TMap<FString, FTileData> TileDataCache {};
  //Pools
  TMap<FString, TArray<FPooledActor>> ActorPool {};
};
