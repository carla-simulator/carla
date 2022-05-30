// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"

#include "VegetationSpawner.generated.h"


USTRUCT()
struct FSpawnedFoliage
{
  GENERATED_BODY()
  bool InUse {false};
  UInstancedStaticMeshComponent* Mesh {nullptr};
  TArray<int32> Indices {};
  TArray<FTransform> Transforms {};
  TArray<AActor*> SpawnedActors {};
};

USTRUCT()
struct FFoliageBlueprintCache
{
  GENERATED_BODY()
  FString Path {};
  FString BPClassName {};
  TSubclassOf<AActor> SpawnedClass { nullptr };

  void GetBPName()
  {
    FString Left;
    FString Right;
    FString Aux = Path;
    Aux.Split(".BP", &Left, &Right, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
    if (Right.IsEmpty())
    {
      BPClassName = Path;
      return;
    }
    Right = "BP" + Right;
    Right.RemoveFromEnd("'", ESearchCase::IgnoreCase);
    BPClassName = Right;
  }
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

  /// @}
  // ===========================================================================
  /// @name Overriden from AActor
  // ===========================================================================
  /// @{

protected:

  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;


private:
  void CheckForProcedurals();
  void UpdateVehiclesInLevel();
  void UpdateProceduralInstanceCount();
  void UpdateFoliage(FSpawnedFoliage& Foliage, TArray<int32>& VehiclesDetection);
  void Default();
  FFoliageBlueprintCache FindInCache(const FString& Path);
  AActor* SpawnFoliage(const FFoliageBlueprintCache& CacheBPClass, const FTransform& FoliageTransform);
  FFoliageBlueprintCache GetClassFromPath(const FString& Path);
  bool IsFoliageTypeEnabled(const FString& Path);
  FString GetVersionFromFString(const FString& string);
  bool IsInstancedStaticMeshComponentLoaded(const UInstancedStaticMeshComponent* Mesh);


private:
  TArray<AActor*> VehiclesInLevel;
  TArray<FSpawnedFoliage> ProceduralInstances;
  TArray<FFoliageBlueprintCache> FoliageCache;
  bool Defaulted {false};

};

