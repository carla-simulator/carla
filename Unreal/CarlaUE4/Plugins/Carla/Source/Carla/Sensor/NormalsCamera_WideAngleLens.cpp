// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/NormalsCamera_WideAngleLens.h"

#include "Carla/Sensor/PixelReader.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

FActorDefinition ANormalsCamera_WideAngleLens::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeWideAngleLensCameraDefinition(TEXT("normals"));
}

ANormalsCamera_WideAngleLens::ANormalsCamera_WideAngleLens(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/NormalsEffectMaterial.NormalsEffectMaterial'"));
}

void ANormalsCamera_WideAngleLens::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ANormalsCamera_WideAngleLens::PostPhysTick);
  FPixelReader::SendPixelsInRenderThread<ANormalsCamera_WideAngleLens, FColor>(*this);
}
