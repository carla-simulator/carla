// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/InstanceSegmentationCamera_WideAngleLens.h"
#include "Carla/Util/CameraModelUtil.h"
#include "Carla/Game/TaggedComponent.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

#include "Components/SceneCaptureComponent2D.h"

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

  // TODO: Setup OnActorSpawnHandler so we can refresh components
  // World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateRaw(this, &AInstanceSegmentationCamera_WideAngleLens::OnActorSpawned));
}

void AInstanceSegmentationCamera_WideAngleLens::SetUpSceneCaptureComponents(TArrayView<USceneCaptureComponent2D_CARLA*> SceneCaptures)
{
  Super::SetUpSceneCaptureComponents(SceneCaptures);

  for (auto SceneCapture : SceneCaptures)
  {
    ApplyViewMode(VMI_Unlit, true, SceneCapture->ShowFlags);

    SceneCapture->ShowFlags.SetNotDrawTaggedComponents(false); // TaggedComponent detects this and sets view relevance for proxy material

    SceneCapture->ShowFlags.SetAtmosphere(false);

    SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

    TArray<UObject *> TaggedComponents;
    GetObjectsOfClass(UTaggedComponent::StaticClass(), TaggedComponents, false, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);

    TArray<UPrimitiveComponent *> ShowOnlyComponents;
    for (UObject *Object : TaggedComponents) {
      UPrimitiveComponent *Component = Cast<UPrimitiveComponent>(Object);
      SceneCapture->ShowOnlyComponents.Emplace(Component);
    }
  }
}

void AInstanceSegmentationCamera_WideAngleLens::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AInstanceSegmentationCamera_WideAngleLens::PostPhysTick);

  TArray<UObject*> TaggedComponents;
  GetObjectsOfClass(UTaggedComponent::StaticClass(), TaggedComponents, false, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);

  for (auto SceneCapture : Super::GetCaptureComponents2D())
  {
      SceneCapture->ClearShowOnlyComponents();
      for (UObject* Object : TaggedComponents)
      {
          UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Object);
          if (Component != nullptr)
            SceneCapture->ShowOnlyComponents.Emplace(Component);
      }
  }

  FPixelReader::SendPixelsInRenderThread<AInstanceSegmentationCamera_WideAngleLens, FColor>(*this);
}
