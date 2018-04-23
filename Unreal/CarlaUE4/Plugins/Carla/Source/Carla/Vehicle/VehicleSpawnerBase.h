// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Util/ActorWithRandomEngine.h"

#include "VehicleSpawnerBase.generated.h"

class ACarlaWheeledVehicle;
class APlayerStart;

UCLASS(Abstract)
class CARLA_API AVehicleSpawnerBase : public AActorWithRandomEngine
{
  GENERATED_BODY()

public:

  // Sets default values for this actor's properties
  AVehicleSpawnerBase(const FObjectInitializer& ObjectInitializer);

protected:

  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  // Called when the actor is removed from the level
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnVehicle(const FTransform &SpawnTransform, ACarlaWheeledVehicle *&SpawnedCharacter);

  //UFUNCTION(BlueprintImplementableEvent)
  void TryToSpawnRandomVehicle();

public:

  void SetNumberOfVehicles(int32 Count);

  int32 GetNumberOfSpawnedVehicles() const
  {
    return Vehicles.Num();
  }

  const TArray<ACarlaWheeledVehicle *> &GetVehicles() const {
    return Vehicles;
  }

  void SetRoadMap(URoadMap *InRoadMap)
  {
    RoadMap = InRoadMap;
  }

  UFUNCTION(Category = "Road Map", BlueprintCallable)
  URoadMap *GetRoadMap()
  {
    return RoadMap;
  }
  /** Function called to spawn another vehicle when there is not enough spawn points in the beginplay */
  UFUNCTION(Category = "Vehicle Spawner", BlueprintCallable)
  void SpawnVehicleAttempt();

protected:

  APlayerStart* GetRandomSpawnPoint();

  ACarlaWheeledVehicle* SpawnVehicleAtSpawnPoint(const APlayerStart &SpawnPoint);

  UPROPERTY()
  URoadMap *RoadMap = nullptr;

  /** If false, no walker will be spawned. */
  UPROPERTY(Category = "Vehicle Spawner", EditAnywhere)
  bool bSpawnVehicles = true;

  /** Number of walkers to be present within the volume. */
  UPROPERTY(Category = "Vehicle Spawner", EditAnywhere, meta = (EditCondition = bSpawnVehicles, ClampMin = "1"))
  int32 NumberOfVehicles = 10;

  UPROPERTY(Category = "Vehicle Spawner", VisibleAnywhere, AdvancedDisplay)
  TArray<APlayerStart *> SpawnPoints;

  UPROPERTY(Category = "Vehicle Spawner", BlueprintReadOnly, VisibleAnywhere, AdvancedDisplay)
  TArray<ACarlaWheeledVehicle *> Vehicles;

  /** Time to spawn new vehicles after begin play if there was not enough spawn points at the moment */
  UPROPERTY(Category = "Vehicle Spawner", BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "0.1", ClampMax = "1000.0", UIMin = "0.1", UIMax = "1000.0"))
  float TimeBetweenSpawnAttemptsAfterBegin = 3.0f;

  /** Min Distance to the player vehicle to validate a spawn point location for the next vehicle spawn attempt */
  UPROPERTY(Category = "Vehicle Spawner", BlueprintReadWrite, EditAnywhere, meta = (ClampMin = "10", ClampMax = "10000", UIMin = "10", UIMax = "10000"))
  float DistanceToPlayerBetweenSpawnAttemptsAfterBegin = 5000;
private:
  
  /** Time handler to spawn more vehicles in the case we could not do it in the beginplay */
  FTimerHandle AttemptTimerHandle;
 
};
