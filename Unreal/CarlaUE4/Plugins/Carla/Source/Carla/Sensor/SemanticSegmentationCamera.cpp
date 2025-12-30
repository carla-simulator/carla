// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SemanticSegmentationCamera.h"
#include "Carla/Sensor/InstanceSegmentationCamera.h"

#include "Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

FActorDefinition ASemanticSegmentationCamera::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("semantic_segmentation"));
}

ASemanticSegmentationCamera::ASemanticSegmentationCamera(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/GTMaterial.GTMaterial'"));
}

void ASemanticSegmentationCamera::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
  Super::SetUpSceneCaptureComponent(SceneCapture);
  AInstanceSegmentationCamera::SetUpSceneCaptureComponentForTagging(SceneCapture);
}

void ASemanticSegmentationCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASemanticSegmentationCamera::PostPhysTick);

  AInstanceSegmentationCamera::PostPhysTickForTagging(*GetCaptureComponent2D());
  FPixelReader::SendPixelsInRenderThread<ASemanticSegmentationCamera, FColor>(*this);
}
