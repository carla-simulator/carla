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

protected:

  APlayerStart* GetRandomSpawnPoint();

  void SpawnVehicleAtSpawnPoint(const APlayerStart &SpawnPoint);

  UPROPERTY()
  URoadMap *RoadMap;

  /** If false, no walker will be spawned. */
  UPROPERTY(Category = "Vehicle Spawner", EditAnywhere)
  bool bSpawnVehicles = true;

  /** Number of walkers to be present within the volume. */
  UPROPERTY(Category = "Vehicle Spawner", EditAnywhere, meta = (EditCondition = bSpawnVehicles, ClampMin = "1"))
  int32 NumberOfVehicles = 10;

  UPROPERTY(Category = "Vechicle Spawner", VisibleAnywhere, AdvancedDisplay)
  TArray<APlayerStart *> SpawnPoints;

  UPROPERTY(Category = "Vehicle Spawner", BlueprintReadOnly, VisibleAnywhere, AdvancedDisplay)
  TArray<ACarlaWheeledVehicle *> Vehicles;
};
