// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma onceFoliages

#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"	

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "VegetationSpawner.generated.h"

USTRUCT()
struct FFoliageBlueprintCache
{
  GENERATED_BODY()
  FString Path {};
  FString BPClassName {};
  TSubclassOf<AActor> SpawnedClass { nullptr };

  FFoliageBlueprintCache();

  bool IsValid() const;
  bool SetBPClassName();
  bool SetSpawnedClass();

  private:
  FString GetVersionFromFString(const FString& String);
};

USTRUCT()
struct FLevelFoliage
{
  GENERATED_BODY()
  UInstancedStaticMeshComponent* Mesh {nullptr};
  TArray<FPooledActor> FoliagePool {};
};

USTRUCT()
struct FPooledActor
{
  GENERATED_BODY()
  bool InUse { false };
  AActor* Actor { nullptr };
  FTransform OriginalTransform { FTransform() };
  int32 OriginalIndex { -1 };

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

  UPROPERTY(Category = "CARLA Vegetation Spwaner", EditDefaultsOnly)
  int32 MinDistance {50};

  UPROPERTY(Category = "CARLA Vegetation Spwaner", EditDefaultsOnly)
  int32 MaxDistance {1000000000};

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
  void CreatePools();

  void AddFoliageIndicesInUse(TArray<int32>& Global, const TArray<int32>& Local);
  bool IsFoliageIndexInUse(const FLevelFoliage& Foliage, int32 Index) const;

  AActor* CreateFoliage(const FFoliageBlueprintCache& CacheBPClass, const FTransform& FoliageTransform) const;
  bool GetFoliageFromPool(FLevelFoliage& Foliage, int32 Index);

  void CacheFoliageTypesInLevel();
  bool IsFoliageTypeEnabled(const FString& Path) const;

private:
  TArray<AActor*> VehiclesInLevel;
  TMap<FString, FFoliageBlueprintCache> FoliageCache;
  TArray<FLevelFoliage> LevelFoliages;
};
