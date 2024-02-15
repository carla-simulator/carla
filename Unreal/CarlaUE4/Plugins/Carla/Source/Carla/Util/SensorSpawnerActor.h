// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SensorSpawnerActor.generated.h"

class ASensor;
class USceneComponent;
class UCarlaEpisode;


USTRUCT(BlueprintType)
struct FSensorTuple
{
	GENERATED_BODY()

	// sensor class to spawn.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<ASensor> SensorClass;

	// Number of sensors to spawn of the SensorClass.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Amount = 1;
};


UCLASS(Blueprintable)
class CARLA_API ASensorSpawnerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASensorSpawnerActor();

	// Called OnBeginPlay().
	UFUNCTION(BlueprintCallable)
	void SpawnSensors();

	
protected:
	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Root
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components")
	USceneComponent* SceneComp;
	
	// Array with sensors to spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	TArray<FSensorTuple> SensorsToSpawn;

	// Initial delay until the sensors start spawning.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.f, ClampMax=40.f), Category="Config")
	float InitialDelay = 6.f;

	// Delay between spawns. Set to 0 for no delay.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.f, ClampMax=40.f), Category="Config")
	float DelayBetweenSpawns = 0.f;

	// Max spawn location of the sensor. Spawn at random location between MaxSpawnLocation and MinSpawnLocation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	FVector MaxSpawnLocation {-110000.f, -100000.f, 9600.f};

	// Min spawn location of the sensor. Spawn at random location between MaxSpawnLocation and MinSpawnLocation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	FVector MinSpawnLocation {-120000.f, -110000.f, 9300.f};

	
private:
	UFUNCTION()
	void OnEpisodeInitialised(UCarlaEpisode* InitialisedEpisode);

	UFUNCTION()
	void SpawnSensorsDelayed();
	
	void GenerateSensorActorDescription(const FActorDefinition* Definition, FActorDescription& SensorDescription) const;

	// Gets a transform with a random location between MaxSpawnLocation and MinSpawnLocation.
	void GetRandomTransform(FTransform &Transform) const;
  
	void SpawnSensorActor(const FActorDescription& SensorDescription) const;

	const FActorDefinition* GetActorDefinitionByClass(const TSubclassOf<AActor> ActorClass) const;

	FTimerHandle InitialDelaySpawnTimerHandle;
	
	UPROPERTY()
	UCarlaEpisode* CarlaEpisode;

	// Used for delayed spawn
	FTimerHandle SpawnSensorsDelayedTimerHandle;
	
	// Used for delayed spawn
	TArray<FSensorTuple> SensorsToSpawnCopy;
};
