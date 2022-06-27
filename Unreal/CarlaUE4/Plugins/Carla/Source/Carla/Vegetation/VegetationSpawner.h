// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma onceFoliages

#include "GameFramework/Actor.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Components/StaticMeshComponent.h"	

#include "Containers/Array.h"
#include "VegetationSpawner.generated.h"

USTRUCT()
struct FFoliageBlueprintCache
{
  GENERATED_BODY()
  FString BPClassName {};
  TSubclassOf<AActor> SpawnedClass { nullptr };

  FFoliageBlueprintCache() = default;
  FFoliageBlueprintCache(const FString& Path);
  bool IsValid() const;

private:
  void Init(const FString& Path);
  FString GetVersionFromFString(const FString& String);
};

USTRUCT()
struct FLevelFoliage
{
  GENERATED_BODY()
  UInstancedStaticMeshComponent* Mesh {nullptr};
  TArray<int32> IndicesInUse {};
  TArray<FTransform> Transforms {};
  TArray<AActor*> SpawnedActors {};
};


USTRUCT()
struct FPooledFoliage
{
  GENERATED_BODY()
  AActor* Actor { nullptr };
  bool InUse { false };

  void EnableActor();
  void DisableActor();
};

UCLASS()
class CARLA_API AVegetationSpawner : public AActor
{
  GENERATED_BODY()

public:
 //Filters for debug, improving performance
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
  void GetVehiclesInLevel();
  void GetFoliageTypesInLevel();
  void CreatePools();
  bool IsFoliageTypeEnabled(const FString& Path);
  bool FindInCache(const FString& Path) const;
  FFoliageBlueprintCache GetBlueprintFromCache(const FString& Path) const;
  bool StillInUse(const FTransform& Transform);
  AActor* GetFoliageFromPool(const FFoliageBlueprintCache& FoliageBlueprint);
  void HideFoliage(const FFoliageBlueprintCache& FoliageBlueprint, AActor* Actor);
  FPooledFoliage CreatePooledFoliage(const FFoliageBlueprintCache& FoliageBlueprint) const;
  void SpawnFoliage(AActor* Actor, const FTransform& FoliageTransform);
  bool AddFoliageToCache(const FFoliageBlueprintCache& NewElement);

  AActor* CreateFoliage(const FFoliageBlueprintCache& CacheBPClass, const FTransform& FoliageTransform);

private:
  TArray<AActor*> VehiclesInLevel;
  TArray<FLevelFoliage> FoliageTypesInLevel;
  TArray<FFoliageBlueprintCache> FoliageCache;
  TMap<FString, TArray<FPooledFoliage>> FoliagePool;
};
