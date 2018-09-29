// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/PixelReader.h"
#include "Carla/Sensor/SceneCaptureSensor.h"

#include "SceneCaptureCamera.generated.h"

UCLASS()
class CARLA_API ASceneCaptureCamera : public ASceneCaptureSensor
{
  GENERATED_BODY()

public:

  using ASceneCaptureSensor::EnablePostProcessingEffects;

protected:

  void Tick(float DeltaTime) override
  {
    Super::Tick(DeltaTime);
    FPixelReader::SendPixelsInRenderThread(*this);
  }
};
