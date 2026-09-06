// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/InstanceSegmentationCamera_WideAngleLens.h"
#include "Carla.h"
#include "Carla/Util/CameraModelUtil.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

FActorDefinition AInstanceSegmentationCamera_WideAngleLens::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeWideAngleLensCameraDefinition(
      TEXT("instance_segmentation"));
}

AInstanceSegmentationCamera_WideAngleLens::AInstanceSegmentationCamera_WideAngleLens(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  Super::SetCubemapSampler(CameraModelUtil::GetSampler(ESamplerFilter::SF_Point));

  AddPostProcessingMaterial(
      TEXT("Material'/Game/Carla/PostProcessMaterials/M_InstanceSegmentationSensorMaterial'"));
}

void AInstanceSegmentationCamera_WideAngleLens::SetUpSceneCaptureComponents(
    TArrayView<USceneCaptureComponent2D_CARLA*> SceneCaptures)
{
  Super::SetUpSceneCaptureComponents(SceneCaptures);
}

void AInstanceSegmentationCamera_WideAngleLens::PostPhysTick(
    UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AInstanceSegmentationCamera_WideAngleLens::PostPhysTick);
  // Skip the whole 6-face capture + compute + readback pipeline when no
  // client is subscribed.
  if (!AreClientsListening())
    return;
  FPixelReader::SendPixelsInRenderThread<AInstanceSegmentationCamera_WideAngleLens, FColor>(*this);
}
