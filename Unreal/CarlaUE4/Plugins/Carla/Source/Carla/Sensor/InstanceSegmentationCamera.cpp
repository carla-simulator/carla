// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/InstanceSegmentationCamera.h"
#include "Carla/Game/TaggedComponent.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

#include "Components/SceneCaptureComponent2D.h"

FActorDefinition AInstanceSegmentationCamera::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("instance_segmentation"));
}

AInstanceSegmentationCamera::AInstanceSegmentationCamera(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  AddPostProcessingMaterial(TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  // TODO: Setup OnActorSpawnHandler so we can refresh components
  // World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateRaw(this, &AInstanceSegmentationCamera::OnActorSpawned));
}

void AInstanceSegmentationCamera::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
  Super::SetUpSceneCaptureComponent(SceneCapture);
  ApplyViewMode(VMI_Unlit, true, SceneCapture.ShowFlags);
  SetUpSceneCaptureComponentForTagging(SceneCapture);
}

void AInstanceSegmentationCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AInstanceSegmentationCamera::PostPhysTick);

  PostPhysTickForTagging(*GetCaptureComponent2D());
  FPixelReader::SendPixelsInRenderThread<AInstanceSegmentationCamera, FColor>(*this);
}

void AInstanceSegmentationCamera::SetUpSceneCaptureComponentForTagging(USceneCaptureComponent2D &SceneCapture) {
  SceneCapture.ShowFlags.SetNotDrawTaggedComponents(false); // TaggedComponent detects this and sets view relevance for proxy material
  SceneCapture.ShowFlags.SetAtmosphere(false);
  SceneCapture.PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
  AddTaggedComponentsToSceneCapture(SceneCapture);
}

void AInstanceSegmentationCamera::PostPhysTickForTagging(USceneCaptureComponent2D &SceneCapture) {
  SceneCapture.ClearShowOnlyComponents();
  AddTaggedComponentsToSceneCapture(SceneCapture);
}

void AInstanceSegmentationCamera::AddTaggedComponentsToSceneCapture(USceneCaptureComponent2D &SceneCapture) {
  TArray<UObject*> TaggedComponents;
  GetObjectsOfClass(UTaggedComponent::StaticClass(), TaggedComponents, false, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);
  GetObjectsOfClass(UTaggedLandscapeComponent::StaticClass(), TaggedComponents, false, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);

  for (UObject* Object : TaggedComponents) {
    UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Object);
    SceneCapture.ShowOnlyComponents.Emplace(Component);
  }
}
