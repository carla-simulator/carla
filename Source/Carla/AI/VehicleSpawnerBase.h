// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/Actor.h"
#include "VehicleSpawnerBase.generated.h"
//#include "Content/Blueprints/BaseVehiclePawn.h"


class APlayerStart;

UCLASS(Abstract)
class CARLA_API AVehicleSpawnerBase : public AActor
{
	GENERATED_BODY()
	

public:	
	// Sets default values for this actor's properties
	AVehicleSpawnerBase(const FObjectInitializer& ObjectInitializer);



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

  UFUNCTION(BlueprintCallable)
  const FRandomStream &GetRandomStream() const
  {
    return RandomStream;
  }

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnVehicle(const FTransform &SpawnTransform, AWheeledVehicle *&SpawnedCharacter);

  //UFUNCTION(BlueprintImplementableEvent)
  void TryToSpawnRandomVehicle();

  UFUNCTION(BlueprintImplementableEvent)
  AAICarlaVehicleController* GetVehicleController(AWheeledVehicle* Vechicle);



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

   void SetNumberOfVehicles(int32 Count);

   APlayerStart* GetRandomSpawnPoint() const;

   void SpawnVehicleAtSpawnPoint(const APlayerStart &SpawnPoint);

protected:

/** If false, no walker will be spawned. */
  UPROPERTY(Category = "Vehicle Spawner", EditAnywhere)
  bool bSpawnVehicles = true;

  /** Number of walkers to be present within the volume. */
  UPROPERTY(Category = "Vehicle Spawner", EditAnywhere, meta = (EditCondition = bSpawnVehicles, ClampMin = "1"))
  int32 NumberOfVehicles = 10;

  /** Minimum walk distance in centimeters. */
  /*UPROPERTY(Category = "Vechicle Spawner", EditAnywhere, meta = (EditCondition = bSpawnWalkers))
  float MinimumWalkDistance = 1500.0f;
*/
  /** If false, a random seed is generated each time. */
  UPROPERTY(Category = "Vehicle Spawner", EditAnywhere, meta = (EditCondition = bSpawnVehicles))
  bool bUseFixedSeed = true;

  /** Seed for spawning random walkers. */
  UPROPERTY(Category = "Vehicle Spawner", EditAnywhere, meta = (EditCondition = bUseFixedSeed))
  int32 Seed = 123456789;

  UPROPERTY()
  FRandomStream RandomStream;

  //UPROPERTY(Category = "Vechicle Spawner", VisibleAnywhere, AdvancedDisplay)
  TArray<APlayerStart *> SpawnPoints;
/*
  UPROPERTY(Category = "Vechicle Spawner", BlueprintReadOnly, EditAnywhere, AdvancedDisplay)
  TArray<AWheeledVehicle *> Vehicles;
*/
  UPROPERTY(Category = "Vehicle Spawner", BlueprintReadOnly, VisibleAnywhere, AdvancedDisplay)
  TArray< AWheeledVehicle *> Vehicles;
	
	
};
