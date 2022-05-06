// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureCamera.h"

#include "Runtime/RenderCore/Public/RenderingThread.h"

FActorDefinition ASceneCaptureCamera::GetSensorDefinition()
{
  constexpr bool bEnableModifyingPostProcessEffects = true;
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(
      TEXT("rgb"),
      bEnableModifyingPostProcessEffects);
}

ASceneCaptureCamera::ASceneCaptureCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
}

void ASceneCaptureCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureCamera::PostPhysTick);
  FPixelReader::SendPixelsInRenderThread(*this);
  // do the gbuffer thing
  {
    auto GBufferStream = CameraGBuffers.SceneColor.GetDataStream(*this);
    auto Buffer = GBufferStream.PopBufferFromPool();
    TArray<FColor> SceneColorPixels;
    SceneColorPixels.SetNum(GetImageWidth()*GetImageHeight());
    for (FColor& color : SceneColorPixels)
    {
      color = FColor(255, 128, 64, 200);
    }
    auto Offset = carla::sensor::SensorRegistry::get<FCameraGBufferUint8*>::type::header_offset;
    Buffer.copy_from(Offset, SceneColorPixels);
    if(Buffer.data())
    {
      SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
      GBufferStream.Send(CameraGBuffers.SceneColor, std::move(Buffer), 
          GetImageWidth(), GetImageHeight(), GetFOVAngle());
    }
  }
}
