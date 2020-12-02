// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureCamera.h"

#include "Runtime/RenderCore/Public/RenderingThread.h"
// #include "Runtime/Engine/Public/UnrealEngine.h"

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

void ASceneCaptureCamera::Tick(const float DeltaTime)
{
  Super::Tick(DeltaTime);
  // FlushRenderingCommands(true);
  // UE_LOG(LogCarla, Warning, TEXT("FlushRenderingCommands()"));
  // FPixelReader::SendPixelsInRenderThread(*this);
  // RenderFence.BeginFence();
  // UE_LOG(LogCarla, Warning, TEXT("RenderFence.BeginFence()"));
}

void ASceneCaptureCamera::SendPixels(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  if(ReadyToCapture)
  {
    UE_LOG(LogCarla, Warning, TEXT("FlushRenderingCommands()"));
    FlushRenderingCommands(true);

    // UE_LOG(LogCarla, Warning, TEXT("RenderFence.Wait() (waiting...)"));
    // RenderFence.Wait();

    // FFrameEndSync::Sync(false);

    // UE_LOG(LogCarla, Warning, TEXT("Fence done! Sending Pixels..."));

    FPixelReader::SendPixelsInRenderThread(*this);
    // UE_LOG(LogCarla, Warning, TEXT("Pixels sended!"));
    ReadyToCapture = false;
  }
}
