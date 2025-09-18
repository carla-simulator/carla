// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SemanticSegmentationCamera_WideAngleLens.h"

#include "Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

FActorDefinition ASemanticSegmentationCamera_WideAngleLens::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeWideAngleLensCameraDefinition(
    TEXT("semantic_segmentation"));
}

ASemanticSegmentationCamera_WideAngleLens::ASemanticSegmentationCamera_WideAngleLens(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  Super::SetCubemapSampler(CameraModelUtil::GetSampler(ESamplerFilter::SF_Point));

  AddPostProcessingMaterial(TEXT("Material'/Carla/PostProcessingMaterials/GTMaterial.GTMaterial'"));
}

void ASemanticSegmentationCamera_WideAngleLens::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASemanticSegmentationCamera_WideAngleLens::PostPhysTick);
  FPixelReader::SendPixelsInRenderThread<ASemanticSegmentationCamera_WideAngleLens, FColor>(*this);
}
