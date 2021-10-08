// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Carla/Sensor/SceneCaptureSensor.h"
#include "Carla/Sensor/Sensor.h"

#include "SensorManager.generated.h"


UCLASS()
class CARLA_API UAtlasManager8Bit : public UObject
{
  GENERATED_BODY()

public:

  void RegisterSceneCaptureSensor(ASceneCaptureSensor* InSensor);

  void UnregisterSceneCaptureSensor(ASceneCaptureSensor* InSensor);

  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

private:
  UPROPERTY()
  TArray<class ASceneCaptureSensor*> CaptureSensors;

  FTexture2DRHIRef SceneCaptureAtlasTexture;
  TArray<FColor> AtlasPixels;
  uint32 AtlasTextureWidth = 0u;
  uint32 AtlasTextureHeight = 0u;
  bool IsAtlasTextureValid = true;
};

UCLASS()
class CARLA_API UAtlasManager16Bit : public UObject
{
  GENERATED_BODY()

public:

  void RegisterSceneCaptureSensor(ASceneCaptureSensor* InSensor);

  void UnregisterSceneCaptureSensor(ASceneCaptureSensor* InSensor);

  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

private:
  UPROPERTY()
  TArray<class ASceneCaptureSensor*> CaptureSensors;

  FTexture2DRHIRef SceneCaptureAtlasTexture;
  TArray<FFloat16Color> AtlasPixels;
  uint32 AtlasTextureWidth = 0u;
  uint32 AtlasTextureHeight = 0u;
  bool IsAtlasTextureValid = true;
};

UCLASS()
class CARLA_API ASensorManager : public AActor
{
  GENERATED_BODY()
public:

  ASensorManager(const FObjectInitializer& ObjectInitializer);

  void Register8BitCameraSensor(ASceneCaptureSensor* InSensor);

  void Unregister8BitCameraSensor(ASceneCaptureSensor* InSensor);

  void Register16BitCameraSensor(ASceneCaptureSensor* InSensor);

  void Unregister16BitCameraSensor(ASceneCaptureSensor* InSensor);

  void RegisterGenericSensor(ASensor* InSensor);

  void UnregisterGenericSensor(ASensor* InSensor);

  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds);

private:

  UPROPERTY()
  UAtlasManager8Bit* AtlasManager8Bit = nullptr;

  UPROPERTY()
  UAtlasManager16Bit* AtlasManager16Bit = nullptr;

  UPROPERTY()
  TArray<ASensor*> GenericSensors;
};
