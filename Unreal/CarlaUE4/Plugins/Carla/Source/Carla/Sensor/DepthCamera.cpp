// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/DepthCamera.h"

#include "Carla/Sensor/PixelReader.h"

FActorDefinition ADepthCamera::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("depth"));
}

ADepthCamera::ADepthCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  AddPostProcessingMaterial(
#if PLATFORM_LINUX
      TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial_GLSL.DepthEffectMaterial_GLSL'")
#else
      TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial.DepthEffectMaterial'")
#endif
  );

  Offset = carla::sensor::SensorRegistry::get<ADepthCamera*>::type::header_offset;
}

void ADepthCamera::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

#if !UE_BUILD_SHIPPING
  ACarlaGameModeBase* GameMode = Cast<ACarlaGameModeBase>(GetWorld()->GetAuthGameMode());
  if(!GameMode->IsCameraStreamEnabled()) return;
#endif

  SendPixelsInStream(*this);
}
