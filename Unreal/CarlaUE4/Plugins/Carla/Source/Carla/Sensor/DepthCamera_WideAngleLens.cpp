// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/DepthCamera_WideAngleLens.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

FActorDefinition ADepthCamera_WideAngleLens::GetSensorDefinition()
{
    return UActorBlueprintFunctionLibrary::MakeWideAngleLensCameraDefinition(
        TEXT("depth"));
}

ADepthCamera_WideAngleLens::ADepthCamera_WideAngleLens(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  AddPostProcessingMaterial(
#if PLATFORM_LINUX
      TEXT("Material'/Carla/PostProcessingMaterials/WideAngleLens/DepthEffectMaterial_GLSL_WAL.DepthEffectMaterial_GLSL_WAL'")
#else
      TEXT("Material'/Carla/PostProcessingMaterials/WideAngleLens/DepthEffectMaterial_WAL.DepthEffectMaterial_WAL'")
#endif
  );
}

void ADepthCamera_WideAngleLens::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ADepthCamera_WideAngleLens::PostPhysTick);
  FPixelReader::SendPixelsInRenderThread<ADepthCamera_WideAngleLens, FColor>(*this);
}
