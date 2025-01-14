// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/InstanceSegmentationCamera.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include <util/ue-header-guard-begin.h>
#include "Components/SceneCaptureComponent2D.h"
#include <util/ue-header-guard-end.h>

FActorDefinition AInstanceSegmentationCamera::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("instance_segmentation"));
}

AInstanceSegmentationCamera::AInstanceSegmentationCamera(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  AddPostProcessingMaterial(TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  AddPostProcessingMaterial(TEXT("Material'/Game/Carla/PostProcessMaterials/M_InstanceSegmentationSensorMaterial'"));
  
  // TODO: Setup OnActorSpawnHandler so we can refresh components
  // World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateRaw(this, &AInstanceSegmentationCamera::OnActorSpawned));
}

void AInstanceSegmentationCamera::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
  Super::SetUpSceneCaptureComponent(SceneCapture);
}

void AInstanceSegmentationCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AInstanceSegmentationCamera::PostPhysTick);
  Super::PostPhysTick(World, TickType, DeltaSeconds);
  
  auto FrameIndex = FCarlaEngine::GetFrameCounter();
  ImageUtil::ReadSensorImageDataAsyncFColor(*this, [this, FrameIndex](
    TArrayView<const FColor> Pixels,
    FIntPoint Size) -> bool
  {
    SendDataToClient(*this, Pixels, FrameIndex);
    return true;
  });
}
