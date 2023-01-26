// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/NormalsCamera.h"

#include "Carla/Sensor/PixelReader.h"

FActorDefinition ANormalsCamera::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeNormalsCameraDefinition();
}

ANormalsCamera::ANormalsCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  Enable16BitFormat(false);
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/NormalsEffectMaterial.NormalsEffectMaterial'"));
}

void ANormalsCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ANormalsCamera::PostPhysTick);
  FPixelReader::SendPixelsInRenderThread<ANormalsCamera, FColor>(*this);
}
