// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/ShaderBasedSensor.h"

#include "SemanticSegmentationCamera.generated.h"

UCLASS()
class CARLA_API ASemanticSegmentationCamera : public AShaderBasedSensor
{
  GENERATED_BODY()

public:

  ASemanticSegmentationCamera(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
  {
    LoadPostProcessingMaterial(TEXT("Material'/Carla/PostProcessingMaterials/GTMaterial.GTMaterial'"));
  }
};
