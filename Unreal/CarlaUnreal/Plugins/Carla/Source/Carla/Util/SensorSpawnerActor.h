// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

#include "SensorSpawnerActor.generated.h"

class ASensor;
class USceneComponent;
class UCarlaEpisode;
struct FActorDefinition;
struct FActorDescription;

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

  // If true, sensor will be attached to the AttachActorClass if the Actor is found.
  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  bool bAttachToActor = false;
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

  UFUNCTION(BlueprintCallable, CallInEditor, Category="Config")
  void StartRecordingSensorData();

  UFUNCTION(BlueprintCallable, CallInEditor, Category="Config")
  void StopRecordingSensorData();

  UFUNCTION(BlueprintCallable, CallInEditor, Category="Config")
  void ToggleRadarVisibility();

protected:
  // Called when the game starts or when spawned.
  virtual void BeginPlay() override;

  virtual void Tick(float DeltaSeconds) override;

  // Root
  UPROPERTY(BlueprintReadOnly, Category="Components")
  USceneComponent* SceneComp;
  
  // Array with sensors to spawn
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
  TArray<FSensorTuple> SensorsToSpawn;

  // Class that the sensor will be attached if Actor is found. bAttachToActor bool has to be set to true.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Spawn")
  TSubclassOf<AActor> AttachActorClass;

  // Initial delay until the sensors start spawning.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.f, ClampMax=40.f), Category="Config|Spawn")
  float InitialDelay = 4.f;

  // Delay between spawns. Set to 0 for no delay.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.f, ClampMax=40.f), Category="Config|Spawn")
  float DelayBetweenSpawns = 0.f;

  // Max spawn location of the sensor. Spawn at random location between MaxSpawnLocation and MinSpawnLocation.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Spawn")
  FVector MaxSpawnLocation {-110000.f, -100000.f, 9600.f};

  // Min spawn location of the sensor. Spawn at random location between MaxSpawnLocation and MinSpawnLocation.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Spawn")
  FVector MinSpawnLocation {-120000.f, -110000.f, 9300.f};

  // Enables save data at begin play to /Saved/SensorSpawnerCaptures folder.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|SaveData")
  bool bSaveDataAtBeginPlay = false;

  // Subclass of ASceneCaptureSensor that we are going to save the images to disk. Set to nullptr to capture all ASceneCaptureSensor subclasses.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|SaveData")
  TSubclassOf<class ASensor> SensorClassToCapture;

  // Path where the sensor data will be saved. By default is the project/Saved/ folder.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|SaveData")
  FString SaveImagePath;

  // cm/s
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Radar")
  float VelocityRange = 0.075;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Radar")
  float PointSize = 15.0f;



private:
  bool bRecordingData = false;

  bool bRadarVisibility = false;

  UFUNCTION()
  void OnEpisodeInitialised(UCarlaEpisode* InitialisedEpisode);

  UFUNCTION()
  void SpawnSensorsDelayed();
  
  void GenerateSensorActorDescription(const FActorDefinition* Definition, FActorDescription& SensorDescription) const;

  // Gets a transform with a random location between MaxSpawnLocation and MinSpawnLocation.
  void GetRandomTransform(FTransform &Transform) const;
  
  void SpawnSensorActor(const FActorDescription& SensorDescription, bool bAttachToActor);

  const FActorDefinition* GetActorDefinitionByClass(const TSubclassOf<AActor> ActorClass) const;

  void AddSensorToSaveDataArray(AActor* Actor);
  
  void SaveSensorData(float DeltaSeconds);

  void AttachSensorToActor(AActor* SensorActor);
  
  void DrawRadarSensorPoints() const;

  UPROPERTY()
  UCarlaEpisode* CarlaEpisode;

  FTimerHandle InitialDelaySpawnTimerHandle;

  // Used for delayed spawn.
  FTimerHandle SpawnSensorsDelayedTimerHandle;
  
  // Used for delayed spawn. Track cameras taking pictures on tick.
  TArray<FSensorTuple> SensorsToSpawnCopy;

  // Track cameras saving pictures on tick.
  TArray<class ASensor*> SpawnedSensorsArray;
  
};
