// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/DepthCamera_WideAngleLens.h"
#include "Carla.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Util/CameraModelUtil.h"

#include "Carla/Sensor/PixelReader.h"

FActorDefinition ADepthCamera_WideAngleLens::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeWideAngleLensCameraDefinition(
      TEXT("depth"));
}

ADepthCamera_WideAngleLens::ADepthCamera_WideAngleLens(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  // Depth is encoded as a base-256 value across the R/G/B channels, so the
  // cubemap-to-fisheye resample must use point sampling; linear filtering
  // would interpolate the encoded bytes and decode to nonsense depth.
  Super::SetCubemapSampler(CameraModelUtil::GetSampler(ESamplerFilter::SF_Point));

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
  // Skip the whole 6-face capture + compute + readback pipeline when no
  // client is subscribed.
  if (!AreClientsListening())
    return;
  FPixelReader::SendPixelsInRenderThread<ADepthCamera_WideAngleLens, FColor>(*this);
}
