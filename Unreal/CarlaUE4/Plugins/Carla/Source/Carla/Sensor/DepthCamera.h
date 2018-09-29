// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/PixelReader.h"
#include "Carla/Sensor/ShaderBasedSensor.h"

#include "DepthCamera.generated.h"

UCLASS()
class CARLA_API ADepthCamera : public AShaderBasedSensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition()
  {
    return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("depth_camera"));
  }

  ADepthCamera(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
  {
    LoadPostProcessingMaterial(
#if PLATFORM_LINUX
        TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial_GLSL.DepthEffectMaterial_GLSL'")
#else
        TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial.DepthEffectMaterial'")
#endif
    );
  }

protected:

  void Tick(float DeltaTime) override
  {
    Super::Tick(DeltaTime);
    FPixelReader::SendPixelsInRenderThread(*this);
  }
};
