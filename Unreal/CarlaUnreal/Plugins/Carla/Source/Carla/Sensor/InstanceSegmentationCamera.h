// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/ShaderBasedSensor.h"
#include "Carla/Actor/ActorDefinition.h"

#include "InstanceSegmentationCamera.generated.h"

/// Sensor that produces "Instance segmentation" images.
UCLASS()
class CARLA_API AInstanceSegmentationCamera : public AShaderBasedSensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  AInstanceSegmentationCamera(const FObjectInitializer &ObjectInitializer);

protected:

  void SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture) override;
  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;
};
